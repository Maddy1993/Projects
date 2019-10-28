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

#include "structure.h"
#include "convert.h"
#include "transfer.h"
#include "client.h"

#define PACKET_SIZE 3000
#define UNUSED(x) (void)(x)

char hostname[20];
const int REQUEST_TO_CONNECT = 1;
const int ACK = 2;
const int UPDATE = 3;
const int READ = 4;
const int CHECK = 5;
const int UNLINK = 6;
const int QUERY = 7;
const int STOREOBJ = 12;
const int DATAPACKET = 13;
const int FAILED = 15;
const int SUCCESS = 16;
const int RESPONSE = 17;

const int NOTNULL_FLAG = 8;
const int NULL_FLAG = 9;
const int NULL_NODE = -10;
const int PEERVAL = 10;
const int ROOTVAL = 11;
const int END = 14;

const char* ACK_MSG = "Ack received";
const char* UNKNOWN = "Unknown Data";
const char* STATUS_CHECK = "Status Check";
const char* SEVERED = "Connection Broken";
const char* CONNECTION_MSG = "Requesting to Connect";
const char* QUERY_MSG = "Find the place for this node";
const char* UPDATE_MSG = "Update yourself";
const char* NORESPONSE = "No Response";
const char* STOREOBJECT = "Request to Store Object";
const char* STOREDATA = "Request to Store Data";
const char* FAILED_MSG = "Failed to connect";

char* getIPfromName(char* dns_name){
	struct hostent *h;
	struct in_addr **addr_list;

	if((h = gethostbyname(dns_name)) == NULL) {  // get the host info
        herror("gethostbyname");
        return dns_name;
    }

	 // print information about this host:
    addr_list = (struct in_addr **)h->h_addr_list;

	strcpy(hostname, inet_ntoa(*addr_list[0]));
	//printf("IP Address: %s\n",hostname);

	return hostname;
}
void sendData(int clientID, struct peer_information *peer_node, int packetType){

	int sentBytes;
	long bufferSize;

	char dataSent[PACKET_SIZE];
	char *currAddress;

	strcpy(hostname, getIPfromName(peer_node->hostname));

	memset(dataSent, 0,PACKET_SIZE);

	//copy the  struct header to an array to send the over the network
	currAddress = dataSent;
	memcpy(currAddress, &packetType, sizeof(int));
	//printf("Packed: packet type: %d\n", packetType);

	currAddress += sizeof(int);

	if(packetType == REQUEST_TO_CONNECT || UPDATE){
		currAddress = packConnectionPacket(peer_node, currAddress);
	} else if(packetType == ACK)
		/*do nothin*/

	fflush(NULL);

	bufferSize = currAddress - dataSent;

	//send a request to the root.
	if((sentBytes = send(clientID, dataSent, bufferSize, 0)) == -1){
		fprintf(stderr, "Send error: %s\n", strerror(errno));
		//exit(EXIT_FAILURE);
	}
	//printf("sentBytes %d\n",sentBytes);
	//printf("%d type Message Sent to %s\n", packetType, peer_node->hostname);
	UNUSED(sentBytes);

}

char* packConnectionPacket(struct peer_information* peer_node, char* packetPoint){

	char * currAddress = packetPoint;
	struct peer_information *temp;
	int ip_addr;

	if(peer_node->predecessor == NULL){
		memcpy(currAddress, &NULL_FLAG, sizeof(int));
		currAddress += sizeof(int);

		memcpy(currAddress, &NULL_NODE, sizeof(int));
		//printf("Packet: predecessor is not present\n");
		currAddress += sizeof(int);
	} else{
		memcpy(currAddress, &NOTNULL_FLAG, sizeof(int));
		currAddress += sizeof(int);

		temp = peer_node->predecessor;
		ip_addr = ip_str_to_int(temp->hostname);
		memcpy(currAddress, &ip_addr, sizeof(int));

		currAddress += sizeof(int);
		int port = atoi(temp->Port);
		memcpy(currAddress, &port, sizeof(int));

		currAddress += sizeof(int);

		//printf("Packed: Node predecessor hostname: %s and Port: %s\n",temp->hostname, temp->Port);
	}

	if(peer_node->successor == NULL){
		memcpy(currAddress, &NULL_FLAG, sizeof(int));
		currAddress += sizeof(int);

		memcpy(currAddress, &NULL_NODE, sizeof(int));
		//printf("Packerd: successor is not present\n");
		currAddress += sizeof(int);
	} else{
		memcpy(currAddress, &NOTNULL_FLAG, sizeof(int));
		currAddress += sizeof(int);

		temp = peer_node->successor;
		ip_addr = ip_str_to_int(temp->hostname);
		memcpy(currAddress, &ip_addr, sizeof(int));

		currAddress += sizeof(int);
		int port = atoi(temp->Port);
		memcpy(currAddress, &port, sizeof(int));

		currAddress += sizeof(int);

		//printf("Packed: Node successor hostname: %s and Port: %s\n",temp->hostname, temp->Port);
	}

	int port = atoi(peer_node->Port);
	memcpy(currAddress, &port, sizeof(int));
	//printf("Packed: Port: %s\n", peer_node->Port);

	currAddress += sizeof(int);
	memcpy(currAddress, &peer_node->hashID, sizeof(char) * HASHSIZE);
	//printf("Packed: peer_node hashID: %s\n", peer_node->hashID);

	currAddress +=  (sizeof(char) * HASHSIZE);
	ip_addr = ip_str_to_int(hostname);
	memcpy(currAddress, &ip_addr, sizeof(int));
	//printf("Packed: IP %d\n", ip_addr);

	currAddress += sizeof(int);
	if(strcmp(peer_node->mode, PEER) == 0){
		memcpy(currAddress, &PEERVAL, sizeof(int));
		currAddress += sizeof(int);
	} else if (strcmp(peer_node->mode, ROOT) == 0){
		memcpy(currAddress, &ROOTVAL, sizeof(int));
		currAddress += sizeof(int);
	}

	//printf("Packet Connection Packet\n");
	return currAddress;
}

struct peer_information* readData(int clientID, struct peer_information *new_node){
	int recvBytes=0;
	int connection_request=0, flag_value=0;
	int port=0, address=0;
	char *result;
	char dataReceived[PACKET_SIZE];
	char *currAddress;

	UNUSED(dataReceived);
	UNUSED(recvBytes);
	memset(dataReceived, 0, PACKET_SIZE);

	struct peer_information* temp = create_node();

	if((recvBytes = recv(clientID, dataReceived, PACKET_SIZE-1, 0)) == -1){	//receive data from the client: HELLO Message
			fprintf(stderr, "Reading error: %s\n", strerror(errno));
	}
	printf("bytes read: %d\n",recvBytes);

	currAddress = dataReceived;
	memcpy(&connection_request, currAddress, sizeof(int));
	//printf("Received Data: connection_request:  %d\n", connection_request);
	currAddress += sizeof(int);

	if(connection_request == ACK){
		//printf("Ack received\n");
		memset(temp->flag, 0, MSGLEN);
		memcpy(temp->flag, ACK_MSG, strlen(ACK_MSG));
		new_node = temp;
	//	printf("In transfer.c: new_node->flag %s\n",new_node->flag);
		free(temp);
		return new_node;
	} else if(connection_request == CHECK){
		//printf("Status Check packet received\n");
		memset(new_node->flag, 0, MSGLEN);
		memcpy(new_node->flag, STATUS_CHECK, strlen(STATUS_CHECK));
		//new_node = temp;
		//free(temp);
		//return new_node;
	} else if(connection_request == UNLINK){
		printf("Unlink request received\n");
		memset(new_node->flag, 0, MSGLEN);
		memcpy(new_node->flag, SEVERED, strlen(SEVERED));
		//new_node = temp;
		//free(temp);
		//return new_node;
	} else if(connection_request == QUERY){
		//printf("Query Message Received.\n");
		memset(temp->flag, 0, MSGLEN);
		memcpy(temp->flag, QUERY_MSG, strlen(QUERY_MSG));
		new_node = temp;
		free(temp);
		//return new_node;
	} else if(connection_request == REQUEST_TO_CONNECT){
		//printf("Connection Request Received.\n");
		memset(new_node->flag, 0, MSGLEN);
		memcpy(new_node->flag, CONNECTION_MSG, strlen(CONNECTION_MSG));
		//new_node = temp;
		//free(temp);
		//return new_node;
	} else if(connection_request == UPDATE){
		//printf("Update Message Received.\n");
		memset(new_node->flag, 0, MSGLEN);
		memcpy(new_node->flag, UPDATE_MSG, strlen(UPDATE_MSG));
		//new_node = temp;
		//free(temp);
		//return new_node;
	}  else if(connection_request == STOREOBJ){
		//printf("Request to store object Received.\n");
		memset(new_node->flag, 0, MSGLEN);
		memcpy(new_node->flag, STOREOBJECT, strlen(STOREOBJECT));
	} else if(connection_request == DATAPACKET){
		//printf("Request to store data Received.\n");
		memset(new_node->flag, 0, MSGLEN);
		memcpy(new_node->flag, STOREDATA, strlen(STOREDATA));
	} else if(connection_request == _ITERATIVE){
		printf("Request to retrieve object iteratively.\n");
		memset(new_node->flag, 0, MSGLEN);
		memcpy(new_node->flag, ITERATIVE, strlen(ITERATIVE));
	} else if(connection_request == _RECURSIVE){
		printf("Request to retrieve object recursively.\n");
		memset(new_node->flag, 0, MSGLEN);
		memcpy(new_node->flag, RECURSIVE, strlen(ITERATIVE));
	}



	memcpy(&flag_value, currAddress, sizeof(int));
	//printf("predecessor flag_value: %d\n",flag_value);

	currAddress  += sizeof(int);

	if(flag_value == 9){
		new_node->predecessor = NULL;
	//	printf(" predecessor: %p\n", new_node->predecessor);
		currAddress += sizeof(int);
	} else if(flag_value == 8){

		temp = create_node();
		memcpy(&address, currAddress, sizeof(int));
	//	printf("address received: %d\n", address);
		result = ip_int_to_str(address);
	//	printf("result: %s\n", result);
		memcpy(temp->hostname, result, strlen(result));
		currAddress += sizeof(int);

		memcpy(&port, currAddress, sizeof(int));
		sprintf(temp->Port, "%d", port);
		currAddress += sizeof(int);

		new_node->predecessor = temp;

	//	printf(" UnPacked: Node predecessor hostname: %s and Port: %s\n",temp->hostname, temp->Port);
	}

	memcpy(&flag_value, currAddress, sizeof(int));
	//printf("successor flag_value is: %d\n",flag_value);

	currAddress  += sizeof(int);

	if(flag_value == 9){
		new_node->successor = NULL;
	//	printf(" successor: %p\n", new_node->successor);
		currAddress += sizeof(int);
	} else if(flag_value == 8){
		temp = create_node();
		memcpy(&address, currAddress, sizeof(int));
	//	printf("address received: %d\n", address);
		result = ip_int_to_str(address);
	//	printf("result: %s\n", result);
		memcpy(temp->hostname, result, strlen(result));
		currAddress += sizeof(int);

		memcpy(&port, currAddress, sizeof(int));
		sprintf(temp->Port, "%d", port);
		currAddress += sizeof(int);

		new_node->successor = temp;

	//	printf(" UnPacked: Node successor hostname: %s and Port: %s\n",temp->hostname, temp->Port);
	}

	memcpy(&port, currAddress, sizeof(int));
	sprintf(new_node->Port, "%d", port);
	///printf(" Port: %s\n", new_node->Port);

	currAddress += sizeof(int);
	memcpy(new_node->hashID, currAddress, sizeof(char) * HASHSIZE);
	//printf(" hashID: %s\n",new_node->hashID);

	currAddress +=  (sizeof(char) * HASHSIZE);
	memcpy(&address, currAddress, sizeof(int));
	//printf("address received: %d\n", address);
	result = ip_int_to_str(address);
	//printf("result: %s\n", result);
	memcpy(new_node->hostname, result, strlen(result));
	//printf(" hostname: %s\n", new_node->hostname);

	currAddress += sizeof(int);

	memcpy(&address, currAddress, sizeof(int));
	if(address == PEERVAL){
		memcpy(new_node->mode, PEER, strlen(PEER));
	} else if(address == ROOTVAL){
		memcpy(new_node->mode, ROOT, strlen(ROOT));
	}

	currAddress += sizeof(int);
	//currAddress += sizeof(char) * IPSIZE;
//	printf("currAddress buffer: %ld\n", currAddress - dataReceived);
	UNUSED(dataReceived);

	//free(temp);
	//printf("Data Reading completed\n");
	return new_node;
}
