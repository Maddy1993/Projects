#include "structure.h"

#ifndef TRANSFER_H
#define TRANSFER_H

extern const int REQUEST_TO_CONNECT;
extern const int ACK;
extern const int UPDATE;
extern const int READ;
extern const int CHECK;
extern const int QUERY;
extern const int STOREOBJ;
extern const int DATAPACKET;
extern const int FAILED;
extern const int SUCCESS;
extern const int RESPONSE;

extern const int NULL_FLAG;
extern const int NOTNULL_FLAG;
extern const int NULL_NODE;
extern const int PEERVAL;
extern const int ROOTVAL;
extern const int UNLINK;
extern const int END;

extern const char* ACK_MSG;
extern const char* UNKNOWN;
extern const char* STATUS_CHECK;
extern const char* SEVERED;
extern const char* CONNECTION_MSG;
extern const char* QUERY_MSG;
extern const char* UPDATE_MSG;
extern const char* NORESPONSE;
extern const char* STOREOBJECT;
extern const char* STOREDATA;
extern const char* FAILED_MSG;

void sendData(int, struct peer_information*, int);
struct peer_information* readData(int, struct peer_information*);
char* getIPfromName(char*);
char* packConnectionPacket(struct peer_information*, char*);

#endif //DHT_PEER_H