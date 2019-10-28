#include <stdio.h>

#include "structure.h"

#define INTERNAL_PACKET_SIZE 600
#define RETRIEVE_PACKET_SIZE 520
#define FILENAME_LENGTH 512

extern char fileVal[512];

#ifndef ROOTCLIENT_H
#define ROOTCLIENT_H

void * clientRunner(void*);
void checkandStore(int, char*, struct peer_information*);
void sendDatainParts(int, char*, char*);
void writetoFile(char*, char*, long, FILE*);
void processPacket(int, struct peer_information*);

#endif