#include <stdio.h>

#include "structure.h"

extern struct peer_information* destNode;

#ifndef OBJECT_H
#define OBJECT_H

int sendName(char*, int);
int sendObjectData(char*, int, int);
void sendPacket(int, char *, int);

#endif