#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


main(int argc, char * argv[]){
	//VARIABLES.
	int port;
	int i, currentSocket, fd;
	char buff[1024], ch;
	FILE * fp;
	struct sockaddr_in servAddress;
	struct sockaddr_in clientAddress;
	struct sockaddr_in * pV4Addr;
	socklen_t clilen;
	
	//ACCEPTING THE PORT.
	if(argc == 1){
		printf("Enter the port number: ");
		scanf("%d",&port);
	}
	else if(argc == 2){
		port = atoi(argv[1]);
	}

	//CREATING THE PORT.
	printf("\nStarting the server...\n");

	currentSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(currentSocket > 0)
		printf("Master started (Port %d)\n", port);
	
	//MAKING THE SERVER ADDRESS.
	bzero((char *)&servAddress, sizeof(servAddress));

	servAddress.sin_family = AF_INET;
	servAddress.sin_addr.s_addr = INADDR_ANY;
	servAddress.sin_port = htons(port);

	//BINDING.
	if(bind(currentSocket, (struct sockaddr *) &servAddress, sizeof(servAddress)) < 0){
		printf("ERROR: Couldn't bind socket\n");
	}

	//LISTENING FOR CONNECTIONS.
	printf("\nWaiting for connections...\n");
	listen(currentSocket,5);

		fp = fopen("temp.so","r");
		//ACCEPTING THE CONNECTIONS.
		clilen = sizeof(clientAddress);
		fd = accept(currentSocket, (struct sockaddr*)&clientAddress, &clilen);
		if(fd > 0){
			printf("Connection 1\n");
			ch = '\0';
		i = -1;
		while(fread(buff, 1, 1024, fp) != 0){
			/**if(i >= 1024-1){
				write(fd, buff, 1024);
				memset(buff, '\0', 1024);
				i = -1;
				continue;
			}
			i++;
			buff[i] = ch;**/
			write(fd, buff, 1024);
		}
		write(fd, buff, i);
		}//if
		

	//CLOSE THE STREAMS AND SOCKETS.
	close(currentSocket);
	
}
