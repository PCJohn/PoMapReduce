#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


main(){


	FILE * fp;
	int status, len;
	int sockfd, port, i;
	struct sockaddr_in servAddress;
	struct hostent *server;
	//char * serverName;		/*Name of the server*/
	char received;			/*Signals received*/
	char buff[1024];


	port = 8000;
	//MAKING THE SOCKET.
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		printf("ERROR: Couldn't open socket\n");
	server = gethostbyname("localhost");
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


	fp = fopen("Cache/temp.so","w");
	received = '\0';

	i = 0;
	while( (status = read(sockfd, buff, 1024 )) > 0 ){
		/**if(i >= 1024){
			write(fileno(fp), buff, sizeof(buff));
			i = 0;
			continue;
		}
		buff[i++] = received;**/
		fwrite(buff, 1, 1024, fp);
	}

	fclose(fp);

}
