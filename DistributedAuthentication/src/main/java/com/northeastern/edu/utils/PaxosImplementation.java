package com.northeastern.edu.utils;

import generated.thrift.impl.MessageType;
import generated.thrift.impl.OperationType;
import generated.thrift.impl.RPCPacket;
import generated.thrift.impl.RPCPacketService;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TTransportException;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

import java.io.*;
import java.time.LocalDateTime;
import java.util.*;
import java.util.logging.Logger;

public class PaxosImplementation implements RPCPacketService.Iface {

    //Logger for the class.
    private static Logger LOGGER = Logger.getLogger(PaxosImplementation.class.getName());

    //Key-Value Pair for a given server.
    private Map<String, String> keyValuePair;

    //FileName to write and read data from memory.
    private String memoryObjectFileName = "data";

    //Replica Connections. Client-Availability mapping
    private Map<RPCPacketService.Client, Boolean> replicas;

    //Replica Port Numbers on localhost.
    private List<Integer> replicaPorts;

    //Host Address.
    private String hostAddress;

    //Current sequence number.
    private static LocalDateTime currentSequenceNumber;

    //Variable to represent the maximum value seen so far.
    private static Map<String, String> valueOfHighestProposal;

    //Flag to represent the promise status.
    private static boolean promiseStatus;

    //Variable representing the agreed upon value.
    private static Map<String, String> agreedValue;

    //Variable representing the agreed upon sequence number.
    private static String agreedProposal;

    //Constructor for the Class.
    public PaxosImplementation(List<Integer> replicaPorts, Integer portNumber) throws IOException {
        //Construct file name for the server.
        this.memoryObjectFileName += ":" + portNumber.toString() + ".json";

        //Host Address
        this.hostAddress = "localhost";

        //create connection for the clients.
        this.replicaPorts = replicaPorts;

        //Load the existing key value store of the server.
        this.keyValuePair = (Map<String, String>) loadMemoryObject(0);

        //The current sequence number would be set to now() initially.
        currentSequenceNumber = LocalDateTime.now();
    }

    //Loads the key value data store from memory.
    private Object loadMemoryObject(int mode) throws IllegalStateException, IOException {
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
    private void writeToMemory(Map<String, String> keyValuePair) {
        try {
            //Creating a map of values to store.
            JSONObject jsonObject = new JSONObject();
            OutputStream writer = new FileOutputStream(memoryObjectFileName);
            jsonObject.put("data", keyValuePair);
            writer.write(jsonObject.toJSONString().getBytes());
            writer.flush();
            writer.close();
        } catch (IOException e) {
            LOGGER.severe("Error while saving the file to memory." + e.getMessage());
        }
    }

    //Generates the structure of a default memory object
    private Object defaultMemoryObject() {
        Map<String, Object> defaultMemoryObject = new HashMap<>();
        return defaultMemoryObject;
    }

    //Creates a client connection for a given port.
    private void createConnection(Integer replicaPort) {
        try {
            TTransport transport = new TSocket(this.hostAddress, replicaPort);
            transport.open();

            TProtocol protocol = new TBinaryProtocol(transport);

            RPCPacketService.Client client = new RPCPacketService.Client(protocol);
            if (client.ping() == MessageType.SUCCESS)
                this.replicas.put(client, true);
        } catch (TTransportException e) {
            LOGGER.severe("Error creating connection to the client: " + e.getMessage());
        } catch (TException e) {
            LOGGER.warning("Error creating connection to replica server on port: " + replicaPort);
        }
    }

    //Checks the existence of connection with replicas
    //or creates new ones if one doesn't exist.
    private void checkOrCreateConnection() throws TException {
        //For each replica port.
        if (replicas == null) {
            this.replicas = new HashMap<>();
            for (Integer replicaPort : replicaPorts) {
                createConnection(replicaPort);
            }
        }
        //Ping replicas to verify their availability.
        else {
            for (RPCPacketService.Client replica: this.replicas.keySet()) {
                if (replica.ping() != MessageType.SUCCESS) {
                    this.replicas.put(replica, false);
                } else {
                    this.replicas.put(replica, true);
                }
            }
        }
    }

    //For each of the replica, sends a proposal request.
    private Map<RPCPacketService.Client, RPCPacket> sendProposalToReplicas(Map<String, String> value, int operationType) throws TException {
        //Store responses to proposal sent back by replicas.
        Map<RPCPacketService.Client, RPCPacket> proposalResponses = new HashMap<>();

        for (RPCPacketService.Client replica: this.replicas.keySet()) {
            if (this.replicas.get(replica)) {
                //Construct the proposal message.
                RPCPacket proposal = new RPCPacket();
                proposal.sequence_number = LocalDateTime.now().toString();
                proposal.type = MessageType.PROPOSAL.getValue();
                proposal.keyValue = value;
                proposal.operationType = operationType;

                //Forward the proposal by invoking the call.
                RPCPacket packet = replica.proposal(proposal);
                proposalResponses.put(replica, packet);
            }
        }

        return proposalResponses;
    }

    //If the sequence is below the current highest sequence number,
    //the request is dropped.
    private boolean verifySequenceNumberForProcessing(String sequence_number) {
        return LocalDateTime.parse(sequence_number).isAfter(currentSequenceNumber);
    }

    //Creates a promise response packet based on the whether it has accepted
    //a proposal or current sequence number values
    private RPCPacket generatePromiseResponse(RPCPacket response, RPCPacket proposerData) {

        //Has promised other proposers?
        if (promiseStatus) {
            response.type = MessageType.PROMISE.getValue();
            response.sequence_number = agreedProposal;
            response.keyValue = agreedValue;
        } else {
            //update the agreed value, proposal number and promise status
            agreedValue = proposerData.keyValue;
            agreedProposal = proposerData.sequence_number;
            promiseStatus = true;

            response.type = MessageType.PROMISE.getValue();
            response.sequence_number = proposerData.sequence_number;
            response.keyValue = proposerData.keyValue;
        }


        return response;
    }

    //If the proposer receives the requested responses from a majority
    //of the acceptors, then it can issue a proposal with number n
    //and value v, where v is the value of the highest-numbered proposal
    //among the responses, or is any value selected by the proposer if
    //the responders reported no proposals.
    private Map<RPCPacketService.Client, RPCPacket> identifyProposalValue(Map<RPCPacketService.Client, RPCPacket> responses) {
        Iterator<Map.Entry<RPCPacketService.Client, RPCPacket>> entryIterator = responses.entrySet().iterator();

        while (entryIterator.hasNext()){

            Map.Entry<RPCPacketService.Client, RPCPacket> entry = entryIterator.next();

            //Check if there are any proposals from the acceptors. If not,
            //prepare to issue an accept request to the acceptors by removing failed
            //client responses.
            if (entry.getValue().type == MessageType.PROMISE.getValue()) {
                //When the proposed value is greater than the current highest
                //proposal value, accept the proposal.
                if (currentSequenceNumber.isBefore(LocalDateTime.parse(entry.getValue().sequence_number))) {
                    currentSequenceNumber = LocalDateTime.parse(entry.getValue().sequence_number);
                    valueOfHighestProposal = entry.getValue().keyValue;
                }
            } else {
                entryIterator.remove();
            }
        }

        return responses;
    }

    //Calculates majority based on the filtered responses and the total number of replicas
    //available for the proposer.
    private boolean calculateMajority(Map<RPCPacketService.Client, RPCPacket> responses) {
        int totalAvailableReplicas = replicas.size();
        int totalResponses = responses.size();

        //Majority is when the total responses has at least 3/4ths of
        //majority.
        return totalResponses >= totalAvailableReplicas * 3/4;
    }

    //Send accept proposals to the promised acceptors.
    private boolean sendAcceptToReplicas(Map<String, String> value, Map<RPCPacketService.Client, RPCPacket> responses, int operationType) throws TException {
        //Loop through the accept responses.
        RPCPacket acceptProposal = new RPCPacket();
        acceptProposal.type = MessageType.ACCEPT_REQUEST.getValue();
        acceptProposal.keyValue = value;
        acceptProposal.sequence_number = currentSequenceNumber.toString();
        acceptProposal.operationType = operationType;


        Iterator<Map.Entry<RPCPacketService.Client, RPCPacket>> entryIterator = responses.entrySet().iterator();
        while (entryIterator.hasNext()){
            Map.Entry<RPCPacketService.Client, RPCPacket> entry = entryIterator.next();

            //If acceptor is available
            if (this.replicas.get(entry.getKey())) {
                RPCPacket packet = entry.getKey().proposal(acceptProposal);

                if (packet.type == MessageType.FAILURE.getValue()) {
                    entryIterator.remove();
                } else {
                    responses.put(entry.getKey(), packet);
                }
            }
        }

        return calculateMajority(responses);
    }

    //If an acceptor receives an accept request for
    //a proposal numbered n, it accepts the proposal
    //unless it has already responded to a prepare request
    //having a number greater than n.
    private RPCPacket processAcceptProposal(RPCPacket message) {
        //Check if the proposed sequence number
        //is greater than the current sequence number (which represents
        //the sequence number of the latest accepted proposal)
        if (!LocalDateTime.parse(message.sequence_number).isBefore(currentSequenceNumber)) {
            //The acceptor has learned the value successfully.
            writeOrDelete(message.operationType, message.keyValue);

            //Reply to the proposer with a success.
            message.type = MessageType.SUCCESS.getValue();
        } else {
            message.type = MessageType.FAILURE.getValue();
        }

        return message;
    }

    //Writes or deletes key value from the current server store.
    private void writeOrDelete(int operationType, Map<String, String> keyValue) {
        if (operationType == OperationType.WRITE.getValue()) {
            this.keyValuePair.putAll(keyValue);
            writeToMemory(this.keyValuePair);
        } else if (operationType == OperationType.DELETE.getValue()) {
            for (String key : keyValue.keySet()) {
                this.keyValuePair.remove(key);
            }

            writeToMemory(this.keyValuePair);
        }
    }

    private RPCPacket processProposalRequest(RPCPacket message) {
        boolean canProcess = verifySequenceNumberForProcessing(message.sequence_number);

        //Response packet to construct based on processing.
        RPCPacket response = new RPCPacket();

        //If can't process, return a failure response to proposer
        //to increase efficiency rather wait on time out.
        if(!canProcess) {
            response.type = MessageType.FAILURE.getValue();
            return response;
        }

        //If the acceptor receives a prepare message,
        //it responds to the request with a promise not
        //to accept any more proposals numbered less than n
        //and with the highest-numbered proposal (if any)
        //that it has accepted
        return generatePromiseResponse(response, message);
    }

    private Map<String, Integer> getMajorityValue(String key, Map<String, Integer> majorityValue) throws IOException, TException {

        for (RPCPacketService.Client replica : this.replicas.keySet()) {
            //If replica is available
            if (this.replicas.get(replica)) {
                String response = replica.getValueFromMemory(key);

                //If the value exists, increment its count
                if (majorityValue.containsKey(response)) {
                    int value = majorityValue.get(response);
                    majorityValue.put(response, ++value);
                } else {
                    //Else add an entry.
                    majorityValue.put(response, 1);
                }
            }
        }

        return majorityValue;
    }

    @Override
    public RPCPacket proposal(RPCPacket message) throws TException {
        if (message.type == MessageType.ACCEPT_REQUEST.getValue()) {
            return processAcceptProposal(message);
        } else if (message.type == MessageType.PROPOSAL.getValue()){
            return processProposalRequest(message);
        }

        return message;
    }

    @Override
    public List<String> replicaAddresses() throws TException {
        //For each replica port number
        List<String> ports = new ArrayList<>();
        for (Integer portNum : this.replicaPorts) {
            ports.add(portNum.toString());
        }

        return ports;
    }

    @Override
    public MessageType write(Map<String, String> value, int operationType) throws TException {
        checkOrCreateConnection();
        promiseStatus=false;

        //Initiate proposal to all the replicas.
        Map<RPCPacketService.Client, RPCPacket> responses = sendProposalToReplicas(value, operationType);

        //Prepare to issue accept requests to acceptors.
        responses = identifyProposalValue(responses);

        //Check if the proposer has a majority.
        boolean hasMajority = calculateMajority(responses);

        //If it has majority, then send accept requests to
        //the acceptors.
        if (hasMajority) {
            if (sendAcceptToReplicas(value, responses, operationType)) {
                //When all the proccesses have committed the value to their memory,
                //save the value to the proposer memory.
                writeOrDelete(operationType, value);

                return MessageType.SUCCESS;
            }
        }

        return MessageType.FAILURE;
    }

    @Override
    public List<String> getKeys() throws TException {
        try {
            this.keyValuePair = (Map<String, String>) loadMemoryObject(1);
            return new ArrayList<>(keyValuePair.keySet());
        } catch (IOException e) {
            LOGGER.severe("Error loading memory object");
            throw new TException("Error loading memory object: " + e.getMessage());
        }
    }

    @Override
    public String getValue(String key) throws TException {
        try {
            checkOrCreateConnection();

            Map<String, Integer> majorityValue = new HashMap<>();
            majorityValue.put(getValueFromMemory(key), 1);
            //Contact other replicas and get the value.
            majorityValue = getMajorityValue(key, majorityValue);

            //Majority value is
            String currentKey="";
            int maxCount = 0;
            for (String value: majorityValue.keySet()) {
                if (maxCount < majorityValue.get(value)) {
                    maxCount = majorityValue.get(value);
                    currentKey = value;
                }
            }

            return currentKey;
        } catch (IOException e) {
            LOGGER.severe("Error loading memory object");
            throw new TException("Error loading memory object: " + e.getMessage());
        }
    }

    @Override
    public String getValueFromMemory(String key) throws TException {
        try {
            //Get the value from the current system.
            this.keyValuePair = (Map<String, String>) loadMemoryObject(1);
            return this.keyValuePair.getOrDefault(key, MessageType.FAILURE.toString());
        } catch (IllegalStateException | IOException e) {
            LOGGER.severe("Error loading memory object");
            throw new TException("Error loading memory object: " + e.getMessage());
        }

    }

    @Override
    public MessageType ping() throws TException {
        LOGGER.info("Ping message received");
        return MessageType.SUCCESS;
    }
}
