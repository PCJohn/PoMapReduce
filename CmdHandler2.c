#include <stdio.h>
#include <stdlib.h>
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

//METHID TO SEND A FILE.
int sendFile(struct wrkrTrack workers, char * fname, FILE * fp){
	char * buff;	/*Buffer for sending the file*/
	int i, fd;
	long size;

	//SIGNAL THE WORKERS TO ACCEPT THE FILE.
	fseek(fp, 0L, SEEK_END);	/*Go to the end of the file*/
	size = ftell(fp);		/*Getting the size of the file*/
	rewind(fp);			/*Go back to the start of the file*/
	buff = (char *)malloc(size);
	addSize(fname, size);		/*Add the file size to the sent file name*/
	broadcastCmd(workers, SAVE, fname);

	//SENDING THE CONTENTS.
	while(fread(buff, 1, size, fp) != EOF){
		for(i = 0; i < workers.workerCount; i++)
			write(workers.worker_fd[i], buff, size);
	}

}

//METHOD TO RECEIVED A FILE.
int receiveFile(int fd, char * fname){
	FILE * fp;
	long size;
	char * buff;

	size = getSize(fname);
	buff = (char *)malloc(size);
	fp = fopen(fname, "a");
	if(read(fd, buff, size) > 0)
		fwrite(buff, 1, size, fp);

}

//METHOD TO HANDLE ONE CONNECTION.
void handleConnection(int fd, int id, int user){
	char received, mode;
	char * fname;
	int flag, n;
	FILE * fp;

	/*Keep reading from the client and handle the commands sent*/
	flag = 1;
	n = 0;
	fname = (char *)malloc(MAX_PATH_LEN+1);
	memset(fname, '\0', MAX_PATH_LEN);
	while(read(fd, &received, sizeof(char)) > 0){
		if(isCmd(received)){
			if(flag == -1){
				if(mode != FILE_CONTENT){
					processReceived(mode, fname, fd, id, user);
					memset(fname, '\0', MAX_PATH_LEN);
					n = 0;
					flag = 1;
				}
				else
					fclose(fp);
				mode = SIGNAL;
			}
			else{
				mode = received;
				if(mode == FILE_CONTENT)
					fp = fopen(fname, "a");
				flag = -1;
			}
		}
		else if(flag == -1)
			fname[n++] = received;
		else if(mode == FILE_CONTENT)
			fprintf(fp, "%c", received);
	}
}

//HANDLE SIGNALS SENT THROUGH THE CONNECTION.
int processReceived(char mode, char * fname, int fd, int id, int user){
	FILE * fp;
	int master_fd[1];
	switch(mode){
		case OPEN:		fp = fopen(fname,"r");
					if(fp == NULL){
						printf("ERROR: %s not found in the shared directories",fname);
						//exit(1);
					}//TODO
					//sendFile(fd,fname,fp);
					break;

		case SAVE:		receiveFile(fd, fname);
					break;
	
	}

	if(user == MASTER){
		switch(mode){
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
		switch(mode){
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

//METHOD TO ADD THE SIZE OF THE FILE TO THE FILENAME.
int addSize(char * fname, long fsize){
	char * size;
	int n;

	n = sprintf(NULL, 0, "%ld", fsize);		/*Get the number of digits in fsize*/
	size = (char *)malloc(n+1);
	size[n] = '\0';
	snprintf(size, n+1, "%ld", fsize);
	fname = concat(3,fname, "/", size);	/*Add the file size to the name after a slash*/
}

long getSize(char * fname){
	long size;
	char * p;

	p = fname;
	while(*p != '/')
		p++;
	p++;

	return strtol(p, NULL, 10);
}
