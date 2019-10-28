package com.northeastern.edu.utils;

import com.northeastern.edu.secretSharing.Key;
import com.northeastern.edu.secretSharing.SecretSharing;
import com.sun.tools.corba.se.idl.StringGen;
import generated.thrift.impl.*;
import javafx.util.Pair;
import org.apache.thrift.TException;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.math.BigInteger;
import java.time.LocalDateTime;
import java.util.*;
import java.util.logging.Logger;

public abstract class ClientAuthentication implements CommunicationService.Iface {

    //Logger for the class.
    private static Logger LOGGER = Logger.getLogger(ClientAuthentication.class.getName());

    //Secret-Sharing Keys for client.
    protected Map<String, List<String>> clientKeys;

    //Reconstruction keys for the client
    private List<Key> clientReconstructionKeys;

    //Last key commit times
    protected  Map<String, String> lastCommittedDate;

    //FileName to canWriteOrDelete and read data from memory.
    protected String memoryObjectFileName = "data";

    //Random value for the list
    Random randomValue;

    //Secret Sharing class
    SecretSharing sharing;

    protected ClientAuthentication(Integer portNumber) {
        clientKeys = new HashMap<>();
        randomValue = new Random();
        lastCommittedDate = new HashMap<>();

        //Construct file name for the server.
        this.memoryObjectFileName += ":" + portNumber.toString() + ".json";

        sharing = new SecretSharing(this.memoryObjectFileName);
    }

    //Loads the client password value data store from memory.
    protected Object loadKeysMemoryObject(int mode) throws IllegalStateException, IOException {
        try {
            FileReader reader = new FileReader(memoryObjectFileName);
            JSONParser jsonParser = new JSONParser();
            return ((Map<String, String>)jsonParser.parse(reader)).get("keys");
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

    //Loads the client password value data store from memory.
    protected Object loadCommitTimesMemoryObject(int mode) throws IllegalStateException, IOException {
        try {
            FileReader reader = new FileReader(memoryObjectFileName);
            JSONParser jsonParser = new JSONParser();
            return ((Map<String, String>)jsonParser.parse(reader)).get("commit");
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

    //Generates the structure of a default memory object
    protected Object defaultMemoryObject() {
        Map<String, Object> defaultMemoryObject = new HashMap<>();
        return defaultMemoryObject;
    }

    //Attempts reconstruction of the client keys with the keys
    //in the memory.
    private boolean attemptReconstruction(String clientAddress) throws IOException {
        this.clientKeys = (Map<String, List<String>>) loadKeysMemoryObject(1);

        if (clientKeys != null && clientKeys.containsKey(clientAddress)) {
            generateKeysFromString(clientKeys.get(clientAddress));
            return sharing.reconstruction(clientReconstructionKeys);
        } else {
            return false;
        }
    }

    /**
     * Generates keys from the string.
     *
     * @param keys   List of keys
     */
    private void generateKeysFromString(List<String> keys) {
        clientReconstructionKeys = new ArrayList<>();
        for (String key: keys) {
            String[] value = key.split("=");

            if (value.length ==2) {
                Key newKey = new Key(
                        new Pair<>(Integer.parseInt(value[0])
                                , new BigInteger(value[1])));
                clientReconstructionKeys.add(newKey);
            }
        }
    }

    @Override
    public RequestPacket login(String password, String clientAddress) throws TException {

        RequestPacket response = new RequestPacket();
        try {
            if (attemptReconstruction(clientAddress)) {
                response.type = MessageType.SUCCESS_WRITE;
            } else {
                //Using the password, construct the key set.
                List<Key> keys = sharing.preparation(password);

                //As each client would need at least three keys to reconstruct
                //the password, send three random keys to the client.
                Map<String, String> clientKeyMap;


                Collections.shuffle(keys);
                for (int index =0; index<3; index++) {
                    Key key = keys.get(index);

                    clientKeyMap = new HashMap<>();
                    clientKeyMap.put(clientAddress, key.toString());
                    response = storeKeyValue(clientKeyMap, OperationType.LOGIN);

                    if (response.type == MessageType.FAILURE) {
                        return response;
                    }
                }
            }

            return response;
        } catch (IOException e) {
            LOGGER.severe("Error reading keys value from memory.");
            throw new TException("Error reading keys value from memory." + e.getMessage());
        }
    }
}