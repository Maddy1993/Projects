# _**Client-Server Model**_:

The Client-Server Model is a generic model which can be used to establish a connection between the client and server.

For the client, when we pass _./makeClient <-p port> [hostname] [ID]"_, it attempts to connect to [hostname] on <-p port>.

If the connection is successful,
the client sends a HELLO Message to the server. Once the client receives the
STATUS message, the client performs the calculations and forwards the same to
the server. If every message sent by the client is according to the server
guidelines, the server terminates the connection by sending a BYE message with
a 64-bit secret flag.


## Client:
### Testing:

Connection to the server:
        ./client -p <PORT> <server_address> <ID>

We will receive a secret-flag if the operations are successful.
	92c63f67c0bc9f87cb6224ca4a6b24aa82c75e76ac5170fce5594618f586edf6

(The secret flag depends on the values provided at the server)


## Server:
### Testing:


Executing the server file to listen on 27993 port.
      * ./makeServer -p <PORT>  <Binding_address> <Student_ID> <Secret_Flag>

If all the  messages received by the server are accoridng to it
guidelines, the server will output the BYE message

     cs5700fall2017 92c63f67c0bc9f87cb6224ca4a6b24aa82c75e76ac5170fce5594618f586edf6 BYE
