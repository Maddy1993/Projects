#Chat Application: Establishing Anonymous Security

The system is a chat application between client and server.

## Server
The server is a UDP server which has the capability to process LOGIN and LIST requests from multiple clients. On receiving the LOGIN request, server validates the username
in the packet and if the does not exist in its users dictionary, it stores its details in the dictionary and sends the response about the same. If the user exists, the server responds with a
user exists message. Similarly, when the SERVER receives a LIST message from the clients, it lists the users/clients who have contacted the server previously and logged into the system.

## Client
The client is a UDP Client which has simultaneous communication capabilities with a different UDP Client and a Server. It represents a building block in the UDP Chat Application.
A function of the Client is designed to send a request to Server to retrieve all the logged in users of the chat application. A function of the client is designed to send a message to one of
the logged in users. If the user is not logged in, necessary exception messages are displayed. 
