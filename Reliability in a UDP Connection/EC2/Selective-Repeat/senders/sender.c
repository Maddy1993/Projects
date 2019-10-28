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

#define PORT "15095"
#define WAIT_CONN 0
#define BUFFSIZE 500
#define PACKET_SIZE 512
#define MAX_FILE_SIZE 255
#define HOSTNAME_LEN 255
#define PORT_LEN 10
#define MODE_LEN 50

#include "senderCommunication.c"

//function declarations
int initializeConnections(struct addrinfo * );
void processArguments(char**, int);

//global variables
char mode[MODE_LEN], filename[MAX_FILE_SIZE], address[HOSTNAME_LEN], port[PORT_LEN];

int main(int argc, char *argv[])
{

//----------------------Variable Declarations------------------------//
 struct addrinfo Caddr;
 struct addrinfo *rlist;
 int status, socket_desc;

//-------------------------Initializing the Struct addrinfo--------------------------//

 memset(&Caddr, '\0', sizeof Caddr); 	//Set 0 to Caddr for sizeof(struct addrinfo)  bits
 Caddr.ai_family = AF_INET;		         //For IPv4 or IPv6
 Caddr.ai_socktype = SOCK_DGRAM;	     //UDP	 Stream
 Caddr.ai_flags = AI_PASSIVE;		      // Self-IP
 Caddr.ai_protocol = 0;
//---------------------------Handle Command line arguments---------------------------//

memset(filename, 0,sizeof(char)*MAX_FILE_SIZE);
memset(mode, 0, sizeof(char)*MODE_LEN);
memset(address, 0, sizeof(char)*HOSTNAME_LEN);
memset(port, 0, sizeof(char)*PORT_LEN);

 if(argc < 5){
  fprintf(stderr, "Usage: ./sender <-m mode> <-p port> <-h hostname> <-f filename> \n");
  exit(EXIT_FAILURE);
 }
 else{

    //flexible command line arguments
    processArguments(argv, argc);

    //getaddrinfo function initializes the Struct addrinfo with relevat data
    //as it is a UDP connection, the sender should bind to some port on the host it is running
    //for consistency, we will be binding to a port = 15095
     if(status = (getaddrinfo(NULL, PORT, &Caddr, &rlist)) != 0){
      fprintf(stderr,  "getaddrinfo error: %s\n", strerror(status));
      exit(EXIT_FAILURE);
    }
 }

//-----------------Call to Functions to Bind, Listen and Calaculate-----------------//

 socket_desc = initializeConnections(rlist);  //Initialize the socket, connect on the Port
 senderCommunication(socket_desc, mode, filename, address, port);	      //Receive Data from the server and send data to the server.
 freeaddrinfo(rlist);			      //free the memory occupied by the Struct addrinfo
 exit(EXIT_SUCCESS);
}

//this function takes two arguments, character of arguments and length of arguments
//processes the argument line and assigns the port, hostname and mode to the
//respective variables.
//If any garbage value is found, the program terminates
void processArguments(char* argv[], int argc){
  int i;

  for(i = 0; i< argc; i++){
    //if the string is object file name, ignore it
    if(strcmp(argv[i], "./sender") == 0){
      continue;
    } else if(strcmp(argv[i], "-m") == 0){
      //copy the next argument to the mode variable
      memcpy(mode, argv[i+1], strlen(argv[i]));
    } else if(strcmp(argv[i], "-p") == 0){
      //copy the next argument to the port variable
      memcpy(port, argv[i+1], strlen(argv[i+1]));
    } else if(strcmp(argv[i], "-h") == 0){
      //copy the next argument to the hostname variable
      memcpy(address, argv[i+1], strlen(argv[i+1]));
    } else if(strcmp(argv[i], "-f") == 0){
      //copy the next argument to the hostname variable
      memcpy(filename, argv[i+1], strlen(argv[i+1]));
    }
  }

  //check if the assignment partially succeded
  if(strlen(mode) == 0){
    printf("Error in assigning the mode. Please retry again\n");
    exit(EXIT_FAILURE);
  } else if(strlen(port) == 0){
    printf("Error in assigning the port. Please retry again\n");
    exit(EXIT_FAILURE);
  } else if(strlen(address) == 0){
    printf("Error in assigning the hostname. Please retry again\n");
    exit(EXIT_FAILURE);
  } else if(strlen(filename) == 0 || strlen(filename) > 255){
    printf("Error in assigning the filename. Please retry again with a different name\n");
    exit(EXIT_FAILURE);
  }
}

//this functions takes struct aaddrinfo as an argument, and creates a socket and binds it to the address specified
//in the sock_addr
 int initializeConnections(struct addrinfo *res){
	int optName_value= 1;
	struct addrinfo *p;
	int socket_desc;

	//loop through the results list for addresses
	for(p = res ; p != NULL ; p = p -> ai_next){

		//make a socket
		if((socket_desc = socket(p -> ai_family, p-> ai_socktype, p -> ai_protocol))== -1){
			fprintf(stderr, "Call to the function Socket(): %s\n", strerror(errno));
			continue;
		}

		//manipulate the socket to release the "PORT"
		if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &optName_value, sizeof(int)) == -1) {
  		perror("setsockopt");
  		exit(EXIT_FAILURE);
  	}

		if((bind(socket_desc, p->ai_addr, p->ai_addrlen))== 0){
			break;
 		} else{
 			fprintf(stderr, "Bind: %s\n", strerror(errno));
 			continue;
 		}

	close(socket_desc);
	}

	//check for address list
 	if(p == NULL){
		fprintf(stderr, "Connection failed\n");
		exit(EXIT_FAILURE);
 	}

  return socket_desc;
 }
