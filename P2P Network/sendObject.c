#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "client.h"
#include "transfer.h"
#include "connection.h"
#include "structure.h"

#define UNUSED(x) (void)(x)

void sendPacket(int, char*, int);
char* receivePacket(int, char*, char*, long);

struct peer_information* destNode;

int sendName(char *fileName, int ID){
	char dataSent[MAX_FILE_NAME_LENGTH+1];
	char dataReceived[MAX_FILE_NAME_LENGTH];
	char *currAddress;

	int sentBytes;
	//int recvBytes;
	//int connection_request;
	int return_val = -1;

	long fileName_length, BUFFER_SIZE;

	struct timeval timeoutVal;
	fd_set readSet;
	int retry=0, result=0;

	memset(dataReceived, 0, MAX_FILE_NAME_LENGTH);
	memset(dataSent, 0, MAX_FILE_NAME_LENGTH+1);

	currAddress = dataSent;
	memcpy(currAddress, &STOREOBJ, sizeof(const int));

	currAddress += sizeof(const int);
	fileName_length = strlen(fileName);
	memcpy(currAddress, &fileName_length, sizeof(long));

	currAddress += sizeof(long);
	memcpy(currAddress, fileName, fileName_length);

	currAddress += fileName_length;

	BUFFER_SIZE = (long)(currAddress - dataSent);

	while(1 && retry < 3){
		//send a request to the root.
		if((sentBytes = send(ID, dataSent, BUFFER_SIZE, 0)) == -1){
			fprintf(stderr, "Send error: %s\n", strerror(errno));
			//exit(EXIT_FAILURE);
		}

		printf("File sent to ROOT\n");

		// Initialize time out struct
	   timeoutVal.tv_sec = 20;
	   timeoutVal.tv_usec = 0;

	   // Initialize the set
	   FD_ZERO(&readSet);
	   FD_SET(ID, &readSet);

	   // select()
	   result = select(ID+1, &readSet, NULL, NULL, &timeoutVal);
	   ++retry;
	   if(result == 0){
	   		printf("Nothing happened. Retry attempt: %d\n", retry);
	   } else if(retry < 0){
	   		fprintf(stderr, "Select() error: %s\n", strerror(errno));
	   } else if(retry > 0){
	   		if (FD_ISSET(ID, &readSet)) {
				destNode = create_node();
				destNode = selectiveRead(ID, destNode);

    		    break;
    		}
	   }
	}

	printf("%s\n",destNode->flag);
	if(strcmp(destNode->flag, ACK_MSG) == 0){
		return_val = 2;
		printf("File will be stored with the root %s\n", destNode->hostname);
	} else if(strcmp(destNode->flag, UPDATE_MSG) == 0){
		printf("Received Update message\n");
		close(ID);
		return_val = 3;
	} else if(retry == 3){
		return_val = -1;
	}

	return return_val;
}

int sendObjectData(char* fileName, int ID, int messageType){
	int fileSize = 0, currPos = 0;//, remainingChar = 0;
	int curr_seq = 0, ack_seq = 0;
	int bytes_read = 0;
	//int messageType = ACK;
	int return_val = -1;
	char data[DATA_PACKET_SIZE - CONTROL_PACKET_LENGTH];
	char packet[DATA_PACKET_SIZE];
	char dataReceived[DATA_PACKET_SIZE];
	long bufferSize;
	char *currAddress;
	FILE *fp;

	UNUSED(fileSize); UNUSED(currPos);


	if((fp = fopen(fileName, "r")) == NULL){
		fprintf(stderr, "File Open Error: %s\n", strerror(errno));
		//exit(EXIT_FAILURE);
		return_val = -1;
		return return_val;
	}


	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	while(1){
		if((messageType == ACK || messageType == UPDATE) && curr_seq == ack_seq){

			memset(data, 0, DATA_PACKET_SIZE - CONTROL_PACKET_LENGTH);
			memset(packet, 0, DATA_PACKET_SIZE);
			//remainingChar = fileSize - currPos;

			if(feof(fp) == 0){
				curr_seq++;
				bytes_read = fread(data, sizeof(char), DATA_PACKET_SIZE - CONTROL_PACKET_LENGTH, fp);

				printf("reading complete\n");
				if(ferror(fp)!= 0){
					fprintf(stderr, "Error: %s\n", strerror(errno));
					break;
				} else{
					messageType = DATAPACKET;
					currAddress = packet;

					memcpy(currAddress, &messageType, sizeof(int));
					currAddress += sizeof(int);

					memcpy(currAddress, &curr_seq, sizeof(int));
					currAddress += sizeof(int);

					memcpy(currAddress, &bytes_read, sizeof(int));
					currAddress += sizeof(int);

					memcpy(currAddress, data, bytes_read);
					currAddress += bytes_read;

					bufferSize = (long)(currAddress - packet);

					printf("packing complete\n");
					sendPacket(ID, packet, DATA_PACKET_SIZE);
				}
			} else if(feof(fp) != 0){
				fprintf(stderr, "End of file: %s\n",strerror(errno));
				memset(packet, 0, DATA_PACKET_SIZE);
				memcpy(packet, &END, sizeof(const int));
				sendPacket(ID, packet, 4);
				return_val = 1;
				break;
			}

			currPos = ftell(fp);
			memset(dataReceived, 0, DATA_PACKET_SIZE);
			memcpy(dataReceived, receivePacket(ID, packet, dataReceived, bufferSize), DATA_PACKET_SIZE);

			messageType = 0;
			currAddress = dataReceived;
			memcpy(&messageType, currAddress, sizeof(int));
			currAddress += sizeof(int);
			memcpy(&ack_seq, currAddress, sizeof(int));
			currAddress += sizeof(int);
			printf("ack received :%d\n",ack_seq);
			printf("messageType: %d\n",messageType);
		} else if(messageType == ACK && ack_seq != curr_seq){
			printf("Acknowledgments did not match\n");                                  // and when ack sequence and current sequence
			printf("Resending %d because acknowledgment received is: %d\n", curr_seq,ack_seq);             // are not equal
			sendPacket(ID, packet, bufferSize);
		}  else if(messageType !=ACK || messageType != UPDATE){
			printf("Unknown Header Packet. Ending program.\n");
			break;
		} else{
			return_val = 1;
		}


	}

	return return_val;

}

void sendPacket(int ID, char *packet, int size){
	int sentBytes;

	fflush(NULL);
	//send a request to the root.
	if((sentBytes = send(ID, packet, size, 0)) == -1){
		fprintf(stderr, "Send error: %s\n", strerror(errno));
		//exit(EXIT_FAILURE);
	}
	printf("packet sent");
}

char* receivePacket(int ID, char* packet, char* dataReceived, long size){
	//char dataReceived[DATA_PACKET_SIZE];
	int recvBytes;
	struct timeval timeoutVal;
	fd_set readSet;
	int retry=0, result=0;

	memset(dataReceived, 0, DATA_PACKET_SIZE);

	while(1 && retry < 3){

	   // Initialize time out struct
	   timeoutVal.tv_sec = 30;
	   timeoutVal.tv_usec = 0;

	   // Initialize the set
	   FD_ZERO(&readSet);
	   FD_SET(ID, &readSet);

	   // select()
	   result = select(ID+1, &readSet, NULL, NULL, &timeoutVal);
	   //printf("retry value: %d\n",retry);
	   ++retry;
	   if(result == 0){
	   		printf("Nothing happened. Retry attempt: %d\n", retry);
	   } else if(retry < 0){
	   		fprintf(stderr, "Select() error: %s\n", strerror(errno));
	   } else if(retry > 0){
	   		if (FD_ISSET(ID, &readSet)) {

				if((recvBytes = recv(ID, dataReceived, DATA_PACKET_SIZE, 0)) == -1){	//receive data from the client: HELLO Message
					fprintf(stderr, "Reading error: %s\n", strerror(errno));
				}

    		    break;
    		}
	   }

	   sendPacket(ID, packet, size);
	}

	return dataReceived;
}
