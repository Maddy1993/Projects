package com.northeastern.edu.utils;

import com.northeastern.edu.secretSharing.Key;
import generated.thrift.impl.*;

import javafx.util.Pair;
import org.apache.thrift.TException;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

import java.io.*;
import java.math.BigInteger;
import java.time.Duration;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;

public class ServiceHandler extends ClientAuthentication {

    //Logger for the class.
    private static Logger LOGGER = Logger.getLogger(ServiceHandler.class.getName());

    //Variable to hold the data-store of the server.
    //Data-store is represented as key-value pair where key and value strings.
    protected Map<String, String> keyValuePair;

    //Replica Port Numbers on localhost.
    protected List<Integer> replicaPorts;

    //Host Address.
    protected String hostAddress;

    //Host PortNumber.
    protected Integer portNumber;

    //Current sequence number.
    protected static LocalDateTime currentSequenceNumber;

    //Constructor for initializing the key-value store.
    public ServiceHandler(List<Integer> replicaPorts, Integer portNumber) throws IOException {

        super(portNumber);

        //Host Address
        this.portNumber = portNumber;
        this.hostAddress = "localhost";

        //create connection for the clients.
        this.replicaPorts = replicaPorts;

        //The current sequence number would be set to now() initially.
        currentSequenceNumber = LocalDateTime.now();

        //Load the existing key value store of the server.
        this.keyValuePair = (Map<String, String>) loadMemoryObject(0);
    }

    //Loads the key value data store from memory.
    protected Object loadMemoryObject(int mode) throws IllegalStateException, IOException {
        try {
            FileReader reader = new FileReader(memoryObjectFileName);
            JSONParser jsonParser = new JSONParser();
            return ((Map<String, String>)jsonParser.parse(reader)).get("data");
        } catch (IOException e) {
            String message = "Error loading data from memory: " + e.getMessage();
            LOGGER.severe(message);

            if (mode == 0) {
                new File(memoryObjectFileName).createNewFile();
            } else {}

        } catch (ParseException e) {
            LOGGER.info("File: " + memoryObjectFileName + " is empty.");
        }

        return defaultMemoryObject();
    }

    //Write the learned value to memory
    void writeToMemory(Map<String, String> keyValuePair) throws IOException {

        JSONObject jsonObject = new JSONObject();
        try {
            //Creating a map of values to store.
            FileReader reader = new FileReader(memoryObjectFileName);
            JSONParser jsonParser = new JSONParser();

            try {
                jsonObject = (JSONObject) jsonParser.parse(reader);
            } catch (ParseException e) {
                jsonObject = new JSONObject();
            }

            reader.close();

        } catch (IOException e) {
            LOGGER.severe("Error while saving the file to memory." + e.getMessage());
            new File(memoryObjectFileName).createNewFile();
        }

        OutputStream writer = new FileOutputStream(memoryObjectFileName);
        jsonObject.put("data", keyValuePair);
        writer.write(jsonObject.toJSONString().getBytes());
        writer.flush();
        writer.close();
    }

    //Write the learned keys to memory
    void writeKeysToMemory(Map<String, List<String>> keyValuePair) throws IOException {
        JSONObject jsonObject = new JSONObject();

        try {
            //Creating a map of values to store.
            FileReader reader = new FileReader(memoryObjectFileName);
            JSONParser jsonParser = new JSONParser();

            try {
                jsonObject = (JSONObject) jsonParser.parse(reader);
            } catch (ParseException e) {
                jsonObject = new JSONObject();
            }

            reader.close();

        } catch (IOException e) {
            LOGGER.severe("Error while saving the file to memory." + e.getMessage());
            new File(memoryObjectFileName).createNewFile();
        }

        OutputStream writer = new FileOutputStream(memoryObjectFileName);
        jsonObject.put("keys", keyValuePair);
        writer.write(jsonObject.toJSONString().getBytes());
        writer.flush();
        writer.close();
    }

    //Write the learned keys to memory
    void writeCommitTimeToMemory(Map<String, String> keyValuePair) throws IOException {
        JSONObject jsonObject = new JSONObject();

        try {
            //Creating a map of values to store.
            FileReader reader = new FileReader(memoryObjectFileName);
            JSONParser jsonParser = new JSONParser();

            try {
                jsonObject = (JSONObject) jsonParser.parse(reader);
            } catch (ParseException e) {
                jsonObject = new JSONObject();
            }
            reader.close();

        } catch (IOException e) {
            LOGGER.severe("Error while saving the file to memory." + e.getMessage());
            new File(memoryObjectFileName).createNewFile();
        }

        OutputStream writer = new FileOutputStream(memoryObjectFileName);
        jsonObject.put("commit", keyValuePair);
        writer.write(jsonObject.toJSONString().getBytes());
        writer.flush();
        writer.close();
    }

    //Definition needs to be provided by child class.
    protected String getValue(String key) throws TException { return "";}

    protected boolean canWriteOrDelete(Map<String, String> keyValue, OperationType write) throws TException { return false;}

    @Override
    public RequestPacket getValueForKey(String key) throws TException {
        RequestPacket response = new RequestPacket();

        String value = getValue(key);
        Map<String, String> responseValue = new HashMap<>();

        if (!value.isEmpty()) {
            response.type = MessageType.SUCCESS;
            responseValue.put(key, value);
            response.keyValue = responseValue;
        } else {
            response.type = MessageType.FAILURE;
            Map<String, String> r = new HashMap<>();
            r.put("Address", hostAddress+":"+portNumber.toString());
            response.keyValue = r;
        }

        return response;
    }

    @Override
    public List<String> getKeys() throws TException {
        try {
            this.keyValuePair = (Map<String, String>) loadMemoryObject(1);

            if (this.keyValuePair == null) {
                return new ArrayList<>();
            }

            return new ArrayList<>(keyValuePair.keySet());
        } catch (IOException e) {
            LOGGER.severe("Error loading memory object");
            throw new TException("Error loading memory object: " + e.getMessage());
        }
    }

    @Override
    public RequestPacket storeKeyValue(Map<String, String> keyValue, OperationType operationType) throws TException {
        RequestPacket response = new RequestPacket();

        if (canWriteOrDelete(keyValue, operationType)) {
            if (operationType == OperationType.WRITE) {
                this.keyValuePair.putAll(keyValue);
                try {
                    writeToMemory(this.keyValuePair);
                } catch (IOException e) {
                    throw new TException(e.getMessage());
                }
            } else if (operationType == OperationType.LOGIN) {

                //for each key in the key value
                for (String key : keyValue.keySet()) {
                    //Check the last commit time for the client.
                    if (this.clientKeys.containsKey(key)) {
                        List<String> currentKeys = this.clientKeys.get(key);

                        if (currentKeys.size() >=3) {
                            currentKeys = new ArrayList<>();
                        }

                        currentKeys.add(keyValue.get(key));
                        this.clientKeys.put(key, currentKeys);

                    } else {
                        List<String> currentKeys = new ArrayList<>();
                        currentKeys.add(keyValue.get(key));
                        this.clientKeys.put(key, currentKeys);
                    }

                    //Add the commit time to memory.
                    Map<String, String> commitTime = new HashMap<>();
                    commitTime.put(key, LocalDateTime.now().toString());
                    try {
                        writeCommitTimeToMemory(commitTime);
                    } catch (IOException e) {
                        throw new TException(e.getMessage());
                    }
                }

                try {
                    writeKeysToMemory(this.clientKeys);
                } catch (IOException e) {
                    throw new TException(e.getMessage());
                }
                response.type = MessageType.SUCCESS;
            } else {
                response.type = MessageType.FAILURE;
            }

        }
            return response;
        }

    /**
     * Validates if the request to add keys is a new commit.
     *
     * @param key The commit for the key
     * @return
     */
    protected boolean isNewCommit(String key) throws TException{
        try {
            this.lastCommittedDate = (Map<String, String>) loadCommitTimesMemoryObject(1);
            if (this.lastCommittedDate == null) {
                this.lastCommittedDate = new HashMap<>();
                return true;
            } else if (this.lastCommittedDate.containsKey(key)) {
                LocalDateTime storedDate = LocalDateTime.parse(this.lastCommittedDate.get(key));
                return Duration.between(storedDate, LocalDateTime.now()).getSeconds() > 60;
//                return false;
            } else {
                return true;
            }
        } catch (IOException e) {
            LOGGER.severe("Error loading commit times from memory: " + e.getMessage());
            throw new TException("Error loading commit times from memory: " + e.getMessage());
        }
    }

    @Override
    public RequestPacket deleteKey(String key, OperationType operationType) throws TException {
        RequestPacket response = new RequestPacket();

        Map<String, String> keyValue = new HashMap<>();
        keyValue.put(key, this.keyValuePair.get(key));

        if (canWriteOrDelete(keyValue, operationType)) {
            if (operationType == OperationType.DELETE) {
                this.keyValuePair.remove(key);
                try {
                    writeToMemory(this.keyValuePair);
                } catch (IOException e) {
                    throw new TException(e.getMessage());
                }
            } else if(operationType == OperationType.LOGIN) {
                this.clientKeys.remove(key);
                try {
                    writeKeysToMemory(this.clientKeys);
                } catch (IOException e) {
                    throw new TException(e.getMessage());
                }
            }

            response.type = MessageType.SUCCESS;
        } else {
            response.type = MessageType.FAILURE;
        }

        return response;
    }

    @Override
    public List<String> replicaAddresses() throws TException {
        return new ArrayList<>();
    }

    @Override
    public MessageType ping() throws TException {
        return null;
    }

    @Override
    public ServerPacket acceptProposal(ServerPacket packet) throws TException {
        return new ServerPacket();
    }

    @Override
    public String getStoredValue(String key) throws TException {
        return "";
    }
}
