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

#define PORT "27993"
#define WAIT_CONN 0
#define BUFFSIZE 500

#include "clientdataCommunication.c"

//function declarations
int initializeConnections(struct addrinfo * );


int main(int argc, char *argv[])
{

//----------------------Variable Declarations------------------------//
 struct addrinfo Caddr, clientAddr;
 struct addrinfo *rlist;
 struct sockaddr_storage client_addr;
 socklen_t client_addr_len;
 int status, socket_desc; 
 char mode[50];



//-------------------------Initializing the Struct addrinfo--------------------------//

 memset(&Caddr, '\0', sizeof Caddr); 	//Set 0 to Caddr for sizeof(struct addrinfo)  bits
 Caddr.ai_family = AF_INET;		//For IPv4 or IPv6
 Caddr.ai_socktype = SOCK_STREAM;	//TCP Stream
 Caddr.ai_flags = AI_PASSIVE;		// Self-IP
//Caddr.ai_flags = AI_NUMERICHOST;     // For numerical network address.
 Caddr.ai_flags = AI_CANONNAME;		//For looking up the canonical name of the DNS Server.
 Caddr.ai_next = NULL; 			//Because it is a single client communicating to a server
 Caddr.ai_addr = NULL; 			//-do-


//---------------------------Handle Command line arguments---------------------------//

 if(argc <= 2)
 {
  fprintf(stderr, "Usage: client -m <mode> <-p port> [hostname] \n");
  exit(EXIT_FAILURE);
 }
 else{	
	if(argc == 4){
		if(strcmp(argv[3], "cs5700f17.ccs.neu.edu")==0){
			 if(status = (getaddrinfo(argv[3], argv[2], &Caddr, &rlist)) != 0){		//getaddrinfo function initializes the Struct addrinfo with relevat data
				fprintf(stderr,  "getaddrinfo error: %s\n", strerror(status));
				exit(EXIT_FAILURE);
			 }	
                        strcpy(mode, argv[4]);
		}
	}else{
		if(strcmp(argv[1], "cs5700f17.ccs.neu.edu")==0){
			 if(status = (getaddrinfo(argv[1], PORT, &Caddr, &rlist)) != 0){		//getaddrinfo function initializes the Struct addrinfo with relevat data
				fprintf(stderr,  "getaddrinfo error: %s\n", strerror(status));
				exit(EXIT_FAILURE);
			 }
			strcpy(mode, argv[2]);
		}
	 }
     } 
 
//-----------------Call to Functions to Bind, Listen and Calaculate-----------------//

 socket_desc = initializeConnections(rlist);  //Initialize the socket, connect on the Port
 clientdataCommunication(socket_desc, mode);	      //Receive Data from the server and send data to the server.	
 freeaddrinfo(rlist);			      //free the memory occupied by the Struct addrinfo
 exit(EXIT_SUCCESS);
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
  		if((socket_desc = socket(p -> ai_family, p-> ai_socktype, p -> ai_protocol))== -1){
			fprintf(stderr, "Call to the function Socket() failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
  		}
		
		//manipulate the socket to release the "PORT"
		if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &optName_value, sizeof(int)) == -1) {
            		perror("setsockopt");
            		exit(EXIT_FAILURE);
        	}

		//place a request for connection
		 if((connect(socket_desc, p->ai_addr, p->ai_addrlen))==-1){
			fprintf(stderr, "Call to the function Connect() failed: %s\n", strerror(errno));
			//freeaddrinfo(p);//memory map error if there is no sever connected.
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
