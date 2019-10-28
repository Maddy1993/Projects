#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<netinet/in.h>

#include "structure.h"

#define CLIENT_PORT "15099"
#define PACKET_SIZE 512
#define MAX_FILE_NAME_LENGTH 512

#ifndef NODE_H
#define NODE_H

void peerFunctionCalls(struct addrinfo *, struct addrinfo *, char*, char*, char*, char*, char*, struct peer_information*);
void rootFuncationCalls(struct addrinfo *, struct addrinfo *, char*, char*, char*, char*, char*, struct peer_information*);

#endif //NODE_H