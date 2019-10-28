#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "connection.h"
#include "rootClient.h"
#include "structure.h"
#include "transfer.h"

void* peerRunner(void* arg);

void peerThread(struct peer_information* node, int predecessorID){
	pthread_t peerThread;
	pthread_attr_t attr;
	int returnCode;

	struct arguments *peerArg = malloc(sizeof(struct arguments));

	peerArg->node = node;
	peerArg->clientID = predecessorID;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	//create a thread for handle listen connections. A detached thread
	returnCode = pthread_create(&peerThread, NULL, peerRunner, peerArg);  		
	if (returnCode){
		printf("ERROR; return code from pthread_create() is %d\n", returnCode);
	}

	pthread_attr_destroy(&attr);

	returnCode = pthread_join(peerThread, NULL);
	if (returnCode){
		printf("ERROR; return code from pthread_join() is %d\n", returnCode);
	}	
}

void* peerRunner(void* arg){
	int recvBytes=0;
	char dataReceived[INTERNAL_PACKET_SIZE];
	char *curAddr;
	long fileName_len;
	char fileName[FILENAME_LENGTH];

	struct arguments *threadArg = (struct arguments*) arg;
	struct peer_information* currentNode = threadArg->node;
	int predecessorID = threadArg->clientID;
	//struct peer_information* temp = create_node();

	//temp = selectiveRead(predecessorID, temp);
	//printf("temp->flag %s\n",temp->flag);
	//memset(fileName, 0, ARGMODELEN);
	//memcpy(fileName, temp->flag, strlen(temp->flag));

	if((recvBytes = recv(predecessorID, dataReceived, INTERNAL_PACKET_SIZE, 0)) == -1){	//receive data from the client: HELLO Message	
		fprintf(stderr, "Reading error: %s\n", strerror(errno));
	}

	curAddr = dataReceived;
	memcpy(&fileName_len, curAddr, sizeof(long));

	curAddr += sizeof(long);
	memcpy(fileName, curAddr, fileName_len);

	printf("fileName received %s\n",fileName);

	checkandStore(predecessorID, fileName, currentNode);
	pthread_exit(NULL);
}