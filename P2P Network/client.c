#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "connection.h"
#include "transfer.h"
#include "sendObject.h"
#include "client.h"
#include "fileRequest.h"


void printMenu(int);

char clientName[MAX_NAME_LENGTH];
char clientPort[MAX_PORT_LENGTH];
char ROOTHOSTNAME[MAX_NAME_LENGTH];
char ROOTPORT[MAX_PORT_LENGTH];

int main(int argc, char* argv[]){

	int connectionID=0;

	if(argc < 9){
		printf("Usage is: ./dht_client <-p client_port> <-h client_hostname> <-r ROOT_PORT> <-R ROOTHOSTNAME>\n");
		exit(EXIT_FAILURE);
	} else{
		memset(ROOTHOSTNAME, 0, MAX_NAME_LENGTH);
		memset(ROOTPORT, 0, MAX_PORT_LENGTH);
		memset(clientName, 0, MAX_NAME_LENGTH);
		memset(clientPort, 0, MAX_PORT_LENGTH);

		memcpy(clientName, argv[4], strlen(argv[4]));
		memcpy(clientPort, argv[2], strlen(argv[2]));
		memcpy(ROOTHOSTNAME, argv[8], strlen(argv[8]));
		memcpy(ROOTPORT, argv[6], strlen(argv[6]));

		printf("%s", argv[8]);
		printf("%s\n",ROOTHOSTNAME);
		connectionID = makeConnection(getIPfromName(ROOTHOSTNAME), ROOTPORT);
		if(connectionID > 0){
			printMenu(connectionID);
		}else{
			printf("Connection to %s could not be established. Exiting the program\n",ROOTHOSTNAME);
			close(connectionID);
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}

void printMenu(int ID){
	char response[1];

	int result = 0;
	int reConnectionID = 0;

	char fileName[MAX_FILE_NAME_LENGTH];
	struct peer_information* temp_node = create_node();

	memset(fileName, 0, sizeof(char)*MAX_FILE_NAME_LENGTH);

	printf("-----------------------------------------\n");
	printf("Connection Established with %s\n",ROOTHOSTNAME);
	while(1){
		printf(";;;;;;;;;\n");
		printf("|| MENU ||\n");
		printf(";;;;;;;;;\n");
		printf("1. Store an Object: \"s\"\n");
		printf("2. Retrieve an Object iteratively: \"i\"\n");
		printf("3. Retrieve an Object Recursively: \"r\"\n");
		printf("4. EXIT: \"e\"\n");
		printf("Enter a Option: ");
		fflush(stdout);
		scanf("%s", response);
		fflush(stdout);

		if(strcmp(response, "s") == 0){
			printf("Enter the name of the file: ");
			scanf("%s", fileName);
			result = sendName(fileName, ID);
			if( result > 0){
				if(result != 3){
					if((result = sendObjectData(fileName, ID, ACK)) > 0){
						printf("Object Stored Successfully\n");
					} else{
						printf("Could not send data to %s\n", ROOTHOSTNAME);
						continue;
					}
				} else if(result == 3){
					ID = makeConnection(destNode->hostname, destNode->Port);
					if(ID > 0){
						printf("Connected to %s to store data\n",destNode->hostname);
						sendData(ID, temp_node, DATAPACKET);
						if((result = sendObjectData(fileName, ID, UPDATE)) > 0){
							printf("Object Stored Successfully\n");
						} else{
							printf("Could not send data to %s\n", ROOTHOSTNAME);
							continue;
						}
					} else{
						printf("Connection could not be established with %s\n",destNode->hostname);
					}
				}
			} else{
				printf("Could not send Object to %s\n", ROOTHOSTNAME);
				continue;
			}
		} else if(strcmp(response, "i") == 0){
 			//printf("Did not implement iterative retrieval of Objects");
 			//print the display message
			printf("Enter the object name to retrieve: ");
			memset(fileName, 0, MAX_FILE_NAME_LENGTH);
			scanf("%s", fileName);
			printf("\n");

 			sendRequest(ITERATIVE, ID, fileName);
		} else if(strcmp(response, "r") == 0){
			//printf("Did not implement recursive retrieval of Objects");
			printf("Enter the object name to retrieve: ");
			memset(fileName, 0, MAX_FILE_NAME_LENGTH);
			scanf("%s", fileName);
			printf("\n");

 			sendRequest(RECURSIVE, ID, fileName);
		} else if(strcmp(response, "e") == 0){
			printf("Exiting Program....\n");
			close(ID);
			exit(EXIT_SUCCESS);
		} else{
			printf("Unknown Option selected\n");
			continue;
		}

		//re-send connection request to ROOT
		reConnectionID = makeConnection(getIPfromName(ROOTHOSTNAME), ROOTPORT);
		if(reConnectionID > 0){
			ID = reConnectionID;
		} else{
			printf("ROOT node not available\n");
			break;
		}
	}
}
