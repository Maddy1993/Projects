#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<netdb.h>
#include<netinet/in.h>
#include<unistd.h>
#include<time.h>
#include<sys/stat.h>
#include<sys/select.h>
#include<openssl/sha.h>

#include "node.h"
#include "structure.h"
#include "dht_peer.h"
#include "connection.h"
#include "transfer.h"

#define HOST_PORT "15096"
#define HASHSIZE 20
#define WAIT_CONN 0

/////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  //	printf("command type: %s\n", argv[1]);
 if(strcmp(argv[1], "-m") == 0){
 	if(strcmp(argv[2], "0") == 0){
 		if(argc < 11){
			fprintf(stderr, "./dht_peer <-m type> <-p own_port> <-h own_hostname> <-r root_port> <-R root_hostname>\n");
			exit(EXIT_FAILURE);
 		} else{
 			initializePeer(argv);
 		}
 	} else if (strcmp(argv[2], "1") == 0){
 		if(argc < 7){
			  fprintf(stderr, "Usage: ./dht_peer <-m type> <-p own_port> <-h own_hostname> \n");
			  exit(EXIT_FAILURE);
		} else{
			initializePeer(argv);
		}
 	}
 } else if(strcmp(argv[1], "-p") == 0){
	 	if(argc < 9){
			fprintf(stderr, "./dht_peer <-p own_port> <-h own_hostname> <-r root_port> <-R root_hostname>\n");
			exit(EXIT_FAILURE);
	 	} else
	 		initializePeer(argv);
 }

 return 0;
}

void initializePeer(char* argv[]){

//----------------------Variable Declarations------------------------//
 struct addrinfo Caddr;
 struct addrinfo *rlist, *brlist, *clist;
 int status;

 char hostname[255];
 char mode[10];
 char root_port[6];
 char root_hostname[255];
 char own_port[100];

 char emptyMSG[MSGLEN];
 struct peer_information *new_node = create_node();

//-------------------------Initializing the Struct addrinfo--------------------------//

 Caddr.ai_family = AF_INET;		//For IPv4 or IPv6
 Caddr.ai_socktype = SOCK_STREAM;	//UDP	 Stream
// Caddr.ai_flags = 0;
 Caddr.ai_flags = AI_CANONNAME;		//For looking up the canonical name of the DNS Server.
 Caddr.ai_protocol = 0;
//---------------------------Handle Command line arguments---------------------------//

memset(hostname, 0,255);
memset(root_port, 0,255);
memset(root_hostname, 0,255);
memset(own_port, 0,100);

memset(emptyMSG, 0, sizeof(char)*MSGLEN);
//gethostname(hostname, 255);

if(strcmp(argv[1], "-m") == 0){
	if(strcmp(argv[2], "0") ==0){
		memcpy(mode, argv[2], strlen(argv[2]));
		memcpy(root_port, argv[8], strlen(argv[8]));
		memcpy(root_hostname, argv[10], strlen(argv[10]));
		memcpy(own_port, argv[4], strlen(argv[4]));
		memcpy(hostname, argv[6], strlen(argv[6]));

		/*printf("Mode: %s\n", mode);
		printf("root_port: %s\n", root_port);
		printf("root_hostname: %s\n", root_hostname);*/

		memset(&Caddr, '\0', sizeof Caddr); 	//Set 0 to Caddr for sizeof(struct addrinfo)  bits
		if((status = (getaddrinfo(hostname, own_port, &Caddr, &rlist))) != 0){		//getaddrinfo function initializes the Struct addrinfo with relevat data
			fprintf(stderr,  "bind getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
 		}

 		memset(&Caddr, '\0', sizeof Caddr); 	//Set 0 to Caddr for sizeof(struct addrinfo)  bits
 		if((status = (getaddrinfo(root_hostname, root_port, &Caddr, &brlist))) != 0){		//getaddrinfo function initializes the Struct addrinfo with relevat data
			fprintf(stderr,  "connect getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
 		}

 		assignROOT(getIPfromName(root_hostname), root_port);
		peerFunctionCalls(rlist, brlist, mode, root_hostname, root_port, hostname, own_port, new_node);
		freeNode(new_node);
	} else if(strcmp(argv[2], "1") == 0){
		memcpy(mode, argv[2], strlen(argv[2]));
		memcpy(own_port, argv[4], strlen(argv[4]));
		memcpy(hostname, argv[6], strlen(argv[6]));

		strcpy(hostname, getIPfromName(hostname));

		//printf("root_hostname after conversion: %s\n", hostname);

		char ibuf[100];
		unsigned char obuf[20];

		memcpy(ibuf, hostname, strlen(hostname));
 		SHA1((unsigned char*)ibuf, strlen(ibuf), obuf);

		new_node = initializeNode(NULL, NULL, (char*)obuf, own_port, hostname, ROOT, emptyMSG);

		assignROOT(getIPfromName(hostname), own_port);
		memset(&Caddr, '\0', sizeof Caddr); 	//Set 0 to Caddr for sizeof(struct addrinfo)  bits
		if((status = (getaddrinfo(hostname, own_port, &Caddr, &rlist))) != 0){		//getaddrinfo function initializes the Struct addrinfo with relevat data
			fprintf(stderr,  "getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
 		}

 		memset(&Caddr, '\0', sizeof Caddr); 	//Set 0 to Caddr for sizeof(struct addrinfo)  bits
		if((status = (getaddrinfo(hostname, CLIENT_PORT, &Caddr, &clist))) != 0){		//getaddrinfo function initializes the Struct addrinfo with relevat data
			fprintf(stderr,  "getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
 		}
 	//	printf("mode sent: %s\n", mode);
		rootFuncationCalls(clist, rlist, mode, root_hostname, root_port, hostname, own_port, new_node);
		freeNode(new_node);
	} else{
		fprintf(stderr, "Invalid mode type.  Exiting program\n");
		exit(EXIT_FAILURE);
	}
 }else if(strcmp(argv[1], "-p")==0){
 		sprintf(mode, "0");
		memcpy(root_port, argv[6], strlen(argv[6]));
		memcpy(root_hostname, argv[8], strlen(argv[8]));
		memcpy(own_port, argv[2], strlen(argv[2]));
		memcpy(hostname, argv[4], strlen(argv[4]));

		memset(&Caddr, '\0', sizeof Caddr); 	//Set 0 to Caddr for sizeof(struct addrinfo)  bits
		if((status = (getaddrinfo(hostname, own_port, &Caddr, &rlist))) != 0){		//getaddrinfo function initializes the Struct addrinfo with relevat data
			fprintf(stderr,  "bind getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
 		}

 		memset(&Caddr, '\0', sizeof Caddr); 	//Set 0 to Caddr for sizeof(struct addrinfo)  bits
 		if((status = (getaddrinfo(root_hostname, root_port, &Caddr, &brlist))) != 0){		//getaddrinfo function initializes the Struct addrinfo with relevat data
			fprintf(stderr,  "connect getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
 		}

 		assignROOT(root_hostname, root_port);
		peerFunctionCalls(rlist, brlist, mode, root_hostname, root_port, hostname, own_port, new_node);
		freeNode(new_node);
 }
}

void rootFuncationCalls(struct addrinfo *clientList, struct addrinfo *rlist, char* mode, char* root_hostname, char*root_port, char* hostname, char* own_port, struct peer_information *new_node){
	int socket_desc, client_socket_desc;
	int client_socket;


	 	//-----------------Call to Functions to Bind, Listen and Calaculate-----------------//
	 socket_desc = initializeConnections(rlist);  //Initialize the socket, connect on the Port
	 client_socket = initializeConnections(clientList);
	 while(socket_desc != 0){
	 	client_socket_desc = listenAccept(socket_desc,client_socket, own_port, new_node, ROOT);				//listen to the socket initialized and accept the incoming connections.
	 	if(client_socket_desc != -1){
	 		//new_node =
	 		new_node = dht_peer(client_socket_desc, mode, root_hostname, root_port, hostname, own_port, new_node, READ);
	 	//	printf("Something is happening after this point \n");
	 		close(client_socket_desc);
	 		//pthread_exit(NULL);
	 		//printf("Something is happening after this point\n");
	 	} else
	 		break;
	 }

	 printf("Exiting program\n");
	 close(socket_desc);
	 freeaddrinfo(rlist);			      //free the memory occupied by the Struct addrinfo
	 exit(EXIT_SUCCESS);
}

void peerFunctionCalls(struct addrinfo *rlist, struct addrinfo *brlist, char* mode, char* root_hostname, char*root_port, char* hostname, char* own_port, struct peer_information *new_node){
	int bind_socket_desc;
	int connect_socket_desc;
	int successor_socket_desc;

	struct peer_information* temp = create_node();

	 	//-----------------Call to Functions to Bind, Listen and Calaculate-----------------//

	 bind_socket_desc = initializeConnections(rlist);
	 goto join;

	 join:{
	 	printf("Attempting to establish connection with root...\n");
		 connect_socket_desc = initializePeerConnections(brlist);  //Initialize the socket, connect on the Port
		 //If there is no successor or predeccessor present for the peer, make a request to the root for
		 //joining the network.

		 //when root itself breaks
		 if(connect_socket_desc < 0){
		 	 printf("Root is dead !!!! \nExiting program...\n");
			 close(connect_socket_desc);
			 freeaddrinfo(rlist);			      //free the memory occupied by the Struct addrinfo
			 freeaddrinfo(brlist);			      //free the memory occupied by the Struct addrinfo
			 exit(EXIT_SUCCESS);
		 }

	 	if(new_node->successor == NULL && new_node->predecessor == NULL){
	 		new_node = dht_peer(connect_socket_desc, mode, root_hostname, root_port, hostname, own_port, new_node, REQUEST_TO_CONNECT);
	 		close(connect_socket_desc);
	 		//printf("new_node->predecessor %p && new_node->successor %p \n",new_node->predecessor, new_node->successor);
	 		if(new_node->successor == NULL && new_node->predecessor == NULL){
	 			 printf("Exiting program\n");
				 close(connect_socket_desc);
				 freeaddrinfo(rlist);			      //free the memory occupied by the Struct addrinfo
				 exit(EXIT_SUCCESS);
	 		}
	 	}
	 }

	 //If either the successor or predecessor are present the peer, wait in listen mode until a request
	 //comes through
	 while(bind_socket_desc != 0){
	 	connect_socket_desc = listenAccept(bind_socket_desc, 0, own_port, new_node, PEER);
	 	if(connect_socket_desc != -1){
	 		new_node = dht_peer(connect_socket_desc, mode, root_hostname, root_port, hostname, own_port, new_node, READ);
	 		close(connect_socket_desc);
	 		if(new_node->successor == NULL && new_node->predecessor == NULL){
	 			printf("I dont have a successor and predecessor\n");
	 			goto join;
	 		}
	 	} else{
	 		new_node->predecessor = NULL;
	 		//printf("Initialized predecessor to NULL as it is lost.\n");
	 		printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
	 		printf("|| My predecessor is dead ||\n");
	 		printf(";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");

	 		if(new_node->successor == NULL){
	 			printf("successor is lost too\n");
	 			goto join;
	 		}
	 		successor_socket_desc = makeConnection(new_node->successor->hostname, new_node->successor->Port);
	 		if(successor_socket_desc >= 0){
	 			printf("Made connection with successor to unlink\n");
	 			//specifically sending the current node to its successor
		 		sendData(successor_socket_desc, new_node, UNLINK);
		 		printf("Sent unlink request to successor %s\n",new_node->successor->hostname);

		 		temp = selectiveRead(successor_socket_desc, temp);
		 		printf("Received message from successor: %s\n",temp->flag);

		 		printf("\n");
        //temp_flag cases: either of:
        //1. SEVERED: When connection successfully severed with successor.
        //2. NO RESPONSE: When there is no response from the successor.
		 		if(strcmp(temp->flag, SEVERED) == 0){
		 			new_node->successor = NULL;
		 			printf("I have unliked with my successor\n");
		 			goto join;
		 		} else if(strcmp(temp->flag, NORESPONSE) == 0){
          new_node->successor = NULL;
          printf("Something is wrong, my predecessor and successor aren't responding. Contacting root.\n");
          goto join;
        }
	 		} else{
	 			new_node->successor = NULL;
	 			printf("successor is lost too\n");
	 			goto join;
	 		}
	 		//break;
	 	}
	 }
}
