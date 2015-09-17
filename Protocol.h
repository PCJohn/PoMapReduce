//SYMBOLS FOR THE PROTOCOL.
/*File Operations*/
#define OPEN	 	'\1'
#define SAVE		'\2'
#define FILE_CONTENT	'\3'
#define SIGNAL		'\4'
/*MapReduce commands (Master to worker)*/
#define START_MAP	'\6'
#define START_REDUCE	'\10'
#define KILL		'\11'
/*Status*/
#define NOTIFY		'\12'
/*Stuff for the file system*/
#define CACHE		"/home/user/CProg/Cache/"	
#define MAX_PATH_LEN	1024
#define MAX_EXT_LEN 	5
#define MAX_WORKERS 	100
/*Names for the functions we want the user to write*/
#define SPLIT		"split"
#define MAP		"map"
#define	REDUCE		"reduce"
/*Mode for process*/
#define MASTER 1
#define WORKER 0

#ifndef mr_types_H_INCLUDED
#define mr_types_H_INCLUDED

struct pair{
	char * key;
	char * value;
};

struct wrkrTrack{
	int worker_fd[MAX_WORKERS];
	int workerCount;
	int * idle;
};

int isCmd(char);
int broadcastCmd(struct wrkrTrack, char, char *);
int cmd(int, char, char *);
int sendFile(struct wrkrTrack, char *, FILE *);
void handleConnection(int, int, int);
int processReceived(char, char *, int, int, int);
long getSize(char *);
int addSize(char *, long);
#endif
