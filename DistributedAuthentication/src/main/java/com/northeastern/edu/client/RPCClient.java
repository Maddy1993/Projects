package com.northeastern.edu.client;

import generated.thrift.impl.CommunicationService;
import generated.thrift.impl.MessageType;
import generated.thrift.impl.OperationType;
import generated.thrift.impl.RequestPacket;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TTransportException;

import java.io.IOException;
import java.time.LocalDateTime;
import java.util.*;
import java.util.logging.FileHandler;
import java.util.logging.Logger;
import java.util.stream.Stream;

/**
 * Represents an RPC Client implemented as Apache Thrift client.
 *
 * @author mpothukuchi 12th June, 2019
 *
 * References:
 * @see <a href="https://thrift.apache.org/tutorial/java" />
 * @see <a href="https://www.baeldung.com/apache-thrift" />
 */
public class RPCClient {

    //Logger for the class.
    private static Logger LOGGER = Logger.getLogger(RPCClient.class.getName());

    //Port Number of server.
    private static Integer portNumber;

    //Server address parsed.
    private static String serverAddress;

    //Replica Server Communication Ports.
    private static Map<CommunicationService.Client, Boolean> clients;

    //Running Server
    private static CommunicationService.Client availableServer;

    //Random element seed
    private static Random rand;

    //Name of the client.
    private static String clientName;

    /**
     * Formats the message to be printed to the output stream.
     *
     * @param message The message to print to screen
     */
    private static void formatMessage(String  message) {
        System.out.print("<" + LocalDateTime.now() + ">> ");
        System.out.println(message);
    }

    /**
     * Used for any setup required before the program execution
     * starts.
     */
    private static void setup(String fileName) {
        //Initialize the logger.
        try {
            LOGGER.setUseParentHandlers(false);
            LOGGER.addHandler(new FileHandler(fileName));
            rand = new Random();
        } catch (IOException e) {
            System.out.println("Error initializing the logger: " + e.getMessage());
        }
    }

    /**
     * Used to parse the server address and port number
     * arguments passed to the client.
     * @param args The command line arguments passed to the program.
     */
    private static void parseArguments(String[] args) {
        if (args.length == 3) {
            try {
                serverAddress = args[0];
                portNumber = Integer.parseInt(args[1]);
                clientName = args[2];
            } catch (NumberFormatException e) {
                LOGGER.severe("The arguments format doesn't match: " + e.getMessage());
                formatMessage("The port number argument type don't match.");
                System.exit(1);
            }
        } else {
            LOGGER.severe(
                    "The client expects the arguments in the following format."
                            + "\nClient_TCP.java <server_address> <server_port>");
            formatMessage("The client expects the arguments in the following format."
                    + "\nClient_TCP.java <server_address> <server_port>");
            System.exit(1);
        }
    }

    public static void main(String[] args) {
        setup("logs/RPCClient.log");
        parseArguments(args);

        LOGGER.info("Client pre-processing complete.");

        //Initializing the RPC Client.
        try {
            TTransport transport = new TSocket(serverAddress, portNumber);
            transport.open();

            TProtocol protocol = new TBinaryProtocol(transport);
            CommunicationService.Client client = new CommunicationService.Client(protocol);

            //Runs in a while loop until a quit input is received from
            //the user.
            clients = new HashMap<>();
            generateClients(client.replicaAddresses());

            //Gets the random client once all the addresses
            //are retrieved
            CommunicationService.Client randomClient = (CommunicationService.Client) clients.keySet().toArray()[rand.nextInt(clients.size())];
            perform(randomClient);

            //Close once the method returns.
            transport.close();
            LOGGER.info("Connection closed with server");
            formatMessage("Connection ended with server");
        } catch (TTransportException e) {
            LOGGER.severe("Error opening a channel with the server on: " + serverAddress + "-> " + portNumber);
            LOGGER.severe("Exception while opening channel: " + e.getMessage());
            formatMessage("Error opening a channel with the server on: " + serverAddress + "-> " + portNumber);
            formatMessage("Exception while opening channel: " + e.getMessage());
            System.exit(-1);
        } catch (TException e) {
            String message = "Error connecting to server- " + serverAddress + ":" + portNumber + ":: " + e.getMessage();
            LOGGER.severe(message);
            formatMessage(message);
            System.exit(-1);
        }
    }

    //Creates server instances for the replicas and maintains the health of the
    //instances.
    private static void generateClients(List<String> replicaAddresses) throws TTransportException {
        //For each replica
        for (String replicaPort : replicaAddresses) {
            TTransport transport = new TSocket(serverAddress, Integer.parseInt(replicaPort));
            transport.open();

            TProtocol protocol = new TBinaryProtocol(transport);
            CommunicationService.Client client = new CommunicationService.Client(protocol);
            clients.put(client, true);
        }
    }

    /**
     * Contains the logic behind client implementation.
     *
     * @param server The thrift processor which represents the client.
     */
    private static void perform(CommunicationService.Client server) throws TException {

        availableServer = server;

        performLogin(server);

        //Login with the connecting server before performing operations
        do {
            Scanner input = new Scanner(System.in);
            formatMessage("\nPlease choose the operation to perform:");
            System.out.println("\t1. Get Data.\n\t2. Save Data.\n\t3. Delete data.\n\t4. Quit");
            System.out.println("\nEnter the option number: ");


            int option = 0;
            try {
                option = input.nextInt();
            } catch (NoSuchElementException e) {
                LOGGER.warning("Error reading integer: " + e.getMessage());
            }

           performSelectedOperation(option, availableServer, input);
        } while (true);
    }

    /**
     * Performs the login
     * @param server
     */
    private static void performLogin(CommunicationService.Client server) throws TException {
        //Login with the connecting server before performing operations
        Scanner input = new Scanner(System.in);
        formatMessage("\nEnter the Password to Login: \n");

        String password;
        do {
            formatMessage("(Length of the password needs to be 3 or smaller and should not be empty)\n");
            password = input.nextLine();
        } while (password.isEmpty() || password.length() > 3);

        RequestPacket response = server.login(password, clientName);
        if (response.type == MessageType.FAILURE) {
            System.out.println("LOGIN Failed with server: " + getAddressForClient(availableServer));
        } else if (response.type == MessageType.SUCCESS_WRITE) {
            System.out.println("\nSuccessfully authenticated\n");
        } else if (response.type == MessageType.SUCCESS) {
            System.out.println("\nSign Up Succeeded\n");
        }
    }

    //Performs the operation based on the input operation.
    private static void performSelectedOperation(int option, CommunicationService.Client server, Scanner input) throws TException {
        
        //Switch statement to handle the different cases.
        RequestPacket responseValue;
        String value;
        String key;
        int response;
        RequestPacket serverResponse;
        List<String> keys;
        switch (option) {
            case 1:
                try {
                    keys = server.getKeys();

                    if (keys.size() == 0) {
                        System.out.println("Server does not have keys in its store.");
                        break;
                    } else {
                        System.out.println("List of Available keys with the server:");
                        Stream.of(keys).forEach(x->System.out.println("\t" + x));
                    }

                    //Enter the key name from the available list.
                    System.out.println("\nEnter the key name: ");
                    do{
                        key = input.nextLine();
                    } while (key.isEmpty());

                    //Get the value from the server.
                    responseValue = server.getValueForKey(key);
                    if (responseValue.type == MessageType.SUCCESS) {
                        System.out.println("The value associated with key: " + key + " is " + responseValue.keyValue.get(key));
                    } else {
                        System.out.println("Key does not exist at: " + responseValue.keyValue.getOrDefault("Address", ""));
                    }
                    break;
                } catch (TException e) {
                    formatMessage("Error while retrieving keys from the server- " + getAddressForClient(server) + ": " + e.getMessage());
                    LOGGER.warning("Error while retrieving keys from the server- " + getAddressForClient(server) + ": " + e.getMessage());
                    clients.put(server, false);
                    performSelectedOperation(option, getAvailableServer(), input);
                    break;
                }
            case 2:
                //Read the data to store and then
                //the respective file name.
                System.out.println("\nEnter the data: ");
                do{
                    value = input.nextLine();
                } while (value.isEmpty());

                System.out.print("\nEnter the key name for the data: ");
                do{
                    key = input.nextLine();
                } while (key.isEmpty());

                //Add the key to keyStore at the server.
                try {
                    Map<String, String> mapValue = new HashMap<>();
                    mapValue.put(key, value);
                    serverResponse = server.storeKeyValue(mapValue, OperationType.WRITE);

                    if (serverResponse.type == MessageType.FAILURE) {
                        System.out.println("Error saving the keys at the server");
                    } else {
                        System.out.println("Key saved at the server.");
                    }

                    break;
                } catch (TException e) {
                    formatMessage("Error while saving keys from the server- " + getAddressForClient(server) + ": " + e.getMessage());
                    LOGGER.warning("Error while saving keys from the server- " + getAddressForClient(server) + ": " + e.getMessage());
                    clients.put(server, false);
                    performSelectedOperation(option, getAvailableServer(), input);
                    return;
                }
            case 3:
                try {
                    keys = server.getKeys();
                    if (keys.size() == 0) {
                        System.out.println("Server does not have keys in its store.");
                        break;
                    } else {
                        System.out.println("List of Available keys with the server:");
                        Stream.of(keys).forEach(x->System.out.println("\t" + x));
                    }

                    //Enter the key name from the available list.
                    System.out.println("\nEnter the key name to delete: ");
                    do{
                        key = input.nextLine();
                    } while (key.isEmpty());

                    //Get the response for deletion from the server.
                    serverResponse = server.deleteKey(key, OperationType.DELETE);

                    if (serverResponse.type == MessageType.SUCCESS) {
                        System.out.println("Key: " + key + " successfully removed");
                    } else if (serverResponse.type == MessageType.FAILURE){
                        System.out.println("Failed to remove key from Replicas.");
                    } else {
                        System.out.println("Invalid Response");
                        LOGGER.warning("Invalid response from delete operation");
                    }
                } catch (TException e) {
                    formatMessage("Error while deleting keys from the server- " + getAddressForClient(server) + ": " + e.getMessage());
                    LOGGER.warning("Error while deleting keys from the server- " + getAddressForClient(server) + ": " + e.getMessage());
                    clients.put(server, false);
                    performSelectedOperation(option, getAvailableServer(), input);
                    return;
                }
                break;
            case 4:
                System.exit(0);
            default:
                formatMessage("Invalid Option Entered.");
                LOGGER.warning("Invalid Option: " + option + "entered.");
        }
    }

    //Retrieves an available server with healthy status.
    private static CommunicationService.Client getAvailableServer() {
        //For retrieving an available server
        List<CommunicationService.Client> availableClients = new ArrayList<>();

        for (CommunicationService.Client client : clients.keySet()) {
            try {
                if (clients.get(client) && client.ping() == MessageType.SUCCESS) {
                    LOGGER.info("Connected to replica at: " + getAddressForClient(client));
                    formatMessage("Connected to replica at: " + getAddressForClient(client));
                    availableClients.add(client);
                    availableServer = client;
                    return client;
                }
            } catch (TException e) {
                String message = "Error pinging server: " + client.toString();
                LOGGER.warning(message + ": " + e);
                clients.put(client, false);
            }
        }

        LOGGER.severe("No Available Servers");
        formatMessage("No available servers");
        LOGGER.severe("Exiting program");
        formatMessage("Exiting program");
        System.exit(0);
        return null;
    }

    //Gets the socket address for a given client/server object.
    private static String getAddressForClient(CommunicationService.Client client) {
        TSocket socket = (TSocket) client.getOutputProtocol().getTransport();
        return socket.getSocket().getRemoteSocketAddress().toString();
    }
}
