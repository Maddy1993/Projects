# Distributed Authentication

## MultiThreadedRPCClientServer

The implementation is a multi-threaded client server which uses RPC. The mechanism used in this project is Apache Thrift RPC.
The implementation maintains 4 different replicas of the given server making the total available servers to client as 5.
Each PUT or DELETE operation between the replicas is communicated using the 2PC commit algorithm.

Algorithm Reference: http://courses.cs.vt.edu/~cs5204/fall00/distributedDBMS/duckett/tpcp.html

Design:
    Server:
        * Server is designed to talk to client on a specific port and communication commits to other servers on a different port.
        * It establishes connection with other replicas once a PUT or DELETE request is made by the client.
        * All the replicas will have an initial state represented by default memory object in the first pass.
        * The implementation is designed to roll back the on-going PUT or DELETE operation when a server identifies a lost replica.
            It works only when all the replicas are available.

    Client:
        * Client is designed to take the server address, and port numbers of the master server.
        * The master provides the details of all the replicas during the connection setup.
        * The client falls back to the replica only for a GET operation. In case of PUT or DELETE, though the
            request is made to the available replica, the result given to the client will unsuccessful.

Syncing the directory:
    1. The project is available at https://github.com/Maddy1993/ScalableDistributedSystems/tree/master/Project-3. Please clone the directory to access it.
    2. Run Maven clean install to install the dependencies
        mvn clean install

Program Execution:
    Server(n):
        javac src/main/java/com/northeastern/edu/server/RPCServer.java <client_communication_port> <server1_communication_port> <server2_communication_port> <server3_communication_port> <server4_communication_port> <server5_communication_port>
        java src/main/java/com/northeastern/edu/server/RPCServer

    Server1:
        src/main/java/com/northeastern/edu/server/RPCServer.java 10001 10010 10020 10030 10040 10050
        java src/main/java/com/northeastern/edu/server/RPCServer
    Server2:
        src/main/java/com/northeastern/edu/server/RPCServer.java 10002 10020 10010 10030 10040 10050
        java src/main/java/com/northeastern/edu/server/RPCServer
    Server3:
        src/main/java/com/northeastern/edu/server/RPCServer.java 10003 10030 10010 10020 10040 10050
        java src/main/java/com/northeastern/edu/server/RPCServer
    Server4:
        src/main/java/com/northeastern/edu/server/RPCServer.java 10004 10040 10010 10030 10020 10050
        java src/main/java/com/northeastern/edu/server/RPCServer
    Server5:
        src/main/java/com/northeastern/edu/server/RPCServer.java 10005 10050 10010 10030 10040 10020
        java src/main/java/com/northeastern/edu/server/RPCServer

    Client:
        javac src/main/java/com/northeastern/edu/client/RPCClient.java 127.0.0.1 10001
        java src/main/java/com/northeastern/edu/client/RPCClient
