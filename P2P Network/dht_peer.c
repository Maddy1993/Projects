#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <openssl/err.h>

#include "structure.h"
#include "convert.h"
#include "transfer.h"
#include "connection.h"
#include "peerClient.h"
#include "rootClient.h"
#include "file.h"
#include "client.h"

#define UNUSED(x) (void)(x)
#define PACKET_SIZE 3000
#define IPSIZE 20
#define BUFFERSIZE 100
#define LESSTHAN -1
#define EQUAL 0
#define GREATER 1
#define CORRUPT_DATA "CORRUPTED"
#define OBJECT_LENGTH 512

struct peer_information* assignNodePosition(int, char*, char*, struct peer_information*, struct peer_information*, struct peer_information*, int);
void newNodePosition(int, struct peer_information*, struct peer_information*);
void sendUpdate(int, struct peer_information*, struct peer_information*);
void sendStatusCheck(struct peer_information*);


struct peer_information* dht_peer(int clientID, char* mode, char* root_port, char* root_name, char* own_name, char* own_port,
	struct peer_information *node, int connectionType){

	int recvBytes=0;
	const int CYCLE = 0;
	char dataReceived[PACKET_SIZE];
	char emptyMessage[MSGLEN];
	int retry = 0;
	int sockID;

	struct peer_information *new_node = create_node();
	struct peer_information *temp = create_node();
	struct peer_information *temp1 = create_node();
	struct peer_information *return_node = create_node();

	char ibuf[100];
	unsigned char obuf[20];
	char result[20];
	UNUSED(result);

	memset(dataReceived, 0, PACKET_SIZE);
	memset(emptyMessage, 0, MSGLEN);
	UNUSED(emptyMessage);

	if(strcmp(mode, "0") == 0){

		printf("\n--------------------------------------------\n");
		printf("--------------------PEER---------------------\n");
		printf("---------------------------------------------\n");
		memset(ibuf, 0, 100);

		//generating the Hash of peer IPs
		memcpy(ibuf, own_name, strlen(own_name));
		SHA1((unsigned char*)ibuf, strlen(ibuf), obuf);

		//creating peer
		struct peer_information *peer_node = initializeNode(NULL, NULL, (char*)obuf, own_port, own_name, PEER, emptyMessage);

		if(connectionType == REQUEST_TO_CONNECT){
			sendData(clientID, peer_node, REQUEST_TO_CONNECT);
			//peer_node = readData(clientID, new_node);

			while(1){
				new_node = selectiveRead(clientID, new_node);

				if(strcmp(new_node->flag, UNKNOWN) == 0){
					printf("Sent an Update packet. Instead, received: %s\n",new_node->flag);
					sendData(clientID, peer_node, REQUEST_TO_CONNECT);
					retry++;
					return_node = peer_node;
					goto return_value;
				} else if(strcmp(new_node->flag, FAILED_MSG) == 0){
					printf("Could not be placed. I shall retry\n");
					return_node = temp;
					goto return_value;
				}

				break;
			}

			temp =new_node->predecessor;
			//printf("temp: %s\n",temp->hostname);
			printf("Sent ack to requester\n");
			sendData(clientID, peer_node, ACK);
			return_node = new_node;

			printf("Received packet from Root:\n");
			printf(" My position in the network\n");
			temp1 = new_node->predecessor;
			printf("  I am a: %s\n",new_node->mode);

			printf("  My predecessor: %s\n", temp1->hostname);
			temp1 = new_node->successor;
			printf("  My successor: %s\n", temp1->hostname);
			printf("  My name and Port: %s %s\n",new_node->hostname, new_node->Port);

			goto return_value;
		} else if(connectionType == READ){
			//new_node = readData(clientID, temp);
			new_node = selectiveRead(clientID, temp);
			//temp = new_node->predecessor;

			if(strcmp(new_node->flag, UNKNOWN) == 0){
				return node;
			} else if(strcmp(new_node->flag, UPDATE_MSG) == 0){

				sendData(clientID, new_node, ACK);
				printf("Sent ack to requester\n");
				return_node = new_node;

				printf("Received packet from Root:\n");
				printf(" My position in the network\n");
				temp1 = new_node->predecessor;
				printf("  I am a:%s\n",peer_node->mode);
				printf("  My predecessor: %s\n", temp1->hostname);
				temp1 = new_node->successor;
				printf("  My successor: %s\n", temp1->hostname);
				printf("  My name and Port: %s %s\n",new_node->hostname, new_node->Port);

				if(strcmp(new_node->predecessor->hostname, node->predecessor->hostname) != 0){
					printf("predecessor changed\n");
					filesTransfer(new_node, ISPREDECESSOR);
				} else if(strcmp(new_node->successor->hostname, node->successor->hostname) != 0){
					printf("successor changed\n");
					filesTransfer(new_node, ISSUCCESSOR);
				}
				//filesTransfer(new_node);

				goto return_value;
			} else if(strcmp(new_node->flag, STATUS_CHECK) == 0){
				printf("\n----------------------------------------\n");
				if(strcmp(node->hostname, new_node->hostname) == 0){

						sendData(clientID, node, CHECK);

						temp = selectiveRead(clientID, temp);

						//printf("temp: %s\n",temp->flag);
						if(strcmp(temp->flag, ACK_MSG) == 0 || strcmp(temp->flag, STATUS_CHECK) == 0){
							/*printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
							printf("|| My predecessor %s is alive ||\n", node->predecessor->hostname);
							printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
*/
							fflush(NULL);
							//start peers heart beat check to its successor
							sendStatusCheck(node);
							//printf("Sent data to my successor %s (if he is there)!!!\n", node->successor->hostname);
						}
					//}

				}
				return_node = node;
				goto return_value;
			} else if(strcmp(new_node->flag, SEVERED) == 0){
				if(strcmp(node->predecessor->hostname, new_node->hostname) == 0){
					node->predecessor = NULL;
					//send to your predeccssor that you have unlinked
					sendData(clientID, new_node, UNLINK);
				} else if(strcmp(node->successor->hostname, new_node->hostname) == 0){
					node->successor = NULL;
					//send to your predeccssor that you have unlinked
					sendData(clientID, new_node, UNLINK);
				}
				//node->successor = NULL;
				return_node = node;
				goto return_value;
			} else if(strcmp(new_node->flag, STOREOBJECT) == 0){
				//send ack to predecessor signalling him to send the object Name
				//printf("new_node->hostname %s\n", new_node->hostname);
				//printf("new_node->predecessor %p\n",new_node->predecessor);
				//printf("received request to store from %s\n", new_node->predecessor->hostname);
				sendData(clientID, new_node, ACK);
				peerThread(node, clientID);
				return_node = node;
				goto return_value;
			} else if(strcmp(new_node->flag, STOREDATA) == 0){
				//printf("fileVal in dht_peer %s\n",fileVal);
				sendDatainParts(clientID, dataReceived, fileVal);
				return_node = node;
				goto return_value;
			} else if(strcmp(new_node->flag, ITERATIVE) == 0){
				printf("Iterative message\n");
				processPacket(clientID, node);
				return_node = node;
				goto return_value;
			} else if(strcmp(new_node->flag, RECURSIVE) == 0){
				printf("recursive message\n");
				processPacket(clientID, node);
				return_node = node;
				goto return_value;
			}
		}

		return_value: return return_node;

 	} else if(strcmp(mode, "1") == 0){

 		printf("\n--------------------------------------------\n");
		printf("--------------------ROOT---------------------\n");
		printf("---------------------------------------------\n");
 		//new_node = readData(clientID, new_node);
 		new_node = create_node();
 		new_node = selectiveRead(clientID, new_node);
 		//printf("new_node->flag %s\n", new_node->flag);
 		if(strcmp(new_node->flag, UNKNOWN) == 0){
 			return node;
 		}

 		//printf("node->successive_node :%p\n",node->successor);

 		UNUSED(recvBytes);

 		if(strcmp(new_node->flag, CONNECTION_MSG) == 0){
	 			if(node->predecessor == NULL && node->successor == NULL){
	 			printf("I am the first node\n");
	 			node->successor = new_node;
	 			node->predecessor = new_node;

	 			//updating the requested node successor and predeccsor in the list
	 			new_node->successor = node;
	 			new_node->predecessor = new_node->successor;

	 			while(1){
	 				//fflush(NULL);
	 				sendData(clientID, new_node, UPDATE);
		 			temp = selectiveRead(clientID, temp1);

		 			if(strcmp(temp->flag, UNKNOWN) == 0){
						printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
						printf("Resending the Packet\n");
						continue;
					}
					break;
	 			}

			  	printf("Updated  my first node:\n");
				printf(" My position in the network\n");
				temp1 = node->predecessor;
				printf("  My predecessor: %s\n", temp1->hostname);
				temp1 = node->successor;
				printf("  My successor: %s\n", temp1->hostname);
				printf("  My name and Port: %s %s\n",node->hostname, node->Port);

				filesTransfer(node, ISSUCCESSOR);

				goto return_point;

	 			return_point: {return node;}
	 		} else if(node->successor == NULL){
	 				printf("Something happened to ROOT first node. I am taking that position\n");
	 				new_node->predecessor = node;
	 				new_node->successor = node;
	 				node->successor = new_node;
	 				node->predecessor = new_node;

	 				while(1){
	 				//fflush(NULL);
	 				sendData(clientID, new_node, UPDATE);
		 			temp = selectiveRead(clientID, temp1);

		 			if(strcmp(temp->flag, UNKNOWN) == 0){
						printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
						printf("Resending the Packet\n");
						continue;
					}
					break;
	 			}

	 			printf("Updated a new node:\n");
				printf(" My position in the network\n");
				temp1 = node->predecessor;
				printf("  My predecessor: %s\n", temp1->hostname);
				temp1 = node->successor;
				printf("  My successor: %s\n", temp1->hostname);
				printf("  My name and Port: %s %s\n",node->hostname, node->Port);

				goto return_point;
	 		} else{
	 			//printf("before assignNodePosition\n");
	 			new_node= assignNodePosition(clientID, root_name, root_port, node, new_node, node, CYCLE);
	 			///newNodePosition(clientID, node, new_node);
	 			//printf("node is: %p\n",node);
	 			//return new_node;
	 		}
 		}

 	else if(strcmp(new_node->flag, QUERY_MSG) == 0){
 			new_node->predecessor = node->predecessor;
 			new_node->successor = node;
 			temp = node->predecessor;
 			temp->successor = new_node;
 			node->predecessor = new_node;

 			sockID = makeConnection(new_node->hostname, new_node->Port);
 			sendUpdate(sockID, temp, new_node);

 	}  else if(strcmp(new_node->flag, UPDATE_MSG) == 0){
 		sockID = makeConnection(new_node->hostname, new_node->Port);
 		retry = 0;
 		while(1  && retry < 3){
			//fflush(NULL);
			sendData(sockID, new_node, UPDATE);
			temp = selectiveRead(sockID, temp);

			//printf("temp flag: %s\n",temp->flag);
			if(strcmp(temp->flag, UNKNOWN) == 0){
				printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
				printf("Resending the Packet\n");
				++retry;
				continue;
			}
				break;
		}
 	} else if(strcmp(new_node->flag, STATUS_CHECK) == 0){
 		printf("\n--------------------------------------------\n");
 		printf("|| Ring is stable. No dropouts until now ||\n");
 		printf("|| End of Check Cycle ||\n");
 		printf("--------------------------------------------\n");
 		sendData(clientID, new_node, CHECK);
 	} else if(strcmp(new_node->flag, SEVERED) == 0){
 		if(node->predecessor != NULL){
 			if(strcmp(node->predecessor->hostname, new_node->hostname) == 0){
 				node->predecessor = NULL;
 			}
 			sendData(clientID, new_node, UNLINK);
 		}

 		if(node->successor != NULL){
 			if(strcmp(node->successor->hostname, new_node->hostname) == 0){
 				node->successor = NULL;
 			}
 			sendData(clientID, new_node, UNLINK);
 		}
 		goto return_point;
 	}
}
 	//freeNode(new_node);
 	return node;
 }

void newNodePosition(int clientID, struct peer_information* node, struct peer_information* requestingNode){
	char ibuf[BUFFERSIZE];
	unsigned char obuf[HASHSIZE];
	char error[120];

	int result;
	int connectClientID;

	struct peer_information* successive_node = create_node();

	BIGNUM *var1 =NULL;
	var1 = BN_new();
	BIGNUM *var2 = NULL;
	var2 = BN_new();

	if((var1 = BN_new()) == NULL){
		ERR_error_string(ERR_get_error(), error);
		fprintf(stderr, "BIGNUM assignment error %s\n", error);
	}
	if((var2 = BN_new()) == NULL){
		ERR_error_string(ERR_get_error(), error);
		fprintf(stderr, "BIGNUM assignment error %s\n", error);
	}

	successive_node = node->successor;
	printf("successive_node->hostname %s successive_node->Port %s\n",successive_node->hostname, successive_node->Port);

	memset(ibuf, 0, sizeof(char)*BUFFERSIZE);
	memcpy(ibuf, requestingNode->hostname, strlen(requestingNode->hostname));
	printf("length of hostname: %ld\n",strlen(ibuf));
	SHA1((unsigned char*)ibuf, strlen(ibuf), obuf);
	int i;
	for(i=0;i<20;i++){
		printf("%x", obuf[i]);
	}
	printf("\n");
	BN_dec2bn(&var1, (char*)obuf);

	memset(ibuf, 0, sizeof(char)*BUFFERSIZE);
	memcpy(ibuf, successive_node->hostname, strlen(successive_node->hostname));
	printf("length of hostname: %ld\n",strlen(ibuf));
	SHA1((unsigned char*)ibuf, strlen(ibuf), obuf);
	for(i=0;i<20;i++){
		printf("%x", obuf[i]);
	}
	printf("\n");

	BN_dec2bn(&var2, (char*)obuf);

	result = BN_cmp(var1, var2);

	if(result == LESSTHAN){
		printf("Requesting Node %s is smaller than current successor node %s\n",requestingNode->hostname, node->successor->hostname);
		requestingNode->predecessor = node;
		requestingNode->successor = successive_node;
		successive_node->predecessor = requestingNode;
		node->successor = requestingNode;
		if(strcmp(node->mode, ROOT) == 0){
			sendUpdate(clientID, successive_node, requestingNode);
		} else if(strcmp(node->mode, PEER) == 0){
			connectClientID = makeConnection(ROOT_HOSTNAME, ROOT_PORT);
			sendUpdate(connectClientID, successive_node, requestingNode);
		}

	} else if(result == EQUAL){
		printf("Node already present in the network\n");
		/* do nothing*/
	} else if(result == GREATER){
		int connectClientID;
		int retry=0;
		struct peer_information* temp = create_node();

		printf("Requesting Node %s is smaller than current successor node %s\n",requestingNode->hostname, node->successor->hostname);
		connectClientID = makeConnection(successive_node->hostname, successive_node->Port);

		while(1 && retry < 3){
		//fflush(NULL);
		sendData(connectClientID, successive_node, QUERY);
		temp = selectiveRead(connectClientID, temp);

			//printf("temp flag: %s\n",temp->flag);
			if(strcmp(temp->flag, UNKNOWN) == 0){
				printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
				printf("Resending the Packet\n");
				++retry;
				continue;
			}
			break;
		}

	free(temp);

	}
}

void sendUpdate(int clientID, struct peer_information* successive_node, struct peer_information* requestingNode){
	int connectClientID;
	int retry=0;
	struct peer_information* temp = create_node();

	connectClientID = makeConnection(successive_node->hostname, successive_node->Port);

	while(1 && retry < 3){
		//fflush(NULL);
		sendData(connectClientID, successive_node, UPDATE);
		temp = selectiveRead(connectClientID, temp);

		//printf("temp flag: %s\n",temp->flag);
		if(strcmp(temp->flag, UNKNOWN) == 0){
			printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
			printf("Resending the Packet\n");
			++retry;
			continue;
		}
			break;
	}

	free(temp);

	temp = create_node();
	retry = 0;

	while(1 && retry < 3){
		//fflush(NULL);
		sendData(clientID, requestingNode, UPDATE);
		temp = selectiveRead(clientID, temp);

		//printf("temp flag: %s\n",temp->flag);
		if(strcmp(temp->flag, UNKNOWN) == 0){
			printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
			printf("Resending the Packet\n");
			++retry;
			continue;
		}
			break;
	}

	freeNode(temp);
}

void sendStatusCheck(struct peer_information* currentNode){
	int connectClientID, clientID;
	int retry=0;
	struct peer_information* toCheckNode;
	struct peer_information* temp = create_node();

	toCheckNode = currentNode->successor;

	if(toCheckNode != NULL){
		connectClientID =makeConnection(toCheckNode->hostname, toCheckNode->Port);
		while(1 && retry < 3 && connectClientID>=0){
			//fflush(NULL);
			sendData(connectClientID, toCheckNode, CHECK);
			temp = selectiveRead(connectClientID, temp);

			//printf("temp flag: %s\n",temp->flag);
			if(strcmp(temp->flag, UNKNOWN) == 0){
				printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
				printf("Resending the Packet\n");
				++retry;
				continue;
			} else if(strcmp(temp->flag, STATUS_CHECK) == 0){
				/*printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
				printf("|| My successor %s is alive ||\n", toCheckNode->hostname);
				printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");*/
				fflush(NULL);
				break;
			} else if(strcmp(temp->flag, NORESPONSE) == 0){
				printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
				printf("|| No response received from my successor %s ||\n", toCheckNode->hostname);
				printf("|| HE MIGHT BE DEAD ||\n");
				printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");

				//send  data to my predecessor to UNLINK
				clientID = makeConnection(currentNode->predecessor->hostname, currentNode->predecessor->Port);
				sendData(clientID, currentNode, UNLINK);
				freeNode(temp);
				temp = create_node();
				temp = selectiveRead(clientID, temp);

				if(strcmp(temp->flag, STATUS_CHECK) == 0){
					printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
					printf("|| Unlinked with my predecessor %s||\n", currentNode->predecessor->hostname);
					printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
				}

				currentNode->successor = NULL;
				break;
		}
	}

	if(connectClientID < 0){
		printf("Client ID is less than zero\n");
		printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
		printf("|| No response received from my successor %s||\n", toCheckNode->hostname);
		printf("|| HE MIGHT BE DEAD ||\n");
		printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");

		currentNode->successor = NULL;
		//send  data to my predecessor to UNLINK
		clientID = makeConnection(ROOT_HOSTNAME, ROOT_PORT);
		if(clientID >= 0){
				sendData(clientID, currentNode, UNLINK);
				freeNode(temp);
				temp = create_node();
				temp = selectiveRead(clientID, temp);

				if(strcmp(temp->flag, SEVERED) == 0){
					printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
					printf("|| Unlinked with my predecessor ||\n");
					printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
					currentNode->predecessor = NULL;
				}
		} else{
			currentNode->predecessor = NULL;
		}
	}

	//closed connection with my predecessor and successor
	close(clientID);
	close(connectClientID);

	freeNode(temp);
}
}
 struct peer_information* assignNodePosition(int clientID, char* root_name, char* root_port, struct peer_information *currentNode,
 	struct peer_information *requestingNode, struct peer_information *root_node, int cycle){

 	char ibuf[100], ibuf2[100];
 	char error[120];
	unsigned char obuf[20];
	unsigned char obuf2[20];
 	int result; //retry =0;
 	int connectClientID;
 	int retry = 0;

 	struct peer_information *successive_node;
 	struct peer_information* return_node = create_node();
 	struct peer_information *temp;
 	struct peer_information *temp1 =create_node();

 	//printf("Entered assignNodePosition\n");

	if(strcmp(currentNode->hostname, root_node->hostname) == 0){
		++cycle;
		//printf("Iteration: %d\n", cycle);
		if(cycle == 2){
			//printf("Cycle returned to initial state\n");
			requestingNode->successor =  root_node;
			requestingNode->predecessor = root_node->predecessor;
			struct peer_information *predecessive_node = root_node->predecessor;
			//printf("predecessive_node  name: %s\n",predecessive_node->hostname );
			predecessive_node->successor = requestingNode;
			root_node->predecessor = requestingNode;
			//printf("success\n");

			while(1){
 				//fflush(NULL);
 				sendData(clientID, requestingNode, UPDATE);
	 			temp = selectiveRead(clientID, temp1);

	 			//printf("temp flag: %s\n",temp->flag);
	 			if(strcmp(temp->flag, UNKNOWN) == 0){
					printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
					printf("Resending the Packet\n");
					continue;
				}
				break;
 			}

			connectClientID = makeConnection(predecessive_node->hostname, predecessive_node->Port);

			while(1){
 				//fflush(NULL);
 				sendData(connectClientID, predecessive_node, UPDATE);
	 			temp = selectiveRead(connectClientID, temp1);

	 			//printf("temp flag: %s\n",temp->flag);
	 			if(strcmp(temp->flag, UNKNOWN) == 0){
					printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
					printf("Resending the Packet\n");
					continue;
				}
				break;
 			}

			printf("Updated node:\n");
			printf(" My position in the network\n");
			temp1 = root_node->predecessor;
			printf("  My predecessor: %s\n", temp1->hostname);
			temp1 = root_node->successor;
			printf("  My successor: %s\n", temp1->hostname);
			printf("  My name and Port: %s %s\n",root_node->hostname, root_node->Port);

			printf("Updated node: requestingNode\n");
			printf(" My position in the network\n");
			temp1 = requestingNode->predecessor;
			printf("  My predecessor: %s\n", temp1->hostname);
			temp1 = requestingNode->successor;
			printf("  My successor: %s\n", temp1->hostname);
			printf("  My name and Port: %s %s\n",requestingNode->hostname, requestingNode->Port);

			printf("Root hostname: %s\n",root_node->hostname);
			printf("Root Port and mode: %s & %s\n",root_node->Port, root_node->mode);
			temp = root_node->predecessor;
			printf("Root predecessor: %s\n",temp->hostname);
			temp = root_node->successor;
			printf("Root successor: %s\n", temp->hostname);


			temp1 = NULL;
			//printf("root_node value: %p\n",root_node);
			//printf("root_node value: %p\n",temp1);
			temp1 = root_node;
			return temp1;
			//goto end_exec;
		}
	}

	BIGNUM *var1 =NULL;
	var1 = BN_new();
	BIGNUM *var2 = NULL;
	var2 = BN_new();

	if((var1 = BN_new()) == NULL){
		ERR_error_string(ERR_get_error(), error);
		fprintf(stderr, "BIGNUM assignment error %s\n", error);
	}
	if((var2 = BN_new()) == NULL){
		ERR_error_string(ERR_get_error(), error);
		fprintf(stderr, "BIGNUM assignment error %s\n", error);
	}

	//printf("\ncurrentNode hostname: %s\n", currentNode->hostname);
	//printf("requestingNode hostname: %s\n",requestingNode->hostname);

	memset(ibuf, 0, sizeof(char)*100);
	memset(ibuf2, 0, sizeof(char)*100);

	memcpy(ibuf, requestingNode->hostname, strlen(requestingNode->hostname));
	SHA1((unsigned char*)ibuf, strlen(ibuf), obuf);
	BN_dec2bn(&var1, (char*)obuf);

	/*int i;
	for(i=0; i<strlen(requestingNode->hostname);i++){
		printf("%c ",ibuf[i]);
	}
	printf("M\n");*/

	successive_node = currentNode->successor;
	memcpy(ibuf2, successive_node->hostname, strlen(successive_node->hostname));
	SHA1((unsigned char*)ibuf2, strlen(ibuf2), obuf2);

	/*for(i=0; i<strlen(successive_node->hostname);i++){
		printf("%c ",ibuf2[i]);
	}
	printf("M\n");*/

	BN_dec2bn(&var2, (char*)obuf2);
	result = BN_cmp(var1, var2);

	BN_free(var1);
	BN_free(var2);

	if(result == -1){
		printf("node ID of %s is smaller than exisitng successor node ID: %s\n", requestingNode->hostname, successive_node->hostname);
		requestingNode->predecessor = successive_node->predecessor;
		requestingNode->successor = successive_node;

		temp = requestingNode->predecessor;
		if(strcmp(temp->hostname, root_node->hostname) == 0){
			root_node->successor = requestingNode;
		} else{

			while(1 && retry < 3){
 				//fflush(NULL);
 				temp = successive_node->predecessor;
				temp->successor = requestingNode;

				//printf("temp->hostname: %s\n",temp->hostname);
				connectClientID = makeConnection(temp->hostname, temp->Port);
				if(connectClientID > 0){
					sendData(connectClientID, temp, UPDATE);
		 			temp = selectiveRead(connectClientID, temp1);
		 		//	printf("temp->hostname: %s\n",temp->hostname);

		 			//printf("temp flag: %s\n",temp->flag);
		 			if(strcmp(temp->flag, UNKNOWN) == 0){
						printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
						printf("Resending the Packet\n");
						continue;
					}
					break;
				} else{
					printf("Connection could not be established with %s\n",temp->hostname);
					++retry;
					printf("Failed attempt to connect %d. Retrying...\n",retry);
					continue;
				}
 			}

 			if(retry == 3){
 				printf("Failed to update the position.\n");
 				sendData(clientID, requestingNode, FAILED);
 			}
 		}

		//printf("successive_node's predecessor: %s\n","Something");

		while(1){
			//fflush(NULL);
			sendData(clientID, requestingNode, UPDATE);
			//printf("temp->hostname: %s\n",temp1->hostname);
			temp = selectiveRead(clientID, requestingNode);

			//printf("temp flag: %s\n",temp->flag);
			if(strcmp(temp->flag, UNKNOWN) == 0){
				printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
				printf("Resending the Packet\n");
				continue;
			}
			break;
 		}

		if(strcmp(successive_node->hostname, root_node->hostname) == 0){
			root_node->predecessor = requestingNode;
		} else{
			successive_node->predecessor = requestingNode;
			connectClientID = makeConnection(successive_node->hostname, successive_node->Port);

			while(1){
				//fflush(NULL);
				sendData(connectClientID, successive_node, UPDATE);
				printf("before waiting to read message\n");
				temp = selectiveRead(connectClientID, temp1);

				//printf("temp flag: %s\n",temp->flag);
				if(strcmp(temp->flag, UNKNOWN) == 0){
					printf("Sent Update. Instead of ACK packet, Received Message is: %s", temp->hostname);
					printf("Resending the Packet\n");
					continue;
				}
					break;
			}
		}

			/*printf("Updated node: successor\n");
			printf(" Successor position in the network\n");
			temp = successive_node->predecessor;
			printf("  Successor's predecessor: %s\n", temp->hostname);
			temp = successive_node->successor;
			printf("  Successor's successor: %s\n", temp->hostname);
			printf("  Successor name and Port: %s %s\n",successive_node->hostname, successive_node->Port);*/

			printf("My Updated Information\n");
			printf(" Root hostname: %s\n",root_node->hostname);
			printf(" Root Port and mode: %s & %s\n",root_node->Port, root_node->mode);
			temp = root_node->predecessor;
			printf(" Root predecessor: %s\n",temp->hostname);
			temp = root_node->successor;
			printf(" Root successor: %s\n", temp->hostname);

			printf("Updated node: requestingNode\n");
			printf(" My position in the network\n");
			temp = requestingNode->predecessor;
			printf("  My predecessor: %s\n", temp->hostname);
			temp = requestingNode->successor;
			printf("  My successor: %s\n", temp->hostname);
			printf("  My name and Port: %s %s\n",requestingNode->hostname, requestingNode->Port);

			temp1 = NULL;
			//printf("root_node value: %p\n",root_node);
			return_node = root_node;
			//return root_node;
			goto end_exec;

	} else if(result == 0){
		printf("Node already exists in the network\n");
		sendData(clientID, successive_node, UPDATE);
		return root_node;

	} else if(result == 1){
		//recursive return
		printf("Node ID of %s is larger than exisitng successor node ID: %s\n", requestingNode->hostname, successive_node->hostname);
		assignNodePosition(clientID, root_name, root_port, successive_node, requestingNode, root_node, cycle);
	}

	/*return ZERO;*/
	end_exec: return return_node;
 }
