#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include "Protocol.h"
#include "StringManip.h"

#define BUFFER_SIZE 	1024
#define MAX_LINE_LEN	1024
#define PROMPT 		">> "

struct wrkrTrack workers;
char mode;
int port, kvCount = 0;
pthread_t tid[2];

void runMR(char *);
int distribute(char *);
int gotoHelp(void);
void * acceptWorkers(void *);
void * mrCentral(void *);

void error(const char *msg){
	perror(msg);
	exit(1);
}

void* main(int argc, char * argv[]){

	//VARIABLES.
	extern int port;
	int i, currentSocket;
	struct sockaddr_in servAddress;
	struct sockaddr_in * pV4Addr;
	pthread_t thrd1, thrd2;
	
	//ACCEPTING THE PORT.
	if(argc == 1){
		printf("Enter the port number: ");
		scanf("%d",&port);
	}
	else if(argc == 2){
		port = atoi(argv[1]);
	}

	printf("\nWelcome to Po MapReduce [Version 1.1]\n");
	printf("Babi Carter productions\n\n");
	printf("Welcome Master Shifu\n");
	printf("Type:\t 'run' to start a MapReduce task\n\t 'help' for help\n\t 'quit' to quit\n");


	//CREATING THE PORT.
	printf("\nStarting the server...\n");

	currentSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(currentSocket < 0)
		error("ERROR: Couldn't create socket\n");
	else
		printf("Master started (Port %d)\n", port);
	
	//MAKING THE SERVER ADDRESS.
	bzero((char *)&servAddress, sizeof(servAddress));

	servAddress.sin_family = AF_INET;
	servAddress.sin_addr.s_addr = INADDR_ANY;
	servAddress.sin_port = htons(port);

	//BINDING.
	if(bind(currentSocket, (struct sockaddr *) &servAddress, sizeof(servAddress)) < 0)
		error("ERROR: Couldn't bind socket");

	//LISTENING FOR CONNECTIONS.
	printf("\nWaiting for connections...\n");
	listen(currentSocket,5);
	
	//INITIALIZE THE WORKER TRACKER.
	workers.workerCount = 0;

	//STARTING THE THREADS TO HANDLE THE I/O AND THE WORKER CONNECTIONS.
	pthread_create( &thrd1, NULL, mrCentral, NULL);
     	pthread_create( &thrd2, NULL, acceptWorkers, (void*)&currentSocket);

	pthread_join(thrd1, NULL);
	pthread_join(thrd2, NULL);

	//CLOSE THE STREAMS AND SOCKETS.
	for(i = 0; i < workers.workerCount; i++)
		close(workers.worker_fd[i]);
	close(currentSocket);

	return 0;
}

//ACCEPT COMMANDS FROM THE USER TO START THE MAPREDUCE TASK.
void *mrCentral(void* arg){

	char opt;
	char fname[MAX_PATH_LEN];
	char * cmd;
	extern int port;

	cmd = (char *)malloc(MAX_LINE_LEN+1);
	memset(fname, '\0', MAX_PATH_LEN);
	for(;;){
		printf(PROMPT);
		gets(cmd);
		trim(cmd);
		if(strcmp(cmd,"run") == 0){
			printf("Enter the file: ");
			scanf("%s",fname);
			runMR(fname);
		}
		else if(strcmp(cmd,"help") == 0)
			help();
		else if(strcmp(cmd,"quit") == 0)
			break;
	}

	pthread_exit(NULL);
}


//METHOD TO KEEP ACCEPTING WORKERS.
void * acceptWorkers(void * arg){

	int newWorker, currentSocket;
	extern struct wrkrTrack workers;
	struct sockaddr_in clientAddress;
	pid_t pid;
	socklen_t clilen;
	pthread_t workThreads[MAX_WORKERS];

	currentSocket = *(int *)arg;
	workers.idle = (int *)malloc(MAX_WORKERS);
	workers.idle += MAX_WORKERS;

	for(;;){
		//ACCEPTING THE CONNECTIONS.
		clilen = sizeof(clientAddress);
		newWorker = workers.worker_fd[workers.workerCount] = accept(currentSocket, (struct sockaddr*)&clientAddress, &clilen);
		workers.workerCount++;
		*workers.idle = newWorker;
		workers.idle--;

		if(newWorker < 0)
			error("ERROR: Couldn't accept data\n");

		//TODO:
		//CHILD PROCESS TO HANDLE THE CONNECTION WITH ONE WORKER.
		if(fork() == 0){
			printf("Worker %d has connected\n",workers.workerCount);
			handleConnection(newWorker, workers.workerCount, MASTER);
		}
	    }
}

void runMR(char * mrFile){
	extern struct wrkrTrack workers;
	char * objFile;
	char * sharedObj;
	char * interim;
	FILE * fp;
	int i;

	interim = changeExt(concat(2, CACHE, mrFile), ".txt");
	objFile = changeExt(mrFile,".o");
	sharedObj = changeExt(mrFile, ".so");

	//COMPILE AND MAKE THE SHARED OBJECT FILE.
	/*Compile the file only if it is not an executable*/
	if(strcmp(extension(mrFile), "c") == 0){
		compile(mrFile, objFile);
		makeSO(objFile, sharedObj);
	}
	/*Make a shared object file if we get an executable*/
	else if(strcmp(extension(mrFile), "o") == 0){
		makeSO(mrFile, sharedObj);
	}
	/*The user has already given us a shared object file*/
	else if(strcmp(extension(mrFile), "so") == 0){
		strcpy(sharedObj, mrFile);
	}

	//ASSIGNING THE MAP TASKS.
	distribute(sharedObj);

	//RUN THE SPLIT FUNCTION AND SHARE THE OUTPUT WITH THE USERS.
	/*We don't need to explicitly distribute the output: Done by the programmer*/
	broadcastCmd(workers, FILE_CONTENT, interim);
	run(sharedObj, SPLIT, workers.worker_fd, workers.workerCount);
	broadcastCmd(workers, FILE_CONTENT, NULL);

	/*All the workers are now working*/
	for(i = workers.workerCount; i >= 0; i--){
		*workers.idle = workers.worker_fd[i];
		workers.idle--;
	}

	//TELL ALL THE WORKERS TO START.
	broadcastCmd(workers, START_MAP, sharedObj);

	//START THE REDUCERS.
}


//REASSIGN THE MAP TASK IF THE WORKER LEAVES.
int reassign(){
	/**FILE * fp;
	int mostIdle;
	fp = fopen(sofname,"f");

	sendFile(*workers.idle,fp);
	signal(*workers.idle,START_MAP);**/
}

//HELP CENTRE.
int help(){
	//TODO:
	printf("This is help\n");
	return;
}

int distribute(char * fname){
	FILE * fp;
	extern struct wrkrTrack workers;
	int i;

	/*Send the file to all connected workers*/
	printf("Distributing the file...");
	fp = fopen(fname,"r");
	sendFile(workers, fname, fp);
	printf("done\n");
}
