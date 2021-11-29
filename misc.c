#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gmp.h"
#include "misc.h"


/* Function to double up the memory allocation */ 
void doubleUp(int** inBuf, int* bufMaxSize, int bufSize){
*bufMaxSize = *bufMaxSize * 2;
int* temp = (int*)malloc(sizeof(int)*(*bufMaxSize));
if(temp == NULL){printf("(doubleUp)malloc failure, fatal error. Exiting...");}
memset(temp, 0, sizeof(int)*(*bufMaxSize));
memcpy(temp, *inBuf, sizeof(int)*bufSize);
if(*inBuf != NULL){
	free(*inBuf);
	//printf("old block freed of size %d\n", *bufMaxSize/2);
}
*inBuf = temp;
//printf("(doubleUp)size of inBuf doubled to %d\n", *bufMaxSize);
}

/* Function to double up the memory allocation */ 
void doubleUpChar(unsigned char** inBuf, unsigned int* bufMaxSize, unsigned int bufSize){
*bufMaxSize = *bufMaxSize * 2;
unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char)*(*bufMaxSize));
if(temp == NULL){printf("(doubleUp)malloc failure, fatal error. Exiting...");}
memset(temp, 0, sizeof(unsigned char)*(*bufMaxSize));
memcpy(temp, *inBuf, sizeof(unsigned char)*bufSize);
if(*inBuf != NULL){
	free(*inBuf);
	//printf("old block freed of size %d\n", *bufMaxSize/2);
}
*inBuf = temp;
//printf("(doubleUp)size of inBuf doubled to %d\n", *bufMaxSize);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to apply modified mtf */
/* Input: */
/* Output: */
void modifiedMtf(unsigned char* in, int inLen, unsigned char** out){
if(LOG){ debug(HIGH, 1, "(modifiedMtf)Start inLen: %d\n", inLen); }
//*out = (unsigned char*)malloc(sizeof(unsigned char)*inLen);



}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to apply normal mtf */
/* Input: */
/* Output: */
void normalMtf(unsigned char* in, int inLen, unsigned char** out){
if(LOG){ debug(HIGH, 1, "(normalMtf)Start inLen: %d\n", inLen); }
*out = (unsigned char*)malloc(sizeof(unsigned char)*inLen);
int outLen = 0;

int i = 0, j = 0, mtfListLen = 256;
int cur = 0, prev = 0;
unsigned char* mtfList = (unsigned char*)malloc(sizeof(unsigned char)*mtfListLen);
for(i = 0; i < mtfListLen; i++){ mtfList[i] = i; }

for(i = 0; i < inLen; i++){
    if(in[i] == mtfList[0]){
        (*out)[outLen++] = 0;
        continue;
    }
    prev = mtfList[0];
    for(j = 1; j < mtfListLen; j++){
        cur = mtfList[j];
        mtfList[j] = prev;
        if(cur == in[i]){
            mtfList[0] = cur;
            (*out)[outLen++] = j;
            break;
        }
        prev = cur;
    }
}
if(LOG){ debug(MEDIUM, 0, "Mtf: "); }
for(i = 0; i < outLen; i++){
    if(LOG){ debug(MEDIUM, 1, "%d ", (*out)[i]); }
}
if(LOG){ debug(MEDIUM, 0, "\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to apply normal rmtf */
/* Input: */
/* Output: */
void normalRMtf(unsigned char* in, int inLen, unsigned char** out){
//void normalRMtf(unsigned char* in, int inLen, unsigned char** out){
if(LOG){ debug(VERY_HIGH, 1, "(normalRMtf)Start inLen: %d\n", inLen); }
*out = (unsigned char*)malloc(sizeof(unsigned char)*inLen);
int outLen = 0;

int i = 0, j = 0, mtfListLen = 256;
int cur = 0, prev = 0;
unsigned char* mtfList = (unsigned char*)malloc(sizeof(unsigned char)*mtfListLen);
for(i = 0; i < mtfListLen; i++){ mtfList[i] = i; }

for(i = 0; i < inLen; i++){
    if(in[i] == 0){
        (*out)[outLen++] = mtfList[0];
        continue;
    }
    prev = mtfList[0];
    for(j = 1; j < mtfListLen; j++){
        cur = mtfList[j];
        mtfList[j] = prev;
        if(j == in[i]){
            mtfList[0] = cur;
            (*out)[outLen++] = cur;
            break;
        }
        prev = cur;
    }
}
if(LOG){ debug(MEDIUM, 0, "RMtf: "); }
for(i = 0; i < outLen; i++){
    if(LOG){ debug(MEDIUM, 1, "%d ", (*out)[i]); }
}
if(LOG){ debug(MEDIUM, 0, "\n"); }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to apply normal rmtf */
/* Input: */
/* Output: */
void normalRMtfWithCheck(unsigned char* in, int inLen, unsigned char** out, unsigned char* verify){
//void normalRMtf(unsigned char* in, int inLen, unsigned char** out){
if(LOG){ debug(VERY_HIGH, 1, "(normalRMtf)Start inLen: %d\n", inLen); }
*out = (unsigned char*)malloc(sizeof(unsigned char)*inLen);
int outLen = 0;

int i = 0, j = 0, mtfListLen = 256;
int cur = 0, prev = 0;
unsigned char* mtfList = (unsigned char*)malloc(sizeof(unsigned char)*mtfListLen);
for(i = 0; i < mtfListLen; i++){ mtfList[i] = i; }

for(i = 0; i < inLen; i++){
    if(in[i] == 0){
        (*out)[outLen++] = mtfList[0];
        continue;
    }
    prev = mtfList[0];
    for(j = 1; j < mtfListLen; j++){
        cur = mtfList[j];
        mtfList[j] = prev;
        if(j == in[i]){
            mtfList[0] = cur;
            (*out)[outLen++] = cur;
            break;
        }
        prev = cur;
    }
}
if(LOG){ debug(MEDIUM, 0, "RMtf: "); }
for(i = 0; i < outLen; i++){
    if(LOG){ debug(MEDIUM, 1, "%d ", (*out)[i]); }
    if((*out)[i] != verify[i]){ printf("rmtf failure...\n"); }
}
if(LOG){ debug(MEDIUM, 0, "\n"); }
}
