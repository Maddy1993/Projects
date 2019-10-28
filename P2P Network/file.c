#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/bn.h>

#include "structure.h"
#include "rootClient.h"
#include "connection.h"
#include "transfer.h"
#include "sendObject.h"
#include "file.h"

void sendFileName(int, char*, struct peer_information*, char*);
void exchange(char*, long, FILE*);

void filesTransfer(struct peer_information* node, char* peerName){

	//variables to handle file operations
	char fileName[FILENAME_LENGTH];
	char fileName_peer[FILENAME_LENGTH];
	char temp[FILENAME_LENGTH];
	//char *resultVal[FILENAME_LENGTH];
	char *resultVal;
	FILE *fp1, *fp2, *fp3;
	int connectionID = 0;
	char* pointerCh;
	char resultVal1[FILENAME_LENGTH];

	//variables to handle node hashValue comparisons
	unsigned char obuf[HASHSIZE];
	unsigned char obuf1[HASHSIZE];
	unsigned char obuf2[HASHSIZE];
	int result = 0, result1=0;
	BIGNUM *b1 = BN_new();
	BIGNUM *b2 = BN_new();
	BIGNUM *b3 = BN_new();

	//get the index file name of node and its successor
	memset(fileName, 0, FILENAME_LENGTH);
	sprintf(fileName, "%s%s%s", node->hostname, "-", "index.txt");

	memset(temp, 0, FILENAME_LENGTH);
	sprintf(temp, "%s", "replace.txt");

	if(strcmp(peerName, ISSUCCESSOR) == 0){
		//open file
		memset(fileName_peer, 0, FILENAME_LENGTH);
		sprintf(fileName_peer, "%s%s%s", node->successor->hostname, "-", "index.txt");

		//calculate SHA value for the node.
		SHA1((unsigned char*)node->successor->hostname, strlen(node->successor->hostname), obuf);
		BN_dec2bn(&b3, (char*)obuf);

	} else if(strcmp(peerName, ISPREDECESSOR) == 0){
		//open file
		memset(fileName_peer, 0, FILENAME_LENGTH);
		sprintf(fileName_peer, "%s%s%s", node->predecessor->hostname, "-", "index.txt");

		//calculate SHA value for the node.
		SHA1((unsigned char*)node->predecessor->hostname, strlen(node->predecessor->hostname), obuf);
		BN_dec2bn(&b3, (char*)obuf);
	}

	//open the file streams
	if((fp1 = fopen(fileName, "a+")) == NULL){
		fprintf(stderr, "FILE %s opening error: %s\n",fileName, strerror(errno));
		return;
	}

	if((fp2 = fopen(fileName_peer, "a+")) == NULL){
		fprintf(stderr, "FILE %s opening errno: %s\n", fileName_peer, strerror(errno));
		return;
	}

	if((fp3 = fopen(temp, "a+")) == NULL){
		fprintf(stderr, "FILE %s opening errno: %s\n", temp, strerror(errno));
		return;
	}

	//printf("fileName_successor %s\n",fileName_successor);
	SHA1((unsigned char*)node->hostname, strlen(node->hostname), obuf1);
	BN_dec2bn(&b1, (char*)obuf1);

	//memset(resultVal, 0, FILENAME_LENGTH);
	memset(resultVal1, 0, FILENAME_LENGTH);

	//printf("fileName accessing: %s\n",fileName);
	while(1){
		if(fgets((char*) resultVal1, FILENAME_LENGTH, fp1) != NULL){

			if(resultVal1 != NULL){
			 	pointerCh = strtok(resultVal1, "\n");
				if(pointerCh != NULL){
					//printf("pointerCh %s\n",pointerCh);
					resultVal = pointerCh;
	 			}
			}

			printf("resulted val %s %ld\n",resultVal, strlen(resultVal));

			SHA1((unsigned char*)resultVal, strlen(resultVal), obuf2);
			BN_dec2bn(&b2, (char*)obuf2);

			result = BN_cmp(b2, b1);
			result1 = BN_cmp(b2, b3);

			if(strcmp(peerName, ISSUCCESSOR) == 0){
				if((result1 == -1 || result1 == 0 || result1 == 1) && result == 1){
					printf("%s hash is less than successor node ID but greater than current node ID. So, transferring the file to successor\n", resultVal);
					connectionID = makeConnection(node->successor->hostname, node->successor->Port);
					if(connectionID > 0){
						sendFileName(connectionID, resultVal, node->successor, node->hostname);
					}
					//exchange(resultVal, strlen(resultVal), fp2);
				} else if((result == -1 || result == 0)&& result1 == -1){
					printf("%s Less than or equal to current node hashValue. So not distributing this file\n", resultVal);
					exchange(resultVal, strlen(resultVal), fp3);
				}
	 				else
						break;
		}
		 else if(strcmp(peerName, ISPREDECESSOR) == 0){
			 printf("in predecessor Checking\n");
			//when greater than predecessor and less than current node
			if((result == -1 || result == 0) && result1 == 1){
					printf("%s is less than currentNode but greater than predecessor\n", resultVal);
			}
			//when less than predecessor and also less than current node
			else if((result1 == -1 || result1 == 0) && result == -1){
				printf("%s is less than both currentNode and predecessor. So, transferring it to predecessor\n",resultVal);
				connectionID = makeConnection(node->predecessor->hostname, node->predecessor->Port);
				if(connectionID > 0){
					sendFileName(connectionID, resultVal, node->predecessor, node->hostname);
				} else{
					printf("Connection could not be established with %s\n",node->predecessor->hostname);
				}
			}
			//when greater than predecessor and greater than current node, do nothing.
			else
				break;
		}
	} else
		break;
}
	//after recursing through the current node index file, and storing the contents in
	//temporary file, remove the current index file and rename the temporary file to current node
	//index file.
	remove(fileName);
	rename(temp, fileName);

	//free and close pointers
	BN_free(b1);
	BN_free(b2);
	BN_free(b3);

	if((fclose(fp1) != 0)){
		fprintf(stderr, "File Closing Error %s\n",strerror(errno));
	}

	if((fclose(fp2) != 0)){
		fprintf(stderr, "File Closing Error %s\n",strerror(errno));
	}

	if((fclose(fp3) != 0)){
		fprintf(stderr, "File Closing Error %s\n",strerror(errno));
	}
}

void exchange(char* resultVal, long resultVal_length, FILE* file){
	if(feof(file) == 0){
		fwrite(resultVal,sizeof(char), resultVal_length, file);					// Writing data to the file.
		fflush(file);
	}


	if(ferror(file) != 0){
		fprintf(stderr, "Error while writing : %s\n", strerror(errno));
	}
}

void sendFileName(int connectionID, char* fileName, struct peer_information* node, char* objectName){
	int sentBytes = 0;
	char dataSent[INTERNAL_PACKET_SIZE];
	char *addr;
	char nodeObjectName[FILENAME_LENGTH];
	struct peer_information* temp = create_node();

	//packet variables
	long BUFFER_SIZE;
	long nameLength = 0;

	memset(dataSent, 0, INTERNAL_PACKET_SIZE);
	addr = dataSent;

	nameLength = strlen(fileName);
	memcpy(addr, &nameLength, sizeof(long));

	addr += sizeof(long);
	memcpy(addr, fileName, nameLength);

	addr += nameLength;
	BUFFER_SIZE = (long)(addr - dataSent);
	//printf("node->successor %s\n",node->successor->hostname);
	sendData(connectionID, node, STOREOBJ);

	temp = selectiveRead(connectionID, node);

	if(strcmp(temp->flag, ACK_MSG) == 0){
		if((sentBytes = send(connectionID, dataSent, BUFFER_SIZE, 0)) == -1){
			fprintf(stderr, "Send error: %s\n", strerror(errno));
			//exit(EXIT_FAILURE);
		}

		temp = selectiveRead(connectionID, temp);
		if(strcmp(temp->flag, UPDATE_MSG) == 0){
			//according to the  flow, sadly, i have to close the connection and establish a
			//new connection.			
			//printf("If it came till here, the successor must display listening\n");
			close(connectionID);
			connectionID = makeConnection(node->hostname, node->Port);
			if(connectionID > 0){
				memset(nodeObjectName, 0, FILENAME_LENGTH);
				sprintf(nodeObjectName, "%s%s%s", objectName, "-", fileName);
				//printf("nodeObjectName %s\n",nodeObjectName);
				sendData(connectionID, node, DATAPACKET);
				sendObjectData(nodeObjectName, connectionID, UPDATE);

				//remove existing file
				remove(nodeObjectName);
			} else{
				printf("Connection could not be established with %s\n",node->hostname);
			}
		}
	}
}
