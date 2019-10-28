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

#include "receiverCommunication.h"

#define PORT "15095"
#define WAIT_CONN 0
#define HOSTNAME_LEN 255
#define PORT_LEN 10
#define MODE_LEN 50


//function declaration
int initializeConnections(struct addrinfo * );
int receiverCommunication(int, char*);
void processArguments(char**, int);

//global variables
char hostname[HOSTNAME_LEN];
char port[PORT_LEN];
char mode[MODE_LEN];

int main(int argc, char *argv[]){

 //Variable declarations
 struct addrinfo addr;
 struct addrinfo *rlist;
 int status;
 int socket_desc, client_socket_desc;
 void *ipaddrs;

 //initializations
 memset(&addr, 0, sizeof (struct addrinfo));
 addr.ai_family = AF_UNSPEC;					        //For IPv4 or IPv6
 addr.ai_socktype = SOCK_DGRAM;					      //UDP Stream
 addr.ai_flags = AI_PASSIVE;
 addr.ai_canonname = NULL;
 addr.ai_protocol = 0;
 addr.ai_next = NULL;
 addr.ai_addr = NULL;

 memset(mode, 0, MODE_LEN);
 memset(hostname, 0, HOSTNAME_LEN);
 memset(port, 0, PORT_LEN);

if(argc < 5)
 {
  fprintf(stderr, "./server -m <mode> -p <port> -h <hostname> \n");
  exit(EXIT_FAILURE);
 }else{
    processArguments(argv, argc);
		//getaddrinfo function initializes the Struct addrinfo with relevat data
 		if(status = (getaddrinfo(NULL, port, &addr, &rlist)) != 0){
			fprintf(stderr,  "getaddrinfo error: %s\n", strerror(status));
			exit(EXIT_FAILURE);
 		}
 }

socket_desc = initializeConnections(rlist);				 //initialize the server socket to receive client requests.
receiverCommunication(socket_desc, mode);		       //pass the client ID and mode received from the sender
														                      //to compute calculations and return the secret_flag is everything is correct.
}

//this function takes two arguments, character of arguments and length of arguments
//processes the argument line and assigns the port, hostname and mode to the
//respective variables.
//If any garbage value is found, the program terminates
void processArguments(char* argv[], int argc){
  int i;

  memcpy(port, PORT, strlen(PORT));
  for(i = 0; i< argc; i++){
    //if the string is object file name, ignore it
    if(strcmp(argv[i], "./receiver") == 0){
      continue;
    } else if(strcmp(argv[i], "-m") == 0){
      //copy the next argument to the mode variable
      memcpy(mode, argv[i+1], strlen(argv[i]));
    } else if(strcmp(argv[i], "-p") == 0){
      //copy the next argument to the port variable
      memcpy(port, argv[i+1], strlen(argv[i+1]));
    } else if(strcmp(argv[i], "-h") == 0){
      //copy the next argument to the hostname variable
      memcpy(hostname, argv[i+1], strlen(argv[i+1]));
    }
  }

  //check if the assignment partially succeded
  if(strlen(mode) == 0){
    printf("Error in assigning the mode. Please retry again\n");
    exit(EXIT_FAILURE);
  } else if(strlen(port) == 0){
    printf("Error in assigning the port. Please retry again\n");
    exit(EXIT_FAILURE);
  } else if(strlen(hostname) == 0){
    printf("Error in assigning the hostname. Please retry again\n");
    exit(EXIT_FAILURE);
  }
}

//this functions takes struct aaddrinfo as an argument, and creates a socket and binds it to the address specified
//in the sock_addr
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
	 	if((bind(socket_desc, p->ai_addr, p->ai_addrlen)) == 0){
			break;
 		} else{
      fprintf(stderr, "Bind: %s\n", strerror(errno));
    }

		close(socket_desc);
 	}

 	//check for address list
	if(p == NULL){
		fprintf(stderr, "Binding failed\n");
		exit(EXIT_FAILURE);
 	}

  freeaddrinfo(res);
	//return the socket filedescriptor
  return socket_desc;
}
