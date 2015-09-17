#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "Protocol.h"
#include "StringManip.h"

#define BUFF_SIZE 1024

//METHOD TO CHECK IF WE RECEIVED A COMMAND.
int isCmd(char ch){
	switch(ch){
		case OPEN		:
		case SAVE		:
		case FILE_CONTENT	:
		case SIGNAL		:
		case START_MAP		:
		case START_REDUCE	:
		case NOTIFY		: return 1;
	}

	return 0;
}

int broadcastCmd(struct wrkrTrack workers, char signal, char * fname){
	int i, fd;
	char * pathless;
	if(fname != NULL)
		pathless = getFName(fname);
	for(i = 0; i < workers.workerCount; i++){
		fd = workers.worker_fd[i];
		write(fd, &signal, sizeof(char));
		if(fname != NULL){
			write(fd, pathless, strlen(pathless));
			write(fd, &signal, sizeof(char));
		}
	}
}

int cmd(int fd, char signal, char * fname){
	char * pathless;
	write(fd, &signal, sizeof(char));
	if(fname != NULL){
		pathless = getFName(fname);
		write(fd, pathless, strlen(pathless));
		write(fd, &signal, sizeof(char));
	}
}

int broadcast(int fd[MAX_WORKERS], int connectedCount, char * line){
	int i;
	for(i = 0; i < connectedCount; i++)
		write(fd, line, strlen(line));
}

int sendFile(struct wrkrTrack workers, char * fname, FILE * fp){
	char ch;
	char buff[BUFF_SIZE];	/*Buffer for sending the file*/
	int i, j, fd;
	memset(buff,'\0',BUFF_SIZE);

	//SIGNAL THE WORKERS TO ACCEPT THE FILE.
	broadcastCmd(workers, SAVE, fname);

	//SENDING THE CONTENTS: USE ESCAPING TO DISTINGUISH SIGNALS.
	i = 0;
	ch = '\0';
	while(fscanf(fp, "%c", &ch) != EOF){
		if(i >= BUFF_SIZE-1){
			i = 0;
			/*Share the buffer with all the workers*/
			for(j = 0; j < workers.workerCount; j++)
				write(workers.worker_fd[j], buff, BUFF_SIZE);
			memset(buff, '\0', BUFF_SIZE);
			continue;
		}

		if(ch == FILE_CONTENT)
			buff[i++] = ch;		/*Escaping: Send the character twice if it is a signal*/
		buff[i++] = ch;
	}
	for(j = 0; j < workers.workerCount; j++){
		fd = workers.worker_fd[j];
		write(fd, buff, i);		/*Flush out the remaining buffer*/
		ch = FILE_CONTENT;
		write(fd, &ch, sizeof(char));
		ch = '\0';
		write(fd, &ch, sizeof(char));
	}
}

void handleConnection(char * mode, int fd, int id, int user){
	FILE * fp;
	char received;
	char * fname;
	int status, len;

	*mode = SIGNAL;
	fname = (char *)malloc(MAX_PATH_LEN+1);

	/*Keep reading from the client and handle the commands sent*/
	while( (status = read(fd, &received, sizeof(received))) > 0 ){
		if(*mode == SIGNAL && isCmd(received)){
			*mode = received;
			continue;
		}
		else if(*mode == FILE_CONTENT){
			if(received == *mode){
				read(fd, &received, sizeof(received));
				if(received == *mode)
					fprintf(fp, "%c", received);
				else{
					fclose(fp);
					*mode = SIGNAL;
				}
			}
			else{
				fprintf(fp, "%c",received);
			}
		}
		else{
			if(received == *mode){
				processReceived(received, fname, mode, fd, id, user, &fp);
				memset(fname, '\0', MAX_PATH_LEN);
				/*Switch to SIGNAL mode only if we havent switched modes in processReceived*/
				if(*mode == received)
					*mode = SIGNAL;
			}
			else{
				len = strlen(fname);		
				fname[len] = received;
				fname[len+1] = '\0';
			}
		}
	}

}

//HANDLE SIGNALS SENT THROUGH THE CONNECTION.
int processReceived(char received, char * fname, char * mode, int fd, int id, int user, FILE ** fp){
	int master_fd[1];
	switch(*mode){
		case OPEN:		*fp = fopen(fname,"r");
					if(*fp == NULL){
						printf("ERROR: %s not found in the shared directories",fname);
						//exit(1);
					}//TODO
					//sendFile(fd,fname,fp);
					break;

		case SAVE:		fname = concat(2,CACHE,fname);
					*fp = fopen(fname,"a");
					*mode = FILE_CONTENT;
					break;

	}

	if(user == MASTER){
		switch(*mode){
/**			case QUIT:		close(fd);
						workers.worker_fd[id] = -1;
						workers.workerCount--;
						printf("Worker %d has quit\n",id+1);
						break;

			case FAIL:		printf("Worker %d has failed\n",id+1);
						reassign();
						printf("Map task reassigned to worker %d\n",*workers.idle);
						break;

			case SUCCESS:		printf("Worker %d has completed the map task\n",id+1);
						*workers.idle = id;
						workers.idle--;
						mode = SIGNAL;
						break;**/
			case NOTIFY:		printf("Worker %d: %s\n", id, fname);
						break;
		}
	}
	else if(user == WORKER){
		master_fd[0] = fd;
		switch(*mode){
			case START_MAP:		printf("Starting Map task for %s\n",fname);
						fname = concat(2,CACHE,fname);
						printf("%s\n",fname);
						run(fname, MAP, master_fd, 1);
						printf("Completed Map task\n");
						cmd(fd, NOTIFY, "Completed Map task");
						break;

			case START_REDUCE:	printf("Starting Reduce task for %s\n", fname);
						fname = concat(2,CACHE,fname);
						run(fname, REDUCE, master_fd, 1);
						printf("Completed Reduce task\n");
						cmd(fd, NOTIFY, "Completed the Reduce task");
						break;

			case KILL:		//TODO:
						break;
		}
	}

}
