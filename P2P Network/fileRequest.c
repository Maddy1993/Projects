#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "sendObject.h"
#include "transfer.h"
#include "connection.h"
#include "structure.h"

char* packRequestPacket(char*, int, char*);
void readPacket(int, char*, int, char*);

void sendRequest(char* type, int sockID, char* fileName){

	//variables
	char data[REQUEST_PACKET_LENGTH];

	//initialize required variables
	memset(data, 0, REQUEST_PACKET_LENGTH);

	//if type is "i", then the request is of type iterative
	if(strcmp(type, ITERATIVE) == 0){
		memcpy(data, packRequestPacket(fileName, _ITERATIVE, data), REQUEST_PACKET_LENGTH);

		while(1){
			//send packet to ROOT
			sendPacket(sockID, data, REQUEST_PACKET_LENGTH);
			memset(data, 0, REQUEST_PACKET_LENGTH);
			readPacket(sockID, data, _ITERATIVE, fileName);
			break;
		}

	} else if(strcmp(type, RECURSIVE) == 0){
		memcpy(data, packRequestPacket(fileName, _RECURSIVE, data), REQUEST_PACKET_LENGTH);
		//send packet to ROOT
		sendPacket(sockID, data, REQUEST_PACKET_LENGTH);
		memset(data, 0, REQUEST_PACKET_LENGTH);
		readPacket(sockID, data, _RECURSIVE, fileName);
	}
}

void readPacket(int sockID, char* data, int type, char* fileName){
	//variables
	int recvBytes = 0;
	int response = 0;
	int result = 0;
	char port[255];
	long length;
	int connectID;
	long length1;
	char node_name[255];
	char *present;

	struct peer_information *temp = create_node();

	memset(node_name, 0, 255);
	memset(port, 0, 255);

	if((recvBytes = recv(sockID, data, REQUEST_PACKET_LENGTH, 0)) == -1){	//receive data from the client: HELLO Message
		fprintf(stderr, "Reading error: %s\n", strerror(errno));
	}

	present = data;
	memcpy(&response, present, sizeof(int));
	present += sizeof(int);
	printf("response %d\n",response);

	memcpy(&result, present, sizeof(int));
	present += sizeof(int);
	printf("result %d\n",result);

	memcpy(&length, present, sizeof(long));
	present += sizeof(long);
	printf("length %ld\n",length);

	memcpy(node_name, present, length);
	present += length;
	printf("node_name %s\n",node_name);

	memcpy(&length1, present, sizeof(long));
	present += sizeof(long);
	printf("length1 %ld\n",length1);

	memcpy(port, present, length1);
	present += length1;
	printf("port %s\n",port);

	if(response == RESPONSE){
		if(result == FAILED){
			if(type == _ITERATIVE){
				close(sockID);
				connectID = makeConnection(node_name, port);
				sendData(connectID, temp, _ITERATIVE);
				sendRequest(ITERATIVE, connectID, fileName);
			} else if(type == _RECURSIVE){
				printf("Object not found in the ring\n");
			}
		} else{
			printf("Object found in the ring with %s\n", node_name);
		}
	} else if(response == END){
		printf("Object not found in the ring\n");
	}
}

char* packRequestPacket(char* fileName, int type, char* packet){
	char *present;
	long fileName_length;

	//pointer arthimetic to store values
	present = packet;

	memcpy(present, &type, sizeof(int));
	present += sizeof(int);

	fileName_length = strlen(fileName);
	memcpy(present, &fileName_length, sizeof(long));

	present += sizeof(long);
	memcpy(present, fileName, fileName_length);

	present += fileName_length;

	return packet;
}
