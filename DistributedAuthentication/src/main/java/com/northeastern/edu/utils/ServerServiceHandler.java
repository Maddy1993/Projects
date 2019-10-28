package com.northeastern.edu.utils;

import generated.thrift.impl.CommunicationService;
import generated.thrift.impl.MessageType;
import generated.thrift.impl.OperationType;
import generated.thrift.impl.ServerPacket;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TTransportException;

import java.io.IOException;
import java.time.LocalDateTime;
import java.util.*;
import java.util.logging.Logger;

public class ServerServiceHandler extends ServiceHandler {

    //Logger for the class.
    private static Logger LOGGER = Logger.getLogger(ServerServiceHandler.class.getName());

    //Replica Connections. Client-Availability mapping
    private Map<CommunicationService.Client, Boolean> replicas;


    //Variable to represent the maximum value seen so far.
    private static Map<String, String> valueOfHighestProposal;

    //Flag to represent the promise status.
    private static boolean promiseStatus;

    //Variable representing the agreed upon value.
    private static Map<String, String> agreedValue;

    //Variable representing the agreed upon sequence number.
    private static String agreedProposal;

    //Constructor to initialize the addresses of server replicas.
    //and sequence number to initiate paxos.
    public ServerServiceHandler(List<Integer> replicaPorts, Integer portNumber) throws IOException {
        super(replicaPorts, portNumber);
    }

    //Creates a client connection for a given port.
    private void createConnection(Integer replicaPort) {
        try {
            TTransport transport = new TSocket(this.hostAddress, replicaPort);
            transport.open();

            TProtocol protocol = new TBinaryProtocol(transport);

            CommunicationService.Client client = new CommunicationService.Client(protocol);
            if (client.ping() == MessageType.SUCCESS)
                this.replicas.put(client, true);
        } catch (TTransportException e) {
            LOGGER.severe("Error creating connection to the server: " + e.getMessage());
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
            for (CommunicationService.Client replica : this.replicas.keySet()) {
                if (replica.ping() != MessageType.SUCCESS) {
                    this.replicas.put(replica, false);
                } else {
                    this.replicas.put(replica, true);
                }
            }
        }
    }

    //For each of the replica, sends a proposal request.
    private Map<CommunicationService.Client, ServerPacket> sendProposalToReplicas
    (Map<String, String> value, OperationType operationType) throws TException {
        //Store responses to proposal sent back by replicas.
        Map<CommunicationService.Client, ServerPacket> proposalResponses = new HashMap<>();

        for (CommunicationService.Client replica : this.replicas.keySet()) {
            if (this.replicas.get(replica)) {
                //Construct the proposal message.
                ServerPacket proposal = new ServerPacket();
                proposal.sequence_number = LocalDateTime.now().toString();
                proposal.type = MessageType.PROPOSAL;
                proposal.proposalValue = value;
                proposal.operationType = operationType;

                //Forward the proposal by invoking the call.
                ServerPacket packet = replica.acceptProposal(proposal);
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
    private ServerPacket generatePromiseResponse(ServerPacket response, ServerPacket proposerData) {

        //Has promised other proposers?
        if (promiseStatus) {
            response.type = MessageType.PROMISE;
            response.sequence_number = agreedProposal;
            response.proposalValue = agreedValue;
        } else {
            //update the agreed value, proposal number and promise status
            agreedValue = proposerData.proposalValue;
            agreedProposal = proposerData.sequence_number;
            promiseStatus = true;

            response.type = MessageType.PROMISE;
            response.sequence_number = proposerData.sequence_number;
            response.proposalValue = proposerData.proposalValue;
        }


        return response;
    }

    //If the proposer receives the requested responses from a majority
    //of the acceptors, then it can issue a proposal with number n
    //and value v, where v is the value of the highest-numbered proposal
    //among the responses, or is any value selected by the proposer if
    //the responders reported no proposals.
    private Map<CommunicationService.Client, ServerPacket> identifyProposalValue
    (Map<CommunicationService.Client, ServerPacket> responses) {
        Iterator<Map.Entry<CommunicationService.Client, ServerPacket>> entryIterator = responses.entrySet().iterator();

        while (entryIterator.hasNext()) {

            Map.Entry<CommunicationService.Client, ServerPacket> entry = entryIterator.next();

            //Check if there are any proposals from the acceptors. If not,
            //prepare to issue an accept request to the acceptors by removing failed
            //client responses.
            if (entry.getValue().type == MessageType.PROMISE) {
                //When the proposed value is greater than the current highest
                //proposal value, accept the proposal.
                if (currentSequenceNumber.isBefore(LocalDateTime.parse(entry.getValue().sequence_number))) {
                    currentSequenceNumber = LocalDateTime.parse(entry.getValue().sequence_number);
                    valueOfHighestProposal = entry.getValue().proposalValue;
                }
            } else {
                entryIterator.remove();
            }
        }

        return responses;
    }

    //Calculates majority based on the filtered responses and the total number of replicas
    //available for the proposer.
    private boolean hasMajority(Object responses) {

        int totalAvailableReplicas = replicas.size();
        if (responses instanceof Map)
        {
            int totalResponses = ((Map) responses).size();

            //Majority is when the total responses has at least 3/4ths of
            //majority.
            return totalResponses >= totalAvailableReplicas * 3 / 4;
        } else if (responses instanceof Integer) {
            return ((Integer)responses) >= totalAvailableReplicas * 3/4;
        } else {
            LOGGER.severe("Invalid response object type: " + responses.getClass().getName());
            return false;
        }

    }

    //Send accept proposals to the promised acceptors.
    private boolean sendAcceptToReplicas
    (Map<String, String> value,
     Map<CommunicationService.Client, ServerPacket> responses,
     OperationType operationType) throws TException {
        //Loop through the accept responses.
        ServerPacket acceptProposal = new ServerPacket();
        acceptProposal.type = MessageType.ACCEPT_REQUEST;
        acceptProposal.proposalValue = value;
        acceptProposal.sequence_number = currentSequenceNumber.toString();
        acceptProposal.operationType = operationType;


        Iterator<Map.Entry<CommunicationService.Client, ServerPacket>> entryIterator = responses.entrySet().iterator();
        while (entryIterator.hasNext()) {
            Map.Entry<CommunicationService.Client, ServerPacket> entry = entryIterator.next();

            //If acceptor is available
            if (this.replicas.get(entry.getKey())) {
                ServerPacket packet = entry.getKey().acceptProposal(acceptProposal);

                if (packet.type == MessageType.FAILURE) {
                    entryIterator.remove();
                } else {
                    responses.put(entry.getKey(), packet);
                }
            }
        }

        if (hasMajority(responses))
        {
            try {
                if (operationType != OperationType.LOGIN) {
                    super.writeToMemory(value);
                }
            } catch (IOException e) {
                throw new TException(e.getMessage());
            }
            return true;
        }

        return false;
    }

    //If an acceptor receives an accept request for
    //a proposal numbered n, it accepts the proposal
    //unless it has already responded to a prepare request
    //having a number greater than n.
    private ServerPacket processAcceptProposal(ServerPacket message) throws IOException, TException {
        //Check if the proposed sequence number
        //is greater than the current sequence number (which represents
        //the sequence number of the latest accepted proposal)
        if (!LocalDateTime.parse(message.sequence_number).isBefore(currentSequenceNumber)) {
            //The acceptor has learned the value successfully.
            this.keyValuePair = (Map<String, String>) loadMemoryObject(1);
            if (this.keyValuePair == null) {
                this.keyValuePair = new HashMap<>();
            }

            this.clientKeys = (Map<String, List<String>>) loadKeysMemoryObject(1);
            if (message.operationType == OperationType.DELETE) {
                this.keyValuePair = (Map<String, String>) loadMemoryObject(1);

                for (String key : message.proposalValue.keySet()) {
                    this.keyValuePair.remove(key);
                }

                super.writeToMemory(this.keyValuePair);
            } else if(message.operationType == OperationType.WRITE) {
                this.keyValuePair.putAll(message.proposalValue);
                super.writeToMemory(this.keyValuePair);
            } else if (message.operationType == OperationType.LOGIN) {

                //For each key in the proposal value.
                if (this.clientKeys == null) {
                    this.clientKeys = new HashMap<>();
                }

                for (String key: message.proposalValue.keySet()) {

                    if (this.clientKeys.containsKey(key)) {
                        List<String> value = this.clientKeys.get(key);

                        if (value.size() >=3) {
                            value = new ArrayList<>();
                        }

                        value.add(message.proposalValue.get(key));

                        this.clientKeys.put(key, value);

                    } else {
                        List<String> value = new ArrayList<>();
                        value.add(message.proposalValue.get(key));
                        this.clientKeys.put(key, value);
                    }

                    //Add the commit time to memory.
                    Map<String, String> commitTime = new HashMap<>();
                    commitTime.put(key, LocalDateTime.now().toString());
                    writeCommitTimeToMemory(commitTime);
                }
                super.writeKeysToMemory(this.clientKeys);
            }

            //Reply to the proposer with a success.
            promiseStatus = false;
            message.type = MessageType.SUCCESS;
        } else {
            message.type = MessageType.FAILURE;
        }

        return message;
    }

    private ServerPacket processProposalRequest(ServerPacket message) {
        boolean canProcess = verifySequenceNumberForProcessing(message.sequence_number);

        //Response packet to construct based on processing.
        ServerPacket response = new ServerPacket();

        //If can't process, return a failure response to proposer
        //to increase efficiency rather wait on time out.
        if (!canProcess) {
            response.type = MessageType.FAILURE;
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

        for (CommunicationService.Client replica : this.replicas.keySet()) {
            //If replica is available
            if (this.replicas.get(replica)) {
                String response = replica.getStoredValue(key);

                //If the value exists, increment its count
                if (!response.isEmpty()) {
                    if (majorityValue.containsKey(response)) {
                        int value = majorityValue.get(response);
                        majorityValue.put(response, ++value);
                    } else {
                        //Else add an entry.
                        majorityValue.put(response, 1);
                    }
                } else {
                    String message = String.format("Value for key: {0} is not found at {1}", key, getAddressForClient(replica));
                    LOGGER.warning(message);
                }
            }
        }

        return majorityValue;
    }

    //Gets the socket address for a given client/server object.
    private String getAddressForClient(CommunicationService.Client client) {
        TSocket socket = (TSocket) client.getOutputProtocol().getTransport();
        return socket.getSocket().getRemoteSocketAddress().toString();
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

    //Initiates the Paxos algorithm to decide if the operation
    //requested can be performed in a distributed fashion.
    @Override
    protected boolean canWriteOrDelete(Map<String, String> value, OperationType operationType) throws TException {
        checkOrCreateConnection();
        promiseStatus = false;

        //Initiate proposal to all the replicas.
        Map<CommunicationService.Client, ServerPacket> responses = sendProposalToReplicas(value, operationType);

        //Prepare to issue accept requests to acceptors.
        responses = identifyProposalValue(responses);

        //Check if the proposer has a majority.
        boolean hasMajority = hasMajority(responses);

        //If it has majority, then send accept requests to
        //the acceptors.
        return hasMajority
                && sendAcceptToReplicas(value, responses, operationType);
    }

    @Override
    protected String getValue(String key) throws TException {
        try {
            checkOrCreateConnection();

            Map<String, Integer> majorityValue = new HashMap<>();
            majorityValue.put(getStoredValue(key), 1);
            //Contact other replicas and get the value.
            majorityValue = getMajorityValue(key, majorityValue);

            //Majority value is
            String currentKey = "";
            int maxCount = 0;
            for (String value : majorityValue.keySet()) {
                if (maxCount < majorityValue.get(value)) {
                    maxCount = majorityValue.get(value);
                    currentKey = value;
                }
            }

            if (hasMajority(maxCount))
            {
                return currentKey;
            } else {
                return "";
            }

        } catch (IOException e) {
            LOGGER.severe("Error loading memory object");
            throw new TException("Error loading memory object: " + e.getMessage());
        }
    }

    List<String> getAddresses() {
        //For each replica port number
        List<String> ports = new ArrayList<>();
        for (Integer portNum : this.replicaPorts) {
            ports.add(portNum.toString());
        }

        return ports;
    }

    @Override
    public ServerPacket acceptProposal(ServerPacket packet) throws TException {
        try {
            if (packet.type == MessageType.ACCEPT_REQUEST) {
                return processAcceptProposal(packet);
            } else if (packet.type == MessageType.PROPOSAL) {
                return processProposalRequest(packet);
            }

            return packet;
        } catch (IOException e) {
            String message = String.format("Error while processing: {0}: {1}",packet.type.toString(), e.getMessage());
            LOGGER.severe(message);
            throw new TException(message);
        }
    }

    @Override
    public MessageType ping() throws TException {
        LOGGER.info("Ping message received");
        return MessageType.SUCCESS;
    }

    @Override
    public String getStoredValue(String key) throws TException {
        //Get the value from the current system.
        try {
            this.keyValuePair = (Map<String, String>) super.loadMemoryObject(1);
            return this.keyValuePair.getOrDefault(key, "");
        } catch (IOException e) {
            LOGGER.severe("Error reading file from memory: " + e.getMessage());
            return "";
        }
    }
}
