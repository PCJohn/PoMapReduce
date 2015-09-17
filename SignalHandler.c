#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "mr_types.h"

#define BUFF_SIZE 1024

char * getFName(char *);

//METHOD TO CHECK IF WE RECEIVED A SIGNAL.
int isCmd(char ch){
	switch(ch){
		case OPEN		:
		case SAVE		:
		case FILE_CONTENT	:
		case SIGNAL		:
		case START_MAP		:
		case START_REDUCE	:
		case KILL		:
		case SUCCESS		:
		case FAIL		: return 1;
	}

	return 0;
}

int broadcast(struct wrkrTrack workers, char signal, char * fname){
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
	pathless = getFName(fname);
	write(fd, &signal, sizeof(char));
	if(fname != NULL){
		write(fd, pathless, strlen(pathless));
		write(fd, &signal, sizeof(char));
	}
}

int sendFile(struct wrkrTrack workers, char * fname, FILE * fp){
	char ch;
	char buff[BUFF_SIZE];	/*Buffer for sending the file*/
	int i, j, fd;
	memset(buff,'\0',BUFF_SIZE);

	//SIGNAL THE WORKERS TO ACCEPT THE FILE.
	broadcast(workers, SAVE, fname);

	//SENDING THE CONTENTS: USE ESCAPING TO DISTINGUISH SIGNALS.
	i = 0;
	ch = '\0';
	while(fscanf(fp,"%c",&ch) != EOF){
		if(i >= BUFF_SIZE-1){
			i = 0;
			/*Share the buffer with all the workers*/
			for(j = 0; j < workers.workerCount; j++)
				write(workers.worker_fd[j], buff, BUFF_SIZE);
			memset(buff, '\0', BUFF_SIZE);
			continue;
		}
		if(ch == FILE_CONTENT)
			buff[i++] = ch;		/*Send the character twice if it is a signal*/
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
/**
int remoteOpen(struct wrkrTrack workers, char * fname){
	char ch, i = 0;
	//SENDING THE FILENAME TO BE OPENED.
	signal(fd,OPEN);
	write(fd, fname, strlen(fname));
	//SIGNAL THE END OF THE REQUEST.
	signal(fd,SIGNAL);

}

//SAVE A FILE ON ANOTHER SYSTEM.
int remoteSave(int fd, char * fname){
	FILE * fp;
	char ch;
	//SEND THE FILENAME TO BE SAVED.
	signal(fd, SAVE);
	write(fd, fname, strlen(fname));
	//SIGNAL THE END OF THE FILENAME AND SEND THE CONTENTS.
	signal(fd, SIGNAL);
	fp = fopen(fname,"r");
	sendFile(fd, fname, fp);
}**/

