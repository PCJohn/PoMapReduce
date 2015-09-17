#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include "mr_types.h"
#include "StringManip.h"

//METHOD TO CONCATENATE A SERIES OF STRINGS.
char * concat(int num,...){
	va_list strings;
	char * conc;
	int i;

	va_start(strings, num);
	conc = (char *)malloc(MAX_PATH_LEN+1);

	for(i = 0 ; i < num; i++)
		strcat(conc, va_arg(strings, char *));

	va_end(strings);
	return conc;
}

//RETURNS THE EXTENSION OF A FILENAME.
char * extension(char * fname){
	char * ext;
	int i, len;
	ext = (char *)malloc(MAX_EXT_LEN+1);
	len = strlen(fname);
	for(i = len-1; i >= 0; i--){
		if(fname[i] == '.')
			break;
		ext[len-i-1] = fname[i];
	}

	return rev(ext);
}

//CHANGES THE EXTENSION OF A FILENAME.
char * changeExt(char * fname, char * newExt){
	int i, len;
	char * changed;
	
	changed = (char *)malloc(strlen(fname)+strlen(newExt)+1);
	len = strlen(fname);
	for(i = 0; i < len; i++){
		if(fname[i] == '.')
			break;
		changed[i] = fname[i];
	}
	changed[i] = '\0';
	strcat(changed, newExt);

	return changed;
}

//METHOD TO EXTRACT THE FILE NAME FROM THE ENTIRE PATH.
char * getFName(char * fname){
	char * pathless;
	int i, len;
	
	pathless = (char *)malloc(MAX_PATH_LEN+1);
	memset(pathless, '\0', MAX_PATH_LEN);
	len = strlen(fname);
	i = len-1;
	while(i >= 0 && fname[i] != '/'){
		pathless[len-i-1] = fname[i];
		i--;
	}
	
	return rev(pathless);
}

//REVERSING A STRING.
char * rev(char * str){
	int i, len;
	char * reversed;

	len = strlen(str);
	reversed = (char *)malloc(len+1);
	for(i = 0; i < len; i++)
		reversed[len-i-1] = str[i];
	reversed[len] = '\0';
	return reversed;
}

//METHOD TO TRIM WHITE SPACES FROM A STRING.
trim(char * cmd){
	int i, len;
	len = strlen(cmd);
	for(i = 0; i < len; i++){
		if(cmd[i] != 0)
			break;
		else
			cmd += i;
	}
	for(i = len-1; i >= 0; i--){
		if(cmd[i] != 0)
			break;
		else
			cmd[i] = '\0';
	}
}

//METHOD TO GET THE SUBSTRING BETWEEN INDICES.
char * substr(char * str, int start, int end){
	char * sub;
	int i;
	sub = (char *)malloc(end-start);
	for(i = start; i < end; i++)
		*(sub+i) = *(str+i);
	*(sub+1) = '\0';

	return sub;
}
