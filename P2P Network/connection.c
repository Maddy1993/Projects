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
#include <pthread.h>
#include <signal.h>

#include "connection.h"
#include "structure.h"
#include "transfer.h"
#include "check.h"
#include "node.h"
#include "rootClient.h"

const int CONNECTION_ERROR = -11;

pthread_mutex_t mutexVar;

int makeConnection(char* hostname, char* port){

struct addrinfo Caddr;
struct addrinfo *rlist;
int socket_desc_connect;
int status;

//-------------------------Initializing the Struct addrinfo--------------------------//

 Caddr.ai_family = AF_INET;		//For IPv4 or IPv6
 Caddr.ai_socktype = SOCK_STREAM;	//UDP	 Stream
// Caddr.ai_flags = 0;
 Caddr.ai_flags = AI_NUMERICHOST;		//For looking up the IP address
 Caddr.ai_protocol = 0;
//---------------------------Handle Command line arguments---------------------------//

//Set 0 to Caddr for sizeof(struct addrinfo)  bits
memset(&Caddr, '\0', sizeof Caddr);

	//getaddrinfo function initializes the Struct addrinfo with relevat data
	if((status = (getaddrinfo(hostname, port, &Caddr, &rlist))) != 0){
			printf("status is: %d\n",status);
			fprintf(stderr,  "connect getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
	}

	socket_desc_connect = initializePeerConnections(rlist);
	freeaddrinfo(rlist);
	return socket_desc_connect;
}

struct peer_information* selectiveRead(int sockID, struct peer_information* node){
	struct timeval timeoutVal;
	fd_set readSet;
	int result, retry=0;

	//struct peer_information* temp = create_node();

	while(retry < 3){

	   // Initialize time out struct
	   timeoutVal.tv_sec = 20;
	   timeoutVal.tv_usec = 0;

	   // Initialize the set
	   FD_ZERO(&readSet);
	   FD_SET(sockID, &readSet);

	   // select()
	   result = select(sockID+1, &readSet, NULL, NULL, &timeoutVal);
	   //printf("retry value: %d\n",retry);
	   ++retry;
	   if(result == 0){
	   		printf("Nothing happened. Retry attempt: %d\n", retry);
	   } else if(retry < 0){
	   		fprintf(stderr, "Select() error: %s\n", strerror(errno));
	   } else if(retry > 0){
	   		if (FD_ISSET(sockID, &readSet)) {
    		    node = readData(sockID, node);
    		  //  printf("node->hostname: %s\n", node->hostname);
    		    break;
    		}
	   }
	}

	//when there is no Message from successor
	if(retry >= 3){
		memset(node->flag, 0, MSGLEN);
		memcpy(node->flag, NORESPONSE, strlen(NORESPONSE));
	}

   return node;
}

int initializeConnections(struct addrinfo *res){
	int optName_value= 1;
	struct addrinfo *p;
	int socket_desc;

	//loop through the results list for addresses
	for(p = res ; p != NULL ; p = p -> ai_next){

		//make a socket
  		if((socket_desc = socket(p -> ai_family, p-> ai_socktype, p -> ai_protocol))==-1){
			fprintf(stderr, "Call to the function Socket() failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
  		}

		//manipulate the socket to release the "PORT"
		if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &optName_value, sizeof(int)) == -1) {
            		perror("setsockopt");
            		exit(EXIT_FAILURE);
        	}

		//bind the socket
	 	if((bind(socket_desc, p->ai_addr, p->ai_addrlen))==-1){
			fprintf(stderr, "Call to the function Bind() failed: %s\n", strerror(errno));
			//freeaddrinfo(p); //memory map error
			close(socket_desc);
			exit(EXIT_FAILURE);
		}

    	break;
 	}


	//check for address list
 	if(p == NULL){
		fprintf(stderr, "Connection failed\n");
		exit(EXIT_FAILURE);
 	}


  return socket_desc;
 }

int initializePeerConnections(struct addrinfo *bres){
	int optName_value= 1;
	struct addrinfo *p;
	int socket_desc_connect;

 	//loop through the results list for addresses
	for(p = bres ; p != NULL ; p = p -> ai_next){

		//make a socket
  		if((socket_desc_connect = socket(p -> ai_family, p-> ai_socktype, p -> ai_protocol))==-1){
			fprintf(stderr, "Call to the function Socket() failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
  		}

		//manipulate the socket to release the "PORT"
		if (setsockopt(socket_desc_connect, SOL_SOCKET, SO_REUSEADDR, &optName_value, sizeof(int)) == -1) {
    		perror("setsockopt");
    		exit(EXIT_FAILURE);
        }

        //place a request for connection
		 if((connect(socket_desc_connect, p->ai_addr, p->ai_addrlen))==-1){
			//fprintf(stderr, "Call to the function Connect() failed: %s\n", strerror(errno));
			//freeaddrinfo(p);//memory map error if there is no sever connected.
			close(socket_desc_connect);
			//exit(EXIT_FAILURE);
			return CONNECTION_ERROR;
 		}

    	break;
 	}

	//check for address list
 	if(p == NULL){
		fprintf(stderr, "Connection failed\n");
		exit(EXIT_FAILURE);
 	}

  return socket_desc_connect;
 }

 int listenAccept(int sockID, int clientSockID, char* port, struct peer_information* node, char* mode){
	struct sockaddr_storage client_addr;
	struct addrinfo  clientAddr;
	socklen_t client_addr_len;
	int client_socket_desc;
	const int WAIT_CONN = 0;

	struct timeval timeoutVal;
	int result;
	fd_set readSet;
	int retry=0;
	int returnCode;

	pthread_t thread, clientThread;
 	//pthread_attr_t attr;

	struct arguments *point = malloc(sizeof(struct arguments));

	point->node = node;
	memset(point->mode, 0, ARGMODELEN);
	memcpy(point->mode, mode, strlen(mode));
	point->clientID = 0;
	//printf("mode copied is: %s\n",point->mode);

	pthread_mutex_init(&mutexVar, NULL);
	/*pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
*/
	//listen to the socket
 	if((listen(sockID,WAIT_CONN))==-1){
		fprintf(stderr, "Call to the function Listen() failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
 	}

 	printf("Socket Open, listening on Port: %s\n", port);

 	if(strcmp(mode, ROOT) == 0){
 		//if(!clientThread){
 			struct arguments *point1 = malloc(sizeof(struct arguments));
 			point1->node = node;
 			memset(point1->mode, 0, ARGMODELEN);
 			point1->clientID = clientSockID;

	 		//create a thread for handle listen connections. A detached thread
			returnCode = pthread_create(&clientThread, NULL, clientRunner, point1);
			   		//printCycle(node, mode);
			if (returnCode){
				printf("ERROR; return code from pthread_create() is %d\n", returnCode);
			}

			//printf("returnCode: %d\n",returnCode);

			//pthread_attr_destroy(&attr);
 		//}
 	}

	//start select to handle peers
	while(retry < 5){

	   // Initialize time out struct
	   timeoutVal.tv_sec = 25;
	   timeoutVal.tv_usec = 0;

	   // Initialize the set
	   FD_ZERO(&readSet);
	   FD_SET(sockID, &readSet);
	   //FD_SET(clientSockID, &readSet);

	   // select()
	   result = select(sockID+1, &readSet, NULL, NULL, &timeoutVal);
	   //printf("retry value: %d\n",retry);
	   ++retry;
	   if(result == 0){

	   		if(strcmp(mode, ROOT) == 0){
	   			//if(!thread){
	   					//returnCode = pthread_create(&thread, &attr, printCycle, point);
	   					returnCode = pthread_create(&thread, NULL, printCycle, point);
				   		//printCycle(node, mode);
				   		if (returnCode){
					      printf("ERROR; return code from pthread_create() is %d\n", returnCode);
					      continue;
			   			}

			   			//pthread_attr_destroy(&attr);
			   		//} else{
			   			/*returnCode = pthread_join(thread, NULL);

					   if (returnCode) {
					      printf("ERROR; return code from pthread_join() is %d\n", returnCode);
					      continue;
					  	}*/
			   		//}
	   			}

   		if(retry < 5){
   			printf("No incoming connections until now. Continuing to listen\n");
   		} else if(retry == 5){
   			printf("I have been waiting too long...\n");
   		}


	   } else if(retry < 0){
	   		fprintf(stderr, "Select() error: %s\n", strerror(errno));
	   } else if(retry > 0){
	   		if (FD_ISSET(sockID, &readSet)) {
    		    //accept the incoming connection
			 	//make it infinite while loop for multiple connections
				client_addr_len = sizeof client_addr;
				if((client_socket_desc = accept(sockID, (struct sockaddr *) &clientAddr, &client_addr_len)) == -1){
					fprintf(stderr, "Call to the function Accept() failed: %s\n", strerror(errno));
					exit(EXIT_FAILURE);
			 	}
			 	goto return_point;
    		}
	   }
	   if(retry == 2 && strcmp(mode, PEER) == 0)
	   {
	   	client_socket_desc = -1;
	   	goto return_point;
	   }
	}


	pthread_mutex_destroy(&mutexVar);
	//free(point);
	client_socket_desc = -1;
	goto return_point;

	return_point: return client_socket_desc;
 }
