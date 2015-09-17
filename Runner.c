#include <stdio.h>
#include <dlfcn.h>
#include "Protocol.h"
#include "StringManip.h"

void run(char *, char *, int[MAX_WORKERS], int);
int compile(char *, char *);
int makeSO(char *, char *);

//METHOD TO RUN A METHOD IN A LIBRARY.
void run(char * prog, char * fnctName, int fd[MAX_WORKERS], int connectedCount){
	void * dlModule;
	int (*funct)(int[MAX_WORKERS], int);
	const char * error;

	dlModule = dlopen(prog, RTLD_LAZY);
	funct = dlsym(dlModule,fnctName);
	if((error = dlerror())){
		printf("ERROR: %s\n", error);
	}
	(*funct)(fd, connectedCount);
	dlclose(dlModule);
}

//METHOD TO COMPILE A FILE.
int compile(char * prog, char * final){
	int status;
	char * cmd;

	cmd = concat(4,"gcc -c ", prog, " -o ", final);
	status = system(cmd);

	return status;
}

//METHOD TO MAKE A SHARED OBJECT FILE.
int makeSO(char * exeFile, char * final){
	int status;
	char * cmd;

	cmd = concat(4, "gcc -fPIC -shared ", exeFile, " Master.c StringManip.c Runner.c CmdHandler2.c -o ", final);
	status = system(cmd);
	/**cmd = concat(4, "gcc -shared ", "interim.o", " -o ", final);
	status = system(cmd);**/
	return status;
}
