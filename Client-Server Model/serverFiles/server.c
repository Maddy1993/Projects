#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<netdb.h>
#include <unistd.h>
#include<netinet/in.h>
#include<time.h>

#define PORT "27993"
#define WAIT_CONN 0

#include "serverdataCommunication.c"


//function declaration
int initializeConnections(struct addrinfo * );
int listenAccept(int);
int serverdataCommunication(int, char*, char*);
char* tokenizer(char*, int);

 int listenAccept(int sockID){
	struct sockaddr_storage client_addr;
	struct addrinfo  clientAddr;
	socklen_t client_addr_len;
	int client_socket_desc;
	
	//listen to the socket
 	if((listen(sockID,WAIT_CONN))==-1){
		fprintf(stderr, "Call to the function Listen() failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
 	}
	
	printf("Socket Open, listening on Port: %s\n", PORT);

	
	//accept the incoming connection
 	//make it infinite while loop for multiple connections
	client_addr_len = sizeof client_addr;
	if((client_socket_desc = accept(sockID, (struct sockaddr *) &clientAddr, &client_addr_len)) == -1){
		fprintf(stderr, "Call to the function Accept() failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
 	}
	return client_socket_desc;
 }

 int initializeConnections(struct addrinfo *res){
	int optName_value= 1;
	struct addrinfo *p; 
	int socket_desc;
	char ipstr[INET_ADDRSTRLEN];
	void *ipaddrs;
	p=res;

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
		fprintf(stderr, "Binding failed\n");
		exit(EXIT_FAILURE);
 	}
 return socket_desc;	//return the socket filedescriptor
}	


int main(int argc, char *argv[])
{
 struct addrinfo addr;
 struct addrinfo *rlist; 
 int status;
 int socket_desc, client_socket_desc;
 char NUID[50], secret_flag[100];
 void *ipaddrs;
	
 
 memset(&addr, 0, sizeof (struct addrinfo));	//Set 0 to Caddr for sizeof(struct addrinfo)  bits
 addr.ai_family = AF_UNSPEC;			//For IPv4 or IPv6
 addr.ai_socktype = SOCK_STREAM;		//TCP Stream
// addr.ai_flags = AI_PASSIVE;			// Self-IP
 addr.ai_flags = AI_NUMERICHOST;
 addr.ai_next = NULL; 				//Because it is a single client communicating to a server
 addr.ai_addr = NULL; 				//-do-

memset(NUID, 0, sizeof NUID);
memset(secret_flag, 0, sizeof secret_flag);

if(argc < 4)
 {
  fprintf(stderr, "./server <-p port> [hostname] [NEU ID] [FLAG]");
  exit(EXIT_FAILURE);
 }else{
	if(strcmp(argv[1], "27993") == 0){
		//getaddrinfo function initializes the Struct addrinfo with relevat data
 		if(status = (getaddrinfo(argv[2], argv[1], &addr, &rlist)) != 0){
			fprintf(stderr,  "getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
 		}
		//printf("%s %s %s\n", argv[1], argv[3], argv[4]);
		strcpy(NUID, argv[3]);
	        strcpy(secret_flag, argv[4]);
		//printf("Assigned vvalues\n");
	}else{
		//getaddrinfo function initializes the Struct addrinfo with relevat data
 		if(status = (getaddrinfo(argv[1], PORT, &addr, &rlist)) != 0){
			fprintf(stderr,  "getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
 		}
		strcpy(NUID, argv[2]);
	        strcpy(secret_flag, argv[3]);
	}
   }
 
 

socket_desc = initializeConnections(rlist);				//initialize the server socket to receive client requests.	

client_socket_desc = listenAccept(socket_desc);				//listen to the socket initialized and accept the incoming connections.

serverdataCommunication(client_socket_desc, NUID, secret_flag);		//pass the client ID, NUID, and secret_flag received from the client 
									//to compute calculations and return the secret_flag is everything is correct.

close(socket_desc);							//close the socket

freeaddrinfo(rlist);							//free the memory assigned by getaddrinfo
exit(EXIT_SUCCESS);
}
