package com.northeastern.edu.server;

import com.northeastern.edu.utils.ServerServiceHandler;
import generated.thrift.impl.CommunicationService;
import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TThreadPoolServer;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TServerTransport;
import org.apache.thrift.transport.TTransportException;

import java.io.IOException;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.FileHandler;
import java.util.logging.Logger;

/**
 * Represents a RPC server implemented on Apache Thrift.
 *
 * @author mpothukuchi 12th June 2019
 *
 * References:
 * @see <a href="https://thrift.apache.org/tutorial/java" />
 * @see <a href="https://www.baeldung.com/apache-thrift" />
 */
public class RPCServer {

    //Logger for the class.
    private static Logger LOGGER = Logger.getLogger(RPCServer.class.getName());

    //Port Number for the server
    private static Integer portNumber;

    //Port Number for server to server communication.
    private static Integer serverCommunicationPortNumber;

    //List of Port Numbers to contact the servers hosted on localhost.
    private static List<Integer> serverCommunicationPorts = new ArrayList<>(4);

    //Handler for incoming client requests.
    private static ServerServiceHandler utils;

    //Processor for RPC service class.
    private static CommunicationService.Processor processor;

    /**
     * Used for any setup required before the program execution
     * starts.
     *
     * @param name  Name of the logger to use for the
     *              overriding class.
     */
    static void setup(String name) {
        //Initialize the logger.
        try {
            LOGGER.setUseParentHandlers(false);
            LOGGER.addHandler(new FileHandler(name));
        } catch (IOException e) {
            System.out.println("Error initializing the logger: " + e.getMessage());
        }
    }

    /**
     * Can be used to parse any command line arguments.
     * Currently parses the port number from the command
     * line arguments.
     * @param args  List of command line arguments
     */
    static void parseArguments(String[] args) {
        if (args.length == 6) {
            try {
                portNumber = Integer.parseInt(args[0]);
                serverCommunicationPortNumber = Integer.parseInt(args[1]);
                serverCommunicationPorts.add(Integer.parseInt(args[2]));
                serverCommunicationPorts.add(Integer.parseInt(args[3]));
                serverCommunicationPorts.add(Integer.parseInt(args[4]));
                serverCommunicationPorts.add(Integer.parseInt(args[5]));
            } catch (NumberFormatException exception) {
                LOGGER.severe("Port numbers needs to be an integer.");
                System.exit(1);
            }
        } else {
            LOGGER.severe("The server needs the following inputs only:\n\t1. Port Number\n\t2. Port Number for Server-to-Server communication" +
                    "\n\t3. List of 4 port numbers the replicas are hosted on localhost");
            System.exit(1);
        }
    }

    /**
     * Formats the message to be printed to the output stream.
     *
     * @param message The message to print to screen
     */
    private static void formatMessage(String  message) {
        System.out.print("<" + LocalDateTime.now() + ">> ");
        System.out.println(message);
    }

    public static void main(String[] args) {
        setup("logs/RCPServer.log");
        parseArguments(args);

        try {
            //Initialize the client handler.
            utils = new ServerServiceHandler(serverCommunicationPorts, portNumber);

            //Initialize the processor for thrift server.
            processor = new CommunicationService.Processor<>(utils);

            Runnable simple = () -> threadedServer(processor);
            Runnable protocolServer = () -> threadedServerCommunication(processor);

            new Thread(simple).start();
            new Thread(protocolServer).start();
        } catch (IllegalStateException e) {
            LOGGER.warning(e.getMessage());
            formatMessage("Exception in main block: " + e.getMessage());
        } catch (Exception e) {
            LOGGER.warning("Exception in main block: " + e.getMessage());
            formatMessage("Exception in main block: " + e.getMessage());
        }
    }

    /**
     * Creates a single threaded server, listening on the port specified
     * for communication between two servers.
     *
     * @param protocol Thrift processor to initialize the server.
     */
    private static void threadedServerCommunication(CommunicationService.Processor protocol) {
        try {
            TServerTransport serverTransport = new TServerSocket(serverCommunicationPortNumber);
            TServer server = new TThreadPoolServer(new TThreadPoolServer.Args(serverTransport).processor(protocol));

            LOGGER.info("Server-to-Server initialized");
            formatMessage("Server-to-Server initialized");
            server.serve();
        } catch (TTransportException e) {
            LOGGER.severe("Error initializing simple server socket: " + e.getMessage());
            formatMessage("Error initializing server socket: " + e.getMessage());
        }
    }

    /**
     * Creates the multi threaded server and initializes it for listening on
     * the specified port.
     *
     * @param processor Thrift processor to initialize the server with.
     */
    private static void threadedServer(CommunicationService.Processor processor) {
        try {
            TServerTransport serverTransport = new TServerSocket(portNumber);

            TServer server = new TThreadPoolServer(new TThreadPoolServer.Args(serverTransport).processor(processor));

            LOGGER.info("Multi-threaded thrift server initialized");
            formatMessage("Multi-threaded thrift server initialized");
            server.serve();
        } catch (TTransportException e) {
            LOGGER.severe("Error initializing server socket: " + e.getMessage());
            formatMessage("Error initializing server socket: " + e.getMessage());
        }
    }
}
