#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gmp.h"
#include "misc.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to obtain a file handle */
/* Input: */
/* Output: */
FILE* openFile(int logging, char filename[1024], char mode[4]){
char cwd[1024];
if((char*)getcwd(cwd, sizeof(cwd)) != NULL){
        if(LOG){ debug(logging, 1, "(openFile)Current Working Directory: %s\n", cwd); }
} else
{
        if(LOG){ debug(logging, 0, "(openFile)Current Working Directory cannot be obtained. Exiting...\n"); } exit(0);
}
char filepath[1024];
sprintf(filepath,"%s/%s", cwd, filename);
FILE* file = fopen(filepath, mode);
if(file == NULL){
        if(LOG){ debug(logging, 1, "(openFile)%s cannot be opened. Exiting...\n", filepath); } exit(0);
} else {
        if(LOG){ debug(logging, 1, "(openFile)%s opened successfully.\n", filepath); }
}
return file;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to obtain a the length of the file */
/* Input: */
/* Output: */
int fileLen(int logging, FILE* fp){
int result = 0;
fseek(fp, 0, SEEK_END);
result = ftell(fp);
fseek(fp, 0, SEEK_SET);
if(LOG){ debug(logging, 1, "(fileLen)Length of the file is %d\n", result); }
return result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to read the whole file in the buffer */
/* Input: */
/* Output: */
int readFile(int logging, FILE* fp, int filelen, unsigned char** buf){
*buf = (unsigned char*)malloc(sizeof(unsigned char*)*filelen);
if(filelen != fread(*buf, 1, filelen, fp)){
	if(LOG){ debug(logging, 0, "(readFile)File cannot be read properly. Exiting...\n"); } exit(0);
}
if(LOG){ debug(logging, 1, "(readFile)Total number of characters read is %d\n", filelen); }
return SUCCESS;
}

