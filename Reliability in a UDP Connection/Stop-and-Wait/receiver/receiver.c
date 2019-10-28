#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<netdb.h>
#include<unistd.h>
#include<netinet/in.h>
#include<time.h>

#define PORT "15095"
#define WAIT_CONN 0
#define MAX_FILE_SIZE 255

#include "receiverCommunication.c"


//function declaration
int initializeConnections(struct addrinfo * );
int receiverCommunication(int, char*);
char* tokenizer(char*, int);

int main(int argc, char *argv[]){

 struct addrinfo addr;
 struct addrinfo *rlist; 
 int status;
 int socket_desc, client_socket_desc;
 char mode[50];
 void *ipaddrs;
	
 
 memset(&addr, 0, sizeof (struct addrinfo));	//Set 0 to Caddr for sizeof(struct addrinfo)  bits
 addr.ai_family = AF_UNSPEC;					//For IPv4 or IPv6
 addr.ai_socktype = SOCK_DGRAM;					//TCP Stream
 addr.ai_flags = AI_NUMERICHOST;
 addr.ai_protocol = 0;
 addr.ai_next = NULL; 							//Because it is a single client communicating to a server
 addr.ai_addr = NULL; 							//-do-

memset(mode, 0, sizeof mode);

if(argc < 3)
 {
  fprintf(stderr, "./server -m <mode> <-p port> [hostname] ");
  exit(EXIT_FAILURE);
 }else{
	if(argc == 4){
		//getaddrinfo function initializes the Struct addrinfo with relevat data
 		if(status = (getaddrinfo(argv[3], argv[2], &addr, &rlist)) != 0){
			fprintf(stderr,  "getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
 		}
		strcpy(mode, argv[1]);
	}else{
		//getaddrinfo function initializes the Struct addrinfo with relevat data
 		if(status = (getaddrinfo(argv[2], PORT, &addr, &rlist)) != 0){
			fprintf(stderr,  "getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
 		}
		strcpy(mode, argv[1]);
	}
   }
 

socket_desc = initializeConnections(rlist);				//initialize the server socket to receive client requests.	

receiverCommunication(socket_desc, mode);				//pass the client ID and mode received from the sender 
														//to compute calculations and return the secret_flag is everything is correct.

close(socket_desc);										//close the socket

freeaddrinfo(rlist);									//free the memory assigned by getaddrinfo
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
  		if((socket_desc = socket(p -> ai_family, p-> ai_socktype, p -> ai_protocol))==-1){
			fprintf(stderr, "Call to the function Socket() failed: %s\n", strerror(errno));
			continue;
  		}
	
		//manipulate the socket to release the "PORT"
		if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &optName_value, sizeof(int)) == -1) {
            		perror("setsockopt");
            		exit(EXIT_FAILURE);
        	}

 		//bind the socket
	 	if((bind(socket_desc, p->ai_addr, p->ai_addrlen))== 0){
			fprintf(stderr, "Bind: %s\n", strerror(errno));
			break;
 		}

		close(socket_desc);
 	}

 	//check for address list
	if(p == NULL){
		fprintf(stderr, "Binding failed\n");
		exit(EXIT_FAILURE);
 	}
 return socket_desc;	//return the socket filedescriptor
}	

