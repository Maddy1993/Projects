#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>

#include "structure.h"

extern const int CONNECTION_ERROR;

#define ARGMODELEN 512

#ifndef CONNECTION_H
#define CONNECTION_H

struct arguments{
	struct peer_information* node;
	char mode[ARGMODELEN];
	int clientID;
};

pthread_mutex_t mutexVar;

int initializeConnections(struct addrinfo *);
int initializePeerConnections(struct addrinfo *);
void initializePeer(char**);
int listenAccept(int, int, char*, struct peer_information*, char*);
int makeConnection(char*, char*);

//args: 
//int: sockID to read data from
//struct peer_information* : Struct Pointer to Unpack the Data received
struct peer_information* selectiveRead(int, struct peer_information*);

#endif //CONNECTION_H