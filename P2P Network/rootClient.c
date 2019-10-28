#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdint.h>
#include <openssl/bn.h>
#include <openssl/sha.h>

#include "node.h"
#include "transfer.h"
#include "structure.h"
#include "connection.h"
#include "rootClient.h"
#include "client.h"

#define DATA_SIZE 512
#define ACK_SIZE 8
#define WAIT_CONN 0
#define UNUSED(x) (void)(x)

char fileVal[512];

void processPacket(int, struct peer_information*);
void checkandStore(int, char*, struct peer_information*);
void sendAck(int, int);
void writetoFile(char*, char*, long, FILE*);
void sendDatainParts(int, char*, char*);
void sendObjectPacket(int, char*);
void searchObject(int, char*, int, struct peer_information*);

void* clientRunner(void* arg){
	//arguments
	socklen_t client_addr_len;
	struct sockaddr_storage client_addr;
	struct addrinfo  clientAddr;
	int client_socket_desc;
	int sockID;

	//Cast the argument the to "int"
	/*int* ID = (int*)(uintptr_t) arg;
	int sockID =  *ID;
*/
	struct arguments *obj = (struct arguments*) arg;
	struct peer_information* node = (struct peer_information*)obj->node;
	sockID = obj->clientID;

	//printf("SockID is: %d\n",sockID);

	pthread_detach(pthread_self());

	while(1){
		//listen to the socket
	 	if((listen(sockID,WAIT_CONN))==-1){
			fprintf(stderr, "Call to the function Client Socket-Listen() failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
	 	}

		printf("Socket Open for Client, listening on Port: %s\n", CLIENT_PORT);

		//accept connections from peer.
		client_addr_len = sizeof client_addr;
		if((client_socket_desc = accept(sockID, (struct sockaddr *) &clientAddr, &client_addr_len)) == -1){
			fprintf(stderr, "Call to the function Accept() failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
	 	}

	 	printf("Client Connection accepted\n");

	 	processPacket(client_socket_desc,  node);

	 	printf("Packet processing completed\n");
	}

 	pthread_exit(NULL);
}

void processPacket(int clientID, struct peer_information* node){
	int recvBytes;
	char dataReceived[PACKET_SIZE];
	char *curAddress;

	//packet
	int packetType = 0;
	long fileName_length = 0;
	char fileName[MAX_FILE_NAME_LENGTH];

	memset(fileName, 0, MAX_FILE_NAME_LENGTH);
	memset(dataReceived, 0, PACKET_SIZE);

	if((recvBytes = recv(clientID, dataReceived, PACKET_SIZE, 0)) == -1){
			fprintf(stderr, "Reading error: %s\n", strerror(errno));
	}

	curAddress = dataReceived;
	memcpy(&packetType, curAddress, sizeof(const int));
	curAddress += sizeof(const int);

	if(packetType == STOREOBJ){
		memcpy(&fileName_length, curAddress, sizeof(long));

		curAddress +=sizeof(long);
		memcpy(fileName, curAddress, fileName_length);

		curAddress += fileName_length;

		printf("Received request to store(%d) object: %s\n", packetType, fileName);

		memcpy(ROOT_HOSTNAME, node->hostname, strlen(node->hostname));
		memcpy(fileVal, fileName, strlen(fileName));

		if(packetType == STOREOBJ){
			checkandStore(clientID, fileName, node);
		} else if(packetType == 0){
			printf("Unknown packet received from Client.\n");
			pthread_exit(NULL);
		}
	} else if(packetType == _ITERATIVE){

		memcpy(&fileName_length, curAddress, sizeof(long));

		curAddress +=sizeof(long);
		memcpy(fileName, curAddress, fileName_length);

		curAddress += fileName_length;

		searchObject(clientID, fileName, _ITERATIVE, node);
	} else if(packetType == _RECURSIVE){

		memcpy(&fileName_length, curAddress, sizeof(long));

		curAddress +=sizeof(long);
		memcpy(fileName, curAddress, fileName_length);

		curAddress += fileName_length;

		searchObject(clientID, fileName, _RECURSIVE, node);
	}
}

void searchObject(int sockID, char* fileName, int type, struct peer_information* node){
	char indexfileName[MAX_FILE_NAME_LENGTH];
	char result[MAX_FILE_NAME_LENGTH];
	char returnPacket[RETRIEVE_PACKET_SIZE];
	char *resultVal;
	char *pointerCh;
	int flag=0, sentBytes = 0, recvBytes =0;
	long length, length1;
	int connectID;
	FILE *fp;

	//initialization of variables
	sprintf(indexfileName, "%s%s%s", node->hostname, "-", "index.txt");
	memset(result, 0, MAX_FILE_NAME_LENGTH);
	memset(returnPacket, 0, RETRIEVE_PACKET_SIZE);

	//open file of the node
	if((fp= fopen(indexfileName, "r")) == NULL){
		fprintf(stderr, "Error, %s , while opening file: %s\n", strerror(errno), indexfileName);
	}

	while(fgets((char*) result, FILENAME_LENGTH, fp) != NULL){

		//remove the trailing new line character
		if(result != NULL){
		 	pointerCh = strtok(result, "\n");
			if(pointerCh != NULL){
				resultVal = pointerCh;
				}
		}

		//compare the retrieved string with the file Name
		if(strcmp(resultVal, fileName) == 0){
			pointerCh = returnPacket;

			memcpy(pointerCh, &RESPONSE, sizeof(int));
			pointerCh += sizeof(int);

			memcpy(pointerCh, &SUCCESS, sizeof(int));
			pointerCh += sizeof(int);

			length = strlen(node->hostname);
			memcpy(pointerCh, &length, sizeof(long));

			pointerCh += sizeof(long);
			memcpy(pointerCh, node->hostname, length);

			pointerCh += length;
			length1 = strlen(node->Port);
			printf("length of port %ld\n", length1);
			memcpy(pointerCh, &length1, sizeof(long));
			pointerCh += sizeof(long);

			memcpy(pointerCh, node->Port, length1);
			pointerCh += length1;

			flag = 1;

			break;
		}
	}

	if(flag == 1){
		//send a request to the client.
		if((sentBytes = send(sockID, returnPacket, RETRIEVE_PACKET_SIZE, 0)) == -1){
			fprintf(stderr, "Send error: %s\n", strerror(errno));
		}
	} else if(flag == 0){
		if(type == _RECURSIVE){
			printf("it is a recursive call\n");
			connectID = makeConnection(node->successor->hostname, node->successor->Port);
			memset(returnPacket, 0, RETRIEVE_PACKET_SIZE);

			if(strcmp(node->successor->hostname, ROOT_HOSTNAME) == 0){
				pointerCh = returnPacket;
				memcpy(pointerCh, &END, sizeof(int));
				pointerCh += sizeof(int);

				memcpy(pointerCh, &FAILED, sizeof(int));
				pointerCh += sizeof(int);

				length = strlen(node->successor->hostname);
				memcpy(pointerCh, &length, sizeof(long));
				pointerCh += sizeof(long);

				memcpy(pointerCh, node->successor->hostname, length);
				pointerCh += length;

				length1 = strlen(node->successor->Port);
				memcpy(pointerCh, &length1, sizeof(long));
				pointerCh += sizeof(long);

				memcpy(pointerCh, node->successor->Port, length1);
				pointerCh += length1;

				//send a request to the client.
				if((sentBytes = send(sockID, returnPacket, RETRIEVE_PACKET_SIZE, 0)) == -1){
					fprintf(stderr, "Send error: %s\n", strerror(errno));
				}

				return;
			}

			pointerCh = returnPacket;
			int r = _RECURSIVE;
			memcpy(pointerCh, &r, sizeof(int));

			pointerCh += sizeof(int);
			length = strlen(fileName);
			memcpy(pointerCh, &length, sizeof(long));

			pointerCh +=sizeof(long);
			memcpy(pointerCh, fileName, length);

			int l=0;
			while(l < 2){
				//send a request to the next peer.
				if((sentBytes = send(connectID, returnPacket, RETRIEVE_PACKET_SIZE, 0)) == -1){
					fprintf(stderr, "Send error: %s\n", strerror(errno));
				}
				++l;
			}

			printf("sent packet to %s\n",node->successor->hostname);
			memset(returnPacket, 0, RETRIEVE_PACKET_SIZE);

			//receive request from the peer
			if((recvBytes = recv(connectID, returnPacket, RETRIEVE_PACKET_SIZE, 0)) == -1){	//receive data from the client: HELLO Message
					fprintf(stderr, "Reading error: %s\n", strerror(errno));
			}

			//send a response to previous peer
			if((sentBytes = send(sockID, returnPacket, RETRIEVE_PACKET_SIZE, 0)) == -1){
				fprintf(stderr, "Send error: %s\n", strerror(errno));
			}

		} else if(type == _ITERATIVE){
			pointerCh = returnPacket;

			if(strcmp(node->successor->hostname, ROOT_HOSTNAME) == 0){
				memcpy(pointerCh, &END, sizeof(int));
				pointerCh += sizeof(int);

				memcpy(pointerCh, &FAILED, sizeof(int));
				pointerCh += sizeof(int);

				length = strlen(node->successor->hostname);
				memcpy(pointerCh, &length, sizeof(long));
				pointerCh += sizeof(long);

				memcpy(pointerCh, node->successor->hostname, length);
				pointerCh += length;

				length1 = strlen(node->successor->Port);
				memcpy(pointerCh, &length1, sizeof(long));
				pointerCh += sizeof(long);

				memcpy(pointerCh, node->successor->Port, length1);
				pointerCh += length1;
			} else{
				memcpy(pointerCh, &RESPONSE, sizeof(int));
				pointerCh += sizeof(int);

				memcpy(pointerCh, &FAILED, sizeof(int));
				pointerCh += sizeof(int);

				length = strlen(node->successor->hostname);
				memcpy(pointerCh, &length, sizeof(long));
				pointerCh += sizeof(long);

				memcpy(pointerCh, node->successor->hostname, length);
				pointerCh += length;

				length1 = strlen(node->successor->Port);
				memcpy(pointerCh, &length1, sizeof(long));
				pointerCh += sizeof(long);

				memcpy(pointerCh, node->successor->Port, length1);
				pointerCh += length1;
			}

			//send a request to the client.
			if((sentBytes = send(sockID, returnPacket, RETRIEVE_PACKET_SIZE, 0)) == -1){
				fprintf(stderr, "Send error: %s\n", strerror(errno));
			}
		}
	}
}

void checkandStore(int clientID, char* fileName, struct peer_information* node){
	char indexfileName[300];
	unsigned char obuf[20];
	int i, result;
	int flag = 0;

	char dataReceived[PACKET_SIZE];
	/*int recvBytes=0;
	char *curAddress;*/

	BIGNUM *b1 = BN_new();
	BIGNUM *b2 = BN_new();

	memset(indexfileName, 0, 300);

	sprintf(indexfileName, "%s%s%s", node->hostname, "-", "index.txt");
	//node->indexfile = fopen(indexfileName, "a+");


	SHA1((unsigned char*)fileName, strlen(fileName), obuf);
	BN_dec2bn(&b1, (char*)obuf);

	printf("hashValue of %s\n",fileName);
	for(i=0;i<20;i++){
		printf("%x", obuf[i]);
	}


	SHA1((unsigned char*)node->hostname, strlen(node->hostname), obuf);
	BN_dec2bn(&b2, (char*) obuf);

	printf("\nNode hashID: ");
	printf("length of name %ld\n",strlen(node->hostname));

	for(i=0;i<20;i++){
		printf("%x", obuf[i]);
	}

	printf("\n");

	result = BN_cmp(b1, b2);
	if(result == -1 || result == 0){
		printf("less-than\n");
		flag = 1;
		goto fileStorage;
	} else if(result == 1){
		printf("Greater\n");
		printf("ROOT_HOSTNAME %s\n", ROOT_HOSTNAME);
		if(node->successor == NULL){
			printf("As there is no successor, storing the object with me\n");
			flag = 1;
			goto fileStorage;
		} else if(strcmp(node->successor->hostname, getIPfromName(ROOT_HOSTNAME)) == 0){
			printf("As the successor is the ROOT, storing the object with me\n");
			flag = 1;
			goto fileStorage;
		} else{
			flag = 0;
			printf("Node->successor name: %s\n",node->successor->hostname);
			int successorID = makeConnection(node->successor->hostname, node->successor->Port);
			if(successorID >= 0){
				//send the request to successor.
				sendData(successorID, node->successor, STOREOBJ);
				struct peer_information* temp = create_node();
				temp = selectiveRead(successorID, temp);

				//printf("temp %p\n",temp);
				if(strcmp(temp->flag, ACK_MSG) == 0){
					/*memcpy(node->flag, fileName, strlen(fileName));
					sendData(successorID, node->successor, 20);*/
					sendObjectPacket(successorID, fileName);
					temp = selectiveRead(successorID, temp);

					if(strcmp(temp->flag, UPDATE_MSG) == 0){
						printf("Sending packet to client.\n");
						sendData(clientID, temp, UPDATE);
					}
				}
			} else{
				printf("Connection could not be established with my successor\n");
			}
		}

	}

	fileStorage: {
		if(flag == 1){
			printf("just entered\n");
			writetoFile(indexfileName, fileName, strlen(fileName), node->indexfile);
			writetoFile(indexfileName, "\n", strlen("\n"), node->indexfile);

			printf("Stored the object %s to index file of %s\n",fileName, node->hostname);

			printf("node name %s\n",node->hostname);
			printf("root name %s\n",ROOT_HOSTNAME);

			memset(indexfileName, 0, 300);
			sprintf(indexfileName, "%s%s%s", node->hostname, "-", fileName);

			memset(fileVal, 0, MAX_FILE_NAME_LENGTH);
			memcpy(fileVal, indexfileName, strlen(indexfileName));
			printf("indexfileName %s\n",indexfileName);

			if(strcmp(node->hostname, ROOT_HOSTNAME) == 0){
				sendData(clientID, node, ACK);
			} else{
				printf("sent update message\n");
				sendData(clientID, node, UPDATE);
				return;
			}

			memset(indexfileName, 0, 300);
			sprintf(indexfileName, "%s%s%s", node->hostname, "-", fileName);

			sendDatainParts(clientID, dataReceived, indexfileName);
		}
	}
}

void sendDatainParts(int clientID, char* dataReceived, char* fileName){
char* curAddress;
int messageType=0;
int recvBytes=0;
int sequence=0;
int bytes_to_read=0;
char data[DATA_SIZE];
int curr_seq = 0, prev_seq = 0;

	while(1){
		memset(dataReceived, 0, PACKET_SIZE);
		printf("Something is happening after this\n");
		if((recvBytes = recv(clientID, dataReceived, PACKET_SIZE, 0)) == -1){
			fprintf(stderr, "Reading error: %s\n", strerror(errno));
		}
		printf("Something has happened\n");
		curAddress = dataReceived;
		memset(data, 0, DATA_SIZE);

		memcpy(&messageType, curAddress, sizeof(int));
		curAddress += sizeof(int);

		if(messageType == DATAPACKET){
			memcpy(&sequence, curAddress, sizeof(int));

			curAddress += sizeof(int);
			memcpy(&bytes_to_read, curAddress, sizeof(int));

			curAddress += sizeof(int);
			memcpy(data, curAddress, bytes_to_read);

			curAddress += bytes_to_read;

			printf("Received sequence %d \n",sequence);
			FILE *writeFile=NULL;
			curr_seq = sequence;

			printf("fileName: %s\n",fileName);
			if(curr_seq == prev_seq+1){
				writetoFile(fileName, data, bytes_to_read, writeFile);
				sendAck(clientID, curr_seq);
				prev_seq++;
			} else{
				sendAck(clientID, prev_seq);
			}

		} else if(messageType == END){
			printf("End Packet Received\n");
			printf("File transfer complete\n");
			break;
		}
	}
}

void sendAck(int clientID, int number){
	char dataSent[PACKET_SIZE];
	int sentBytes=0;
	char *curAddress;

	memset(dataSent, 0, PACKET_SIZE);
	curAddress = dataSent;

	memcpy(curAddress, &ACK, sizeof(int));
	curAddress+= sizeof(int);
	memcpy(curAddress, &number, sizeof(int));
	curAddress += sizeof(int);

	//send a request to the root.
	if((sentBytes = send(clientID, dataSent, ACK_SIZE, 0)) == -1){
		fprintf(stderr, "Send error: %s\n", strerror(errno));
		//exit(EXIT_FAILURE);
	}

}

void sendObjectPacket(int successorID, char* fileName){
	char dataSent[INTERNAL_PACKET_SIZE];
	char *curAddress;
	int sentBytes = 0;

	UNUSED(sentBytes);
	UNUSED(dataSent);

	memset(dataSent, 0, INTERNAL_PACKET_SIZE);
	curAddress = dataSent;
	long fileName_length = strlen(fileName);

	memcpy(curAddress, &fileName_length, sizeof(long));

	curAddress+=sizeof(long);
	memcpy(curAddress, fileName, fileName_length);

	printf("File name sent to my successor %s\n",fileName);
	//send a request to the root.
	if((sentBytes = send(successorID, dataSent, INTERNAL_PACKET_SIZE, 0)) == -1){
		fprintf(stderr, "Send error: %s\n", strerror(errno));
		//exit(EXIT_FAILURE);
	}

}
void writetoFile(char* file_name, char* data, long data_length, FILE* writeFile){
			if((writeFile = fopen(file_name, "a")) == NULL){
				fprintf(stderr, "%s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}

			if(feof(writeFile) == 0){
				fwrite(data,sizeof(char), data_length, writeFile);					// Writing data to the file.
				fflush(writeFile);
			}


			if(ferror(writeFile) != 0){
				fprintf(stderr, "Error while writing : %s\n", strerror(errno));
			}
			else{
				if(fclose(writeFile) != 0){
					fprintf(stderr, "%s\n", strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
}
