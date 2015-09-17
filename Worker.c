#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dlfcn.h>
#include <signal.h>
#include "Protocol.h"
#include "StringManip.h"


void error(const char *);

char mode;		/*Mode to process the signal*/
int pid;		/*Process id for the current map or reduce task*/

void error(const char *msg){
	perror(msg);
	exit(0);
}

int main(int argc, char * argv[]){

	//VARIABLES.
	int sockfd, port, status, i, len;
	struct sockaddr_in servAddress;
	struct hostent *server;
	char * serverName;		/*Name of the server*/
	char received;			/*Signals received*/
	extern char mode;		/*Mode to process signals*/

	serverName = (char *)(MAX_PATH_LEN+1);
	/*PHASE 1: Receive the map file from the master*/
	printf("\n\nWelcome to Po MapReduce [Version 1.1]\n");
	printf("Babi Carter productions\n\n");
	printf("Logged in as a worker\n");
	//GETTING THE SERVER AND PORT TO CONNECT.
	if(argc == 1){
		printf("Enter the server to connect to: ");
		scanf("%s",serverName);
		printf("Enter the port number to connect: ");
		scanf("%d",&port);
	}
	else if(argc == 2){
		serverName = argv[1];
		printf("Enter the port number to connect: ");
		scanf("%d",&port);
	}
	else if(argc == 3){
		serverName = argv[1];
		port = atoi(argv[2]);
	}

	//MAKING THE SOCKET.
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		error("ERROR: Couldn't open socket\n");
	server = gethostbyname(serverName);
	if(server == NULL){
		printf("ERROR: No such host\n");
		exit(0);
	}

	bzero((char *)&servAddress, sizeof(servAddress));
	servAddress.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		(char *)&servAddress.sin_addr.s_addr,
			server->h_length);
	servAddress.sin_port = htons(port);
	if(connect(sockfd, (struct sockaddr *)&servAddress, sizeof(servAddress)) < 0)
		error("ERROR: Couldn't connect\n");

	//KEEP READING FROM THE SOCKET STORE IN THE MAP TARGET.
	mode = SIGNAL; 				/*Only wait for signals initially*/
	printf("Connected to the master(%s). Waiting for tasks.\n", serverName);
	handleConnection(sockfd, -1, WORKER);

	//CLOSING THE STREAMS.
	close(sockfd);

	return 0;
}

