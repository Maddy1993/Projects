#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "structure.h"
#include "transfer.h"
#include "connection.h"
#include "check.h"

void* printCycle(void* pointer){

	printf("--------------------------------------------------\n");
	printf("\n------Checking for alive connection-----------------\n");
	struct arguments *obj = (struct arguments*) pointer;
	struct peer_information* node = obj->node;
	char identifier[10];
	memset(identifier, 0, 10);
	memcpy(identifier, obj->mode, 4);

	//variable declarations
	int retry;

	pthread_detach(pthread_self());

	pthread_mutex_lock (&mutexVar);
	//printf("Lock Achieved\n");
	//printf("identifier is %s\n",identifier);
	//if the caller of the function is ROOT
	if(strcmp(identifier, ROOT) == 0){

		if(node->successor == NULL && node->predecessor == NULL){
			/*printf("\n;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
			printf(";;;;;;; I don't have a successor ;;;;;;;;\n");
			printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");

			printf("\n;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
			printf(";;;;; I don't have a predecessor ;;;;;;\n");
			printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");

			printf("\n------------End of Checking-----------------------\n");*/
			pthread_mutex_unlock (&mutexVar);
			//printf("UnLock Achieved\n");
			pthread_exit(NULL);
		}

		if(node->successor == NULL){	
			/*printf("\n;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
			printf(";;;;;;; I don't have a successor ;;;;;;;;\n");
			printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");*/
		} else{
			//send heart beat packet to it successor
			//retry three times if no acknowledgment is received
			//printf("Sending CHECK packet to my(%s) successor %s\n", node->hostname, node->successor->hostname);
			retry = performSendRead(node->successor,CHECK);

			//if no data is received after three attempts, declare that the peer is dead and assign yourself a
			// value of NULL(empty)
			if(retry ==3){
				if(strcmp(node->successor->hostname, node->predecessor->hostname) == 0){
					node->successor = NULL;
					node->predecessor = NULL;	
				} else{
					node->successor = NULL;
				}

				
				printf("\n;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
				printf(";;;;;;; I lost my successor ;;;;;;;;\n");
				printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");

			}
		}
	}
		pthread_mutex_unlock (&mutexVar);
		//printf("UnLock Achieved\n");
		printf("\n------------End of Checking-----------------------\n");
		//free(mutexVar);
		pthread_exit(NULL);
	}

 int performSendRead(struct peer_information* peer, int packetType){
 	int retry=0;
 	int sockID;

 	struct peer_information* temp =  create_node();

 	while(1 && retry < 3){
			sockID = makeConnection(peer->hostname, peer->Port);
			//printf("SockID received: %d\n",sockID);
			if(sockID == CONNECTION_ERROR){
				return 3;
			}
			sendData(sockID, peer, packetType);
			temp = selectiveRead(sockID, temp);

			if(strcmp(temp->flag, STATUS_CHECK) == 0){
				printf("\n;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
				printf("My peer (%s) is alive\n", peer->hostname);
				printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
				//send  ACK before closing
				sendData(sockID,temp,ACK);
				close(sockID);
				break;
			} else if(strcmp(temp->flag, SEVERED) == 0){
				// /printf("Tie severed with %s\n",peer->hostname);
				break;
			}
			++retry;
			close(sockID);
		}

	return retry;
 }
