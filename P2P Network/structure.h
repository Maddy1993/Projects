#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define HASHSIZE 20
#define HOSTLEN 255
#define PORTLEN 20
#define MODELEN 5
#define MSGLEN 512

extern char* ROOT;
extern char* PEER;
extern char ROOT_HOSTNAME[HOSTLEN];
extern char ROOT_PORT[PORTLEN];

#ifndef STRUCT_H
#define STRUCT_H

struct peer_information{
	struct peer_information *predecessor;
	FILE *indexfile;
	char hashID[HASHSIZE];
	char hostname[HOSTLEN];
	char Port[PORTLEN];
	char mode[MODELEN];
	char flag[MSGLEN];
	struct peer_information *successor;
};

struct peer_information *create_node();
void assignROOT(char*, char*);

struct peer_information *initializeNode(struct peer_information *, struct peer_information *, 
	char*, char*, char*, char*, char*);

void freeNode(struct peer_information*);

#endif //STRUCT_H