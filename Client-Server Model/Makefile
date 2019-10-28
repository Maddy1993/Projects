all: client.o server.o
	gcc server.o -o makeServer
	gcc client.o -o client
client.o: clientFiles/client.c clientFiles/clientdataCommunication.c
	gcc -c clientFiles/client.c
server.o: serverFiles/server.c serverFiles/serverdataCommunication.c
	gcc -c serverFiles/server.c	
