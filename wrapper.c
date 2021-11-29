#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "gmp.h"
#include "misc.h"
#include "encodeOps.h"
#include "decodeOps.h"

//Global variable for encode
CD* cd; //struct compressData
DD* dd; //struct decompressData

int compressBuf(unsigned char* inBuf, int len, unsigned char** outBuf, int* outLen){
    cd = (CD*)malloc(sizeof(CD));
    cd->filename = NULL; cd->filenameLength = 0;
    cd->logLevel = 0;
    cd->inBufLen = len;
    //cd->of  = inFile;
    cd->inBuf = (unsigned char*)malloc(sizeof(unsigned char)*cd->inBufLen); memset(cd->inBuf, 0, sizeof(unsigned char)*cd->inBufLen);
    memcpy(cd->inBuf, inBuf, sizeof(unsigned char)*cd->inBufLen);
    cd->tG = 0;
    setGLogLevel(cd->logLevel);

    encodeBuffer(cd);
    //printf("fileBufLen: %d\n", cd->fileBufLen);
    *outBuf = (unsigned char*)malloc(sizeof(unsigned char)*cd->fileBufLen); memset(*outBuf, 0, sizeof(unsigned char)*cd->fileBufLen);
    memcpy(*outBuf, cd->fileBuf, sizeof(unsigned char)*cd->fileBufLen);
    //*outBuf=cd->fileBuf;
    *outLen = cd->fileBufLen;
    if(cd->inBuf != NULL)free(cd->inBuf);
    if(cd->fileBuf != NULL)free(cd->fileBuf); 
    if(cd != NULL )free(cd);
}
int decodeBuf(unsigned char* inBuf, int len, unsigned char** outBuf, int* outLen){
    dd = (DD*)malloc(sizeof(DD));
    dd->filename = NULL; dd->filenameLength = 0;
    dd->logLevel = 0;
    dd->inBufLen = len;
    //cd->of  = inFile;
    dd->inBuf = (unsigned char*)malloc(sizeof(unsigned char)*dd->inBufLen); 
    memset(dd->inBuf, 0, sizeof(unsigned char)*dd->inBufLen);
    memcpy(dd->inBuf, inBuf, sizeof(unsigned char)*dd->inBufLen);
    dd->tG = 0;

    decodeBuffer(dd);

    *outBuf = (unsigned char*)malloc(sizeof(unsigned char)*dd->decDataLen); 
    memset(*outBuf, 0, sizeof(unsigned char)*dd->decDataLen);
    memcpy(*outBuf, dd->decData, sizeof(unsigned char)*dd->decDataLen);
//	int i;
//	for(i=0;i<dd->decDataLen;i++)
//	{
//		(*outBuf)[i]=dd->decData[i];
//		 //printf("in decode:%d ",dd->decData[i]);
//	}
    *outLen = dd->decDataLen;
    

    if(dd->decData != NULL){ free(dd->decData); }
    if(dd->inBuf != NULL){ free(dd->inBuf); }
    if(dd != NULL){ free(dd); }
    //return(outBuf);
}
