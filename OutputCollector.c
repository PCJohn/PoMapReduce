#include <stdio.h>
#include "mr_types.h"

collect(struct pair output){
	
	static long kvCount;
	char * line;
	int i = 0;

	//BUILDING THE INTERIM FILE LINE 
	line = concat(3,ouput.key,"\t",ouput.value);
	//WRITE THE LINE TO EVERY RECEVIER
	while(i < worker.workerCount)
		write(workers.worker_fd[i++], line, strlen(line));

	kvCount++;
}

