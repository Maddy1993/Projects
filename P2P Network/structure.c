#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "structure.h"

char* ROOT = "root";
char* PEER = "peer";
char ROOT_HOSTNAME[HOSTLEN];
char ROOT_PORT[PORTLEN];

void assignROOT(char* name, char* port){
	//printf("Entered\n");
	memset(ROOT_HOSTNAME, 0, sizeof(char)*HOSTLEN);
	memset(ROOT_PORT, 0, sizeof(char)*PORTLEN);
	memcpy(ROOT_HOSTNAME, name, strlen(name));
	memcpy(ROOT_PORT, port, strlen(port));
}

struct peer_information *create_node(){
	struct peer_information *new_node = (struct peer_information*)malloc(sizeof(struct peer_information));
	//printf("Node assingned empty space\n");
	new_node->predecessor = NULL;
	//printf("predecessor initialized\n");
	new_node->successor = NULL;
	//printf("successor initialized\n");
	memset(new_node->hashID, 0, HASHSIZE);
	memset(new_node->hostname, 0, HOSTLEN);
	memset(new_node->Port, 0, PORTLEN);
	memset(new_node->mode, 0, MODELEN);
	memset(new_node->flag, 0, MSGLEN);

	return new_node;
}

struct peer_information *initializeNode(struct peer_information *successor,
	struct peer_information *predecessor, char* hashID, char* Port,
	char* hostname, char* mode, char* msg)
{
	struct peer_information *node = create_node();
	node->predecessor = predecessor;
	node->successor = successor;
	//node->indexfile = fopen("index.txt", "a+");
	//snprintf(node->hashID, sizeof(node->hashID), "%s", hashID);
	memcpy(node->hashID, hashID, HASHSIZE);
	memcpy(node->hostname, hostname, HOSTLEN);
	memcpy(node->Port, Port, PORTLEN);
	memcpy(node->mode, mode, MODELEN);
	memcpy(node->flag, msg, MSGLEN);
	//sprintf(node->update, "%s", "false");
	//printf("Initializing node:\n    predecessor: %p\n    successor: %p\n    node hashID: %s\n    hostname: %s\n    host port: %s\n", node->predecessor,node->successor,node->hashID,node->hostname,node->Port);

	return node;
}

void freeNode(struct peer_information* node){
	free(node);
}
