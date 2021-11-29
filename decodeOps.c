#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "gmp.h"
#include "misc.h"
#include "decodeOps.h"
#include "encodeOps.h"


//Global variable for inData
//DD* id; //Moved to main.c
void combineRelativePosNew(unsigned char  *orgBuf,unsigned int fileSize,unsigned short seqCount,unsigned char *seq,unsigned int *rp[],unsigned int *rpCount)
{
                int k,i,j;
                //unsigned int *temp=(unsigned int*)malloc(fileSize*4);
                unsigned int *temp=(unsigned int*)malloc(fileSize*sizeof(unsigned int));
                for(i=0;i<fileSize;i++)
                {
                //orgBuf[i]=i;
                //for(i=0;i<fileSize;i++)
                temp[i]=i;
                }
                unsigned int tempSize=0;
                unsigned int tempCount=0;
                unsigned int prevCount=fileSize;
                for(i=0;i<seqCount;i++)
                {
                        tempSize = prevCount;
                        tempCount=0;
                        for(j=0;j<rpCount[seq[i]];j++)
                        {
                              //unsigned char *ptr ;
                                orgBuf[temp[rp[seq[i]][j]-1]] = seq[i];
                               temp[rp[seq[i]][j]-1]= 0xffffffff ;
                        }
                        for(k=0;k<tempSize;k++)
                        {
                                if(temp[k] != 0xffffffff)
                                temp[tempCount++]=temp[k];
                        }
                        prevCount = tempCount;

                }
                if(temp!=NULL)free(temp);

 }


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to extract the bits from the splitDensityIdBytes */
/* Input: */
/* Output: */
void extractSplitDensityIdBitsOld(DD* id){
if(LOG){ debug(HIGH, 1, "\n(extractSplitDensityIdBits)Start\n"); }
int i = 0, j = 0;
unsigned char tempChar = 0;
printf("splitCharsCount: %d\n", id->splitCharsCount);
id->splitDensityIdBits = (unsigned char*)malloc(sizeof(unsigned char)*(id->splitCharsCount*8 + 1));
memset(id->splitDensityIdBits, 0, sizeof(unsigned char)*(id->splitCharsCount*8 + 1));
id->splitDensityIdBitsCount = 0;
for(i = 0; i < (int)ceil((float)(id->splitCharsCount*3)/(float)8)-1; i++){ //RECALL
    tempChar = id->splitDensityIdBytes[i];
    for(j = 0; j < 8; j++){
        if(tempChar & 1 == 1){
            id->splitDensityIdBits[id->splitDensityIdBitsCount++] = 1;
        } else {
            id->splitDensityIdBits[id->splitDensityIdBitsCount++] = 0;
        }
        tempChar >>= 1;
    }
}
tempChar = id->splitDensityIdBytes[(int)ceil((float)(id->splitCharsCount*3)/(float)8)-1];
for(j = 0; j < id->splitCharsCount*3 % 8; j++){
    if(tempChar & 1 == 1){
       id->splitDensityIdBits[id->splitDensityIdBitsCount++] = 1;
    } else {
       id->splitDensityIdBits[id->splitDensityIdBitsCount++] = 0;
    }
    tempChar >>= 1;
}
printf("splitDensityIdBitsCount: %d\n", id->splitDensityIdBitsCount);
for(i = 0; i < id->splitDensityIdBitsCount; i++){
    if(LOG){ debug(LOW, 1, "%d ", id->splitDensityIdBits[i]); }
    printf("%d ", id->splitDensityIdBits[i]);
}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to extract the bits from the splitDensityIdBytes */
/* Input: */
/* Output: */
void extractSplitDensityIdBits(DD* id){
if(LOG){ debug(HIGH, 1, "\n(extractSplitDensityIdBits)Start\n"); }
int i = 0, j = 0;
unsigned char tempChar = 0, testByte = 0;
//printf("splitCharsCount: %d\n", id->splitCharsCount);
//printf("splitDensityIdBytesLen: %d\n", id->splitDensityIdBytesLen);
id->splitDensityIdBits = (unsigned char*)malloc(sizeof(unsigned char)*(id->splitDensityIdBytesLen*8));
memset(id->splitDensityIdBits, 0, sizeof(unsigned char)*(id->splitDensityIdBytesLen*8));
unsigned short tempSplitDensityIdBitsCount = 0;
if(id->splitDensityIdBitsCount % 8 != 0){ //read the partial byte
    tempChar = id->splitDensityIdBytes[id->splitDensityIdBytesLen-1];
    //printf("tempChar: %d\n", tempChar);
    testByte = 1; testByte <<= (id->splitDensityIdBitsCount % 8)-1;
    for(j = 0; j < id->splitDensityIdBitsCount % 8; j++){
        if(tempChar & testByte){
            id->splitDensityIdBits[tempSplitDensityIdBitsCount++] = 1;
        } else {
            id->splitDensityIdBits[tempSplitDensityIdBitsCount++] = 0;
        }
        testByte >>= 1;
    }
    for(i = id->splitDensityIdBytesLen-2; i >= 0; i--){ //RECALL
        tempChar = id->splitDensityIdBytes[i];
        //printf("tempChar: %d\n", tempChar);
        testByte = 1; testByte <<= 7;
        for(j = 0; j < 8; j++){
            if(tempChar & testByte){
                id->splitDensityIdBits[tempSplitDensityIdBitsCount++] = 1;
            } else {
                id->splitDensityIdBits[tempSplitDensityIdBitsCount++] = 0;
            }
            testByte >>= 1;
        }
    }
} else { //all full bytes
    for(i = id->splitDensityIdBytesLen-1; i >= 0; i--){ //RECALL
        tempChar = id->splitDensityIdBytes[i];
        testByte = 1; testByte <<= 7;
        for(j = 0; j < 8; j++){
            if(tempChar & testByte){
                id->splitDensityIdBits[tempSplitDensityIdBitsCount++] = 1;
            } else {
                id->splitDensityIdBits[tempSplitDensityIdBitsCount++] = 0;
            }
            testByte >>= 1;
        }
    }
}

//printf("splitDensityIdBitsCount: %d\n", id->splitDensityIdBitsCount);
if(tempSplitDensityIdBitsCount != id->splitDensityIdBitsCount){ 
    printf("FATAL ERROR: splitDensityIdBitsCount(%d) read in file is different from the count(%d) derived from the splitDensityIdBytes\n", id->splitDensityIdBitsCount, tempSplitDensityIdBitsCount);
}
for(i = 0; i < id->splitDensityIdBitsCount; i++){
    if(LOG){ debug(LOW, 1, "%d ", id->splitDensityIdBits[i]); }
    //printf("%d ", id->splitDensityIdBits[i]);
}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to convert the partialBytes into partialBytesBuf(expanded array of 0s and 1s) */
/* Input: */
/* Output: */
void extractPartialBytesBufFromBytes(DD* id){
if(LOG){ debug(HIGH, 1, "\n(extractPartialBytesBufFromBytes)Start\n"); }
int i = 0, j = 0;
unsigned char curByte = 0;
id->partialBytesBuf = (unsigned char*)malloc(sizeof(unsigned char)*id->partialBytesBufLen + 8);
memset(id->partialBytesBuf, 0, sizeof(unsigned char)*id->partialBytesBufLen + 8);
int inBufLen = 0;

//Read all bits into an array
for(i = 0; i < (int)ceil((float)id->partialBytesBufLen/(float)8); i++){
    curByte = id->partialBytes[i];
    if(LOG){ (id->logLevel, 1, "\n curByte: %d ", curByte); }
    for(j = 0; j < 8; j++){
        if(curByte & 128){
              id->partialBytesBuf[inBufLen++] = 1; if(LOG){ (id->logLevel, 1, "ONE "); }
        } else {
              id->partialBytesBuf[inBufLen++] = 0; if(LOG){ (id->logLevel, 1, "ZERO "); }
        }
        curByte <<= 1;
    }
}
if(LOG){ for(i = 0; i < id->partialBytesBufLen; i++){ if(LOG){ (id->logLevel, 1, "%d ", id->partialBytesBuf[i]); } } }
if(LOG){ (id->logLevel, 1, "\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to extract the n values from the nCountBytes */
/* Input: */
/* Output: */
void extractNCountFromBuf(unsigned char* nCountBytes, int nCountBufLen, int maxN, DD* id){
if(LOG){ debug(HIGH, 1, "\n(extractNCountFromBuf)Start: nCountBufLen: %d, maxN: %d\n", nCountBufLen, maxN); }
int i = 0, j = 0;
unsigned char curByte = 0;
unsigned char* inBuf = (unsigned char*)malloc(sizeof(unsigned char)*nCountBufLen + 8);
memset(inBuf, 0, sizeof(unsigned char)*nCountBufLen + 8);
int inBufLen = 0;

//Read all bits into an array
for(i = 0; i < (int)ceil((float)nCountBufLen/(float)8); i++){
    curByte = nCountBytes[i];
    if(LOG){ debug(LOW, 1, "\n curByte: %d ", curByte); }
    for(j = 0; j < 8; j++){
        if(curByte & 128){
              inBuf[inBufLen++] = 1; if(LOG){ debug(VERY_LOW, 1, "ONE "); }
        } else {
              inBuf[inBufLen++] = 0; if(LOG){ debug(VERY_LOW, 1, "ZERO "); }
        }
        curByte <<= 1;
    }
}
if(LOG){ for(i = 0; i < nCountBufLen; i++){ if(LOG){ debug(LOW, 1, "%d ", inBuf[i]); } } }

id->nCountValues = (unsigned int*)malloc(sizeof(unsigned int)*id->charDataCount); memset(id->nCountValues, 0, sizeof(unsigned int)*id->charDataCount);

//Extract n values from the array, first value is the maxN occupying first bit as 1
int bcCurN = 0, lastN = maxN,valuesCount = 0;
unsigned int tempInt = 0;
if(LOG){ debug(LOW, 1, "\ntempInt: %d\n", maxN); }
id->nCountValues[valuesCount++] = maxN;
for(i = 1; i < nCountBufLen;){
    bcCurN = bitCount(lastN);
    tempInt = 0;
    for(j = 0; j < bcCurN; j++){
        tempInt = tempInt | (inBuf[i++] << j);
    }
    if(LOG){ debug(LOW, 1, "tempInt: %d\n", tempInt); }
    id->nCountValues[valuesCount++] = tempInt;
    lastN = tempInt;
}

if(inBuf != NULL){ free(inBuf); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to display distinct characters */
/* Input: */
/* Output: */
void displayCharData(DD* id){
if(LOG){ debug(id->logLevel, 0, "\n(displayCharData)Start\n"); }
int i = 0;
if(LOG){ for(i = 0; i < id->charDataCount; i++){ if(LOG){ debug(LOW, 1, "%d ", id->charData[i]); } } }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to display the nCount values */
/* Input: */
/* Output: */
void displayNCountValues(DD* id){
if(LOG){ debug(id->logLevel, 0, "\n(displayNCountValues)Start\n"); }
int i = 0;
if(LOG){ for(i = 0; i < id->charDataCount; i++){ if(LOG){ debug(LOW, 1, "%d ", id->nCountValues[i]); } } }
if(LOG){ debug(id->logLevel, 0, "\n"); }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to display the rCount values */
/* Input: */
/* Output: */
void displayRCountValues(DD* id){
if(LOG){ debug(id->logLevel, 0, "\n(displayRCountValues)Start\n"); }
int i = 0;
if(LOG){ for(i = 0; i < id->rCountBufLen; i++){ if(LOG){ debug(LOW, 1, "%d ", id->rCountBuf[i]); } } }
if(LOG){ debug(id->logLevel, 0, "\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to display the rCount values */
/* Input: */
/* Output: */
void displayNCountRandomValues(DD* id){
if(LOG){ debug(id->logLevel, 0, "\n(displayNCountRandomValues)Start\n"); }
int i = 0;
if(LOG){ for(i = 0; i < id->nCountRandomLen; i++){ if(LOG){ debug(LOW, 1, "%d ", id->nCountRandomBuf[i]); } } }
if(LOG){ debug(id->logLevel, 0, "\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to display the splitChars */
/* Input: */
/* Output: */
void displaySplitChars(DD* id){
if(LOG){ debug(id->logLevel, 0, "\n(displaySplitChars)Start\n"); }
int i = 0;
if(LOG){ for(i = 0; i < id->splitCharsCount; i++){ if(LOG){ debug(LOW, 1, "%d ", id->splitChars[i]); } } }
if(LOG){ debug(id->logLevel, 0, "\n"); }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to verify the magic split */
/* Input: */
/* Output: */
//void combineSplit(int* rpZero, int rpZeroN, int rpZeroCount, int* rpOne, int rpOneN, int rpOneCount, unsigned char* splitId, int splitIdCount, int* rpOrig){
void combineSplitPositions(SD* sD){
int rpZeroN = sD->rpZeroN, rpZeroCount = sD->rpZeroCount, rpOneN = sD->rpOneN, rpOneCount = sD->rpOneCount, splitIdCount = sD->splitIdCount;
if(LOG){ debug(HIGH, 0, "(combineSplit)Start rpZeroN: %d, rpZeroCount: %d, rpOneN: %d, rpOneCount: %d, splitIdCount: %d\n", rpZeroN, rpZeroCount, rpOneN, rpOneCount, splitIdCount); }
int* rpZeroNew = (int*)malloc(sizeof(int)*rpZeroN*2); memset(rpZeroNew, 0, sizeof(int)*rpZeroN*2); int rpZeroNewCount = 0, rpZeroNewN = 0;
int* rpOneNew = (int*)malloc(sizeof(int)*rpOneN*2); memset(rpOneNew, 0, sizeof(int)*rpOneN*2); int rpOneNewCount = 0, rpOneNewN = 0;
int i = 0, j = 0, curPos = -1, prevPos = -1;
int rpZeroCurPos = 0, rpOneCurPos = 0;

//printf("\nNKrpZero: "); for(i = 0; i < rpZeroCount; i++){ printf("%d ", sD->rpZero[i]); } printf(" (%d)(%d)\n", rpZeroN, rpZeroCount);
//printf("\nNKrpOne: "); for(i = 0; i < rpOneCount; i++){ printf("%d ", sD->rpOne[i]); } printf(" (%d)(%d)\n", rpOneN, rpOneCount);
//printf("\nNKsplitId: "); for(i = 0; i < splitIdCount; i++){ printf("%d ", sD->splitId[i]); } printf("\n");

//Find new positions for zero
if(rpZeroCount > 0){
if(sD->rpZero[0] == 1){
    rpZeroNewN++;
    rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
} else {
    if(sD->rpZero[0] > 1){
        for(j = 1; j < sD->rpZero[0]; j++){ //01
            rpZeroNewN++; //For zero
            rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
            rpZeroNewN++; //For one
        }
        rpZeroNewN++; //For zero
        rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
    }
}
prevPos = sD->rpZero[0];
for(i = 1; i < rpZeroCount; i++){
    curPos = sD->rpZero[i]; 
    if(curPos == prevPos + 1){
        rpZeroNewN++; //For zero
        rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
    } else {
        for(j = 1; j <= curPos - prevPos - 1; j++){
            rpZeroNewN++; //For zero
            rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
            rpZeroNewN++; //For one
        }
        rpZeroNewN++; //For zero
        rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
    }
    prevPos = curPos;
}
if(rpZeroN == sD->rpZero[rpZeroCount-1]){
    rpZeroNewN++; //For zero
    rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
}
for(j = 1; j <= rpZeroN - sD->rpZero[rpZeroCount-1]; j++){
    rpZeroNewN++; //For zero
    rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
    rpZeroNewN++; //For one
}
} else {
    if(rpZeroN != 0 && rpZeroCount == 0){
        for(j = 1; j <= rpZeroN; j++){
            rpZeroNewN++; //For zero
            rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
            rpZeroNewN++; //For one
        }
    }
}
if(LOG){ debug(HIGH, 2, "rpZeroNewN: %d, rpZeroNewCount: %d\n", rpZeroNewN, rpZeroNewCount); }

//Find new positions for one
if(rpOneCount > 0){
    rpOneNewN += sD->rpOne[0] + 1;
    rpOneNew[rpOneNewCount++] = rpOneNewN;
    prevPos = sD->rpOne[0];
    for(i = 1; i < rpOneCount; i++){
        curPos = sD->rpOne[i];
        rpOneNewN += curPos - prevPos + 1;
        rpOneNew[rpOneNewCount++] = rpOneNewN;
        prevPos = curPos;
    }
    //rpOneNewN += rpOneN - rpOne[rpOneCount-1] + 1;
    rpOneNewN += rpOneN - sD->rpOne[rpOneCount-1];
} else {
    if(rpOneN != 0 && rpOneCount == 0){
        rpOneNewN += rpOneN + 1;
        rpOneNewCount = 0;
    }
}
if(LOG){ debug(HIGH, 2, "rpOneNewN: %d, rpOneNewCount: %d\n", rpOneNewN, rpOneNewCount); }

//int* rp = (int*)malloc(sizeof(int)*(rpZeroNewN+rpOneNewN+1)*2); memset(rp, 0, sizeof(int)*(rpZeroNewN+rpOneNewN+1)*2); int rpCount = 0, runningN = 0;
sD->allRP = (int*)malloc(sizeof(int)*(rpZeroNewN+rpOneNewN+1)*2); memset(sD->allRP, 0, sizeof(int)*(rpZeroNewN+rpOneNewN+1)*2); int rpCount = 0, runningN = 0;

for(i = 0; i < splitIdCount-1; i++){
    if(sD->splitId[i] == 1){
        if(rpOneCurPos == 0){
            runningN += rpOneNew[rpOneCurPos++];
            sD->allRP[rpCount++] = runningN;
        } else {
            runningN += rpOneNew[rpOneCurPos] - rpOneNew[rpOneCurPos-1];
            sD->allRP[rpCount++] = runningN;
            rpOneCurPos++;
        }
    } else {
            runningN++;
            sD->allRP[rpCount++] = runningN;
            rpZeroCurPos++;
            while(rpZeroCurPos < rpZeroNewCount && rpZeroNew[rpZeroCurPos] - rpZeroNew[rpZeroCurPos-1] == 1){
                runningN++;
                sD->allRP[rpCount++] = runningN;
                rpZeroCurPos++;
            }
            runningN++;
    }
    //printf("rpCount: %d, runningN: %d\n", rpCount, runningN);
}
if(sD->splitId[splitIdCount-1] == 1){
    if(rpOneCurPos == rpOneCount){
        runningN += rpOneNewN - rpOneNew[rpOneCurPos-1];
    } else {
        runningN += rpOneNew[rpOneCurPos] - rpOneNew[rpOneCurPos-1];
        sD->allRP[rpCount++] = runningN;
    }
} else {
            runningN++; rpZeroCurPos++;
            sD->allRP[rpCount++] = runningN;
            while(rpZeroCurPos < rpZeroNewCount && rpZeroNew[rpZeroCurPos] - rpZeroNew[rpZeroCurPos-1] == 1){
                runningN++;
                sD->allRP[rpCount++] = runningN;
                rpZeroCurPos++;
            }
            if(rpZeroNew[rpZeroNewCount-1] + 1 == rpZeroNewN){
                runningN++;
            }
}
//printf("RP CHECK: "); 
//for(i = 0; i < rpCount; i++){ 
//    printf("%d ", sD->allRP[i]); 
    //if(rp[i] != rpOrig[i]){ printf("rp failure...\n"); }
//} printf("\n");
//printf("rpCount: %d, runningN: %d\n", rpCount, runningN);


//if(rp != NULL){ free(rp); }
if(rpZeroNew != NULL){ free(rpZeroNew); }
if(rpOneNew != NULL){ free(rpOneNew); }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to verify the magic split */
/* Input: */
/* Output: */
void combineSplit(SD* sD){
int i = 0;
unsigned char* splitBuf[3]; unsigned int splitBufLen[3];
splitBuf[0] = (unsigned char*)malloc(sizeof(unsigned char)*(sD->rpZeroN)); memset(splitBuf[0], 1, sizeof(unsigned char)*(sD->rpZeroN));
splitBufLen[0] = sD->rpZeroN;
splitBuf[1] = (unsigned char*)malloc(sizeof(unsigned char)*(sD->rpOneN)); memset(splitBuf[1], 1, sizeof(unsigned char)*(sD->rpOneN));
splitBufLen[1] = sD->rpOneN;
splitBuf[2] = (unsigned char*)malloc(sizeof(unsigned char)*(sD->splitIdCount)); memset(splitBuf[2], 1, sizeof(unsigned char)*(sD->splitIdCount));
splitBufLen[2] = sD->splitIdCount;
for(i = 0; i < sD->rpZeroCount; i++){ splitBuf[0][sD->rpZero[i]-1] = 0; }
for(i = 0; i < sD->rpOneCount; i++){ splitBuf[1][sD->rpOne[i]-1] = 0; }
for(i = 0; i < sD->splitIdCount; i++){ splitBuf[2][i] = sD->splitId[i]; }
//printf("splitBufLen0: %d, 1: %d, 2: %d\n", splitBufLen[0], splitBufLen[1], splitBufLen[2]);

/*
for(i = 0; i < 3; i++){
    int j = 0, zeroCount = 0;
    printf("%d%d%d: ", i, i, i);
    for(j = 0; j < splitBufLen[i]; j++){
        printf("%d", splitBuf[i][j]);
        if(splitBuf[i][j] == 0){ zeroCount++; }
    }
    printf("\nn: %d, zeroCount: %d\n", splitBufLen[i], zeroCount);
}
*/

//Verify
unsigned int verifyMaxLen = (sD->rpZeroN)*2+(sD->rpOneN)*2+sD->splitIdCount; verifyMaxLen *= 2;
unsigned char* verify = (unsigned char*)malloc(sizeof(unsigned char)*verifyMaxLen); memset(verify, 0, sizeof(unsigned char)*verifyMaxLen);
unsigned int verifyLen = 0;
unsigned char curChar = -1;
unsigned int verifyBufLen[2]; verifyBufLen[0] = 0; verifyBufLen[1] = 0;
for(i = 0; i < splitBufLen[2]; i++){
    curChar = splitBuf[2][i];
    if(curChar == 0){
        while(verifyBufLen[curChar] < splitBufLen[curChar] && splitBuf[curChar][verifyBufLen[curChar]] == curChar){
            verify[verifyLen++] = curChar; verifyBufLen[curChar]++;
        }
        verify[verifyLen++] = curChar;
        if(verifyBufLen[curChar] < splitBufLen[curChar]){
            verify[verifyLen++] = 1; verifyBufLen[curChar]++;
        }
    } else {
        while(verifyBufLen[curChar] < splitBufLen[curChar] && splitBuf[curChar][verifyBufLen[curChar]] == curChar){
            verify[verifyLen++] = curChar; verifyBufLen[curChar]++;
        }
        verify[verifyLen++] = curChar;
        if(verifyBufLen[curChar] < splitBufLen[curChar]){
            verify[verifyLen++] = 0; verifyBufLen[curChar]++;
        }
    }
}
//printf("verifyLen: %d, filelen: %d\n", verifyLen, filelen);
//printf("verifyLen: %d\n", verifyLen);
//for(i = 0 ; i < verifyLen; i++){
//    if(inBuf[i] != verify[i]){ printf("ERROR\n"); }
    //printf("orig: %d, verify: %d\n", inBuf[i], verify[i]);
//}

sD->allRP = (unsigned int*)malloc(sizeof(unsigned int)*verifyLen); memset(sD->allRP, 0, sizeof(unsigned int)*verifyLen); unsigned int rpCount = 0, runningN = 0;
for(i = 0; i < verifyLen; i++){
    if(verify[i] == 0){
        sD->allRP[rpCount++] = i+1;
    }
}
//printf("rpCount: %d\n", rpCount);

//printf("RP CHECK: "); 
//for(i = 0; i < rpCount; i++){ 
//    printf("%d ", sD->allRP[i]); 
    //if(rp[i] != rpOrig[i]){ printf("rp failure...\n"); }
//} printf("\n");
//printf("rpCount: %d, runningN: %d\n", rpCount, runningN);


//if(rp != NULL){ free(rp); }
for(i = 0; i < 3; i++){ if(splitBuf[i] != NULL){ free(splitBuf[i]); } }
if(verify != NULL){ free(verify); }
}


void initializeBlockDataDecompress(BD** bD){
int i = 0;
(*bD) = (BD*)malloc(sizeof(BD));
(*bD)->blockSize = D_BLOCK_SIZE;
(*bD)->blockTypesCount = D_BLOCK_TYPES;
(*bD)->blockCount = 1;
//(*bD)->blockId = (unsigned char*)malloc(sizeof(unsigned char)*((nCount/(*bD)->blockSize)+1)); memset((*bD)->blockId, 0, sizeof(unsigned char)*((nCount/(*bD)->blockSize)+1));
(*bD)->blockId = NULL;
(*bD)->blockIdLen = 0;
(*bD)->blockRP = (unsigned int**)malloc(sizeof(unsigned int*)*((*bD)->blockTypesCount)); memset((*bD)->blockRP, 0, sizeof(unsigned int*)*((*bD)->blockTypesCount));
(*bD)->blockRPCount = (unsigned int*)malloc(sizeof(unsigned int)*(*bD)->blockTypesCount);
(*bD)->blockRPTypesCount = (unsigned int*)malloc(sizeof(unsigned int)*(*bD)->blockTypesCount);
(*bD)->blockRPNCount = (unsigned int*)malloc(sizeof(unsigned int)*(*bD)->blockTypesCount);
for(i = 0; i < (*bD)->blockTypesCount; i++){
    //(*bD)->blockRP[i] = (unsigned int*)malloc(sizeof(unsigned int)*nCount); memset((*bD)->blockRP[i], 0, sizeof(unsigned int)*nCount);
    (*bD)->blockRP[i] = NULL;
    (*bD)->blockRPCount[i] = 0;
    (*bD)->blockRPTypesCount[i] = 0;
    (*bD)->blockRPNCount[i] = 0;
}
(*bD)->blockStartPos = 0;
(*bD)->allBlocksRPCount = 0;
}


void cleanupBlockDataDecompress(BD* bD){
int i = 0;
if(LOG){ debug(LOW, 0, "Free blockData decompress\n"); }
if(bD->blockId != NULL){ free(bD->blockId); }
for(i = 0; i < bD->blockTypesCount; i++){ if(bD->blockRP[i] != NULL){ free(bD->blockRP[i]); } }
if(bD->blockRP != NULL){ free(bD->blockRP); }
if(bD->blockRPCount != NULL){ free(bD->blockRPCount); }
if(bD->blockRPTypesCount != NULL){ free(bD->blockRPTypesCount); }
if(bD->blockRPNCount != NULL){ free(bD->blockRPNCount); }
if(bD->allRP != NULL){ free(bD->allRP); }
if(bD != NULL){ free(bD); }
if(LOG){ debug(LOW, 0, "blockData freed decompress\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to verify the blocks by combining them and comparing them with the original as 0 and 1(slow) */
/* Input: */
/* Output: */
void combineBlocksSlow(BD* bD, DD* id){
if(LOG){ debug(HIGH, 1, "\n(combineBlocksSlow)Start\n"); }
int i = 0, j = 0;
unsigned char* tempBuf[D_BLOCK_TYPES]; 
int tempBufLen[D_BLOCK_TYPES], tempTypesCount[D_BLOCK_TYPES];
int offset[D_BLOCK_TYPES], blockType = -1, finalBufLen = 0, finalBufOffset = 0;
//Initialize
for(i = 0; i < D_BLOCK_TYPES; i++){
    if(bD->blockRPNCount[i] != 0){
        //printf("tempBuf[%d]: %d\n", i, bD->blockRPNCount[i]);
        tempBuf[i] = (unsigned char*)malloc(sizeof(unsigned char)*bD->blockRPNCount[i]);
        memset(tempBuf[i], 1, sizeof(unsigned char)*bD->blockRPNCount[i]);
        finalBufLen += bD->blockRPNCount[i];
        tempBufLen[i] = 0;
    } else { tempBuf[i] = NULL; }
    tempBufLen[i] = 0; offset[i] = 0; tempTypesCount[i] = 0;
}

unsigned int finalBufAllocSize = finalBufLen*2;
unsigned char* finalBuf = (unsigned char*)malloc(sizeof(unsigned char)*finalBufAllocSize); //Check later
memset(finalBuf, 0, sizeof(unsigned char)*finalBufAllocSize);
//printf("finalBufAllocSize: %d\n", finalBufAllocSize);
//printf("NK CHK: finalBufLen*8: %d(%d)\n", finalBufLen, finalBufLen*8);
//Convert them as 0 and 1
for(i = 0; i < D_BLOCK_TYPES; i++){
    if(bD->blockRPNCount[i] != 0){
        for(j = 0; j < bD->blockRPCount[i]; j++){
            tempBuf[i][bD->blockRP[i][j]-1] = 0;
        }
        tempBufLen[i] = bD->blockRPNCount[i];
    }
}

//Display
//for(i = 0; i < D_BLOCK_TYPES; i++){
//    printf("Type%d: ", i); for(j = 0; j < tempBufLen[i]; j++){ printf("%d", tempBuf[i][j]); } printf("\n");
//}
//printf("CHOK blockIdLen: %d, blockIdOffset: %d\n", bD->blockIdLen, id->blockIdOffset);
//printf("FINAL:");
bD->blockId = (unsigned char*)malloc(sizeof(unsigned char)*bD->blockIdLen); memset(bD->blockId, 0, sizeof(unsigned char)*bD->blockIdLen);
//printf("CHOK blockIdLen: %d, blockIdOffset: %d, blockIdN: %d\n", bD->blockIdLen, id->blockIdOffset, id->blockIdN);
//for(i = 0; i < bD->blockIdLen; i++){
//    printf("%d ", bD->blockId[i]);
//}
memcpy(bD->blockId, id->blockIds + id->blockIdOffset, sizeof(unsigned char)*bD->blockIdLen); id->blockIdOffset += sizeof(unsigned char)*bD->blockIdLen;
//printf("CHOK blockIdLen: %d, blockIdOffset: %d\n", bD->blockIdLen, id->blockIdOffset);
//printf("\nblockIdNK:");
//for(i = 0; i < bD->blockIdLen; i++){
  //  printf("%d ", bD->blockId[i]);
//}
//printf("\n");

for(i = 0; i < bD->blockIdLen; i++){
    //printf("NK CHK: finalBufOffset: %d, i: %d\n", finalBufOffset, i);
    blockType = bD->blockId[i]; tempTypesCount[blockType]++;
    if(tempTypesCount[blockType] == bD->blockRPTypesCount[blockType]){
        while(finalBufOffset + sizeof(unsigned char)*(bD->blockRPNCount[blockType]-(bD->blockRPTypesCount[blockType]-1)*D_BLOCK_SIZE) >= finalBufAllocSize){
            doubleUpChar(finalBuf, &finalBufAllocSize, finalBufOffset);
        }
        memcpy(finalBuf + finalBufOffset, tempBuf[blockType] + offset[blockType], sizeof(unsigned char)*(bD->blockRPNCount[blockType]-(bD->blockRPTypesCount[blockType]-1)*D_BLOCK_SIZE));
        finalBufOffset += sizeof(unsigned char)*(bD->blockRPNCount[blockType]-(bD->blockRPTypesCount[blockType]-1)*D_BLOCK_SIZE);
    } else {
        while(finalBufOffset + sizeof(unsigned char)*D_BLOCK_SIZE >= finalBufAllocSize){
            doubleUpChar(finalBuf, &finalBufAllocSize, finalBufOffset);
        }
        //printf("before memcpy to %d, finalBufAllocSize: %d, offset[%d]: %d\n", finalBufOffset, finalBufAllocSize, blockType, offset[blockType]);
        memcpy(finalBuf + finalBufOffset, tempBuf[blockType] + offset[blockType], sizeof(unsigned char)*D_BLOCK_SIZE);
        finalBufOffset += sizeof(unsigned char)*D_BLOCK_SIZE;
    }
    offset[blockType] += D_BLOCK_SIZE;
}


//printf("CHOK blockIdLen: %d, blockIdOffset: %d\n", bD->blockIdLen, id->blockIdOffset);
//int* testRP = (int*)malloc(sizeof(int)*finalBufOffset); memset(testRP, 0, sizeof(int)*finalBufOffset);
bD->allRP = (int*)malloc(sizeof(int)*finalBufAllocSize); memset(bD->allRP, 0, sizeof(int)*finalBufAllocSize);
//bD->allRP = (int*)malloc(sizeof(int)*(bD->blockCount)*(bD->blockSize)); memset(bD->allRP, 0, sizeof(int)*(bD->blockCount)*(bD->blockSize));
//printf("NKNKNK finalBufLen: %d, finalBufOffset: %d\n", finalBufLen, finalBufOffset);
//printf("CHOK blockIdLen: %d, blockIdOffset: %d\n", bD->blockIdLen, id->blockIdOffset);
int testRPCount = 0;
for(i = 0; i < finalBufOffset; i++){
    //printf("%d", finalBuf[i]);
    if(finalBuf[i] == 0){
        bD->allRP[testRPCount++] = i+1;
        //printf("%d - %d ", rp[testRPCount-1], testRP[testRPCount-1]);
        //if(rp[testRPCount-1] != testRP[testRPCount-1]){ printf(" verify failure...\n"); }
    }
}
//printf("\n");

/*for(i = 0; i < rCount; i++){
    //printf("%d - %d \n", rp[i], testRP[i]);
    if(rp[i] != testRP[i]){ printf("verify failure...\n"); }
}*/
//Free
for(i = 0; i < D_BLOCK_TYPES; i++){
    //if(tempBuf[i] != NULL && bD->blockRPNCount[i] != 0){ free(tempBuf[i]); }
    if(tempBuf[i] != NULL){ free(tempBuf[i]); }
}
if(finalBuf != NULL){ free(finalBuf); }
//if(testRP != NULL){ free(testRP); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to combine the density blocks - verify the blocks by combining them and comparing them with the original using only positions(fast) */
/* Input: */
/* Output: */
//void combineBlocks(int* rp, int rCount, BD* bD){
void combineBlocks(BD* bD, DD* id){
if(LOG){ debug(HIGH, 1, "\n(combineBlocks)Start allBlocksRPCount: %d\n", bD->allBlocksRPCount); }
int i = 0, j = 0;
//int* testRP = (int*)malloc(sizeof(int)*(bD->blockCount)*(bD->blockSize)); memset(testRP, 0, sizeof(int)*(bD->blockCount)*(bD->blockSize));
bD->allRP = (int*)malloc(sizeof(int)*(bD->blockCount)*(bD->blockSize)); memset(bD->allRP, 0, sizeof(int)*(bD->blockCount)*(bD->blockSize));
int testRPCount = 0, blockType = -1;
int tempTypesCount[bD->blockTypesCount]; memset(tempTypesCount, 0, sizeof(tempTypesCount));
int offset[bD->blockTypesCount]; memset(offset, 0, sizeof(offset));
int tempRunningCount = 0, tempRCount = 0;

bD->blockId = (unsigned char*)malloc(sizeof(unsigned char)*bD->blockIdLen); memset(bD->blockId, 0, sizeof(unsigned char)*bD->blockIdLen);
memcpy(bD->blockId, id->blockIds + id->blockIdOffset, sizeof(unsigned char)*bD->blockIdLen); id->blockIdOffset += sizeof(unsigned char)*bD->blockIdLen;
printf("NKBLOCKID: "); for(i = 0; i < bD->blockIdLen; i++){ printf("%d ", bD->blockId[i]); } printf("\n");
for(i = 0; i < bD->blockIdLen; i++){
    if(LOG){ debug(HIGH, 0, "\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"); }
    blockType = bD->blockId[i]; tempTypesCount[blockType]++; tempRunningCount++;
    if(LOG){ debug(HIGH, 1, "-->%d -- ", (tempTypesCount[blockType]-1)*bD->blockSize + 1); }
    if(tempRunningCount == bD->blockCount){
        while((bD->blockRP[blockType][offset[blockType]] > (tempTypesCount[blockType]-1)*bD->blockSize && bD->blockRP[blockType][offset[blockType]] <= (bD->blockRPNCount[blockType]-(bD->blockRPTypesCount[blockType]-1)*bD->blockSize)) && tempRCount < bD->allBlocksRPCount){
            tempRCount++;
            if(LOG){ debug(HIGH, 3, "(%d) (%d) (%d)", bD->blockRP[blockType][offset[blockType]], bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize, (tempRunningCount-1)*bD->blockSize + (bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize)); }
            bD->allRP[testRPCount++] = (tempRunningCount-1)*bD->blockSize + (bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize);
            //if(rp[testRPCount-1] != testRP[testRPCount-1]){ printf("%d - %d verify failure...\n", rp[testRPCount-1], testRP[testRPCount-1]); }
            offset[blockType]++;
        }
    } else {
        while(bD->blockRP[blockType][offset[blockType]] > (tempTypesCount[blockType]-1)*bD->blockSize && bD->blockRP[blockType][offset[blockType]] <= tempTypesCount[blockType]*bD->blockSize){
            tempRCount++;
            if(LOG){ debug(HIGH, 3, "(%d) (%d) (%d) ", bD->blockRP[blockType][offset[blockType]], bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize, (tempRunningCount-1)*bD->blockSize + (bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize)); }
            bD->allRP[testRPCount++] = (tempRunningCount-1)*bD->blockSize + (bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize);
            //if(rp[testRPCount-1] != testRP[testRPCount-1]){ printf("%d - %d verify failure...\n", rp[testRPCount-1], testRP[testRPCount-1]); }
            offset[blockType]++;
        }
    }
    if(LOG){ debug(HIGH, 2, " -- %d<-- %d ", tempTypesCount[blockType]*bD->blockSize, tempRunningCount*bD->blockSize); }
}

if(LOG){ 
    debug(HIGH, 0, "\nblockId:");
    for(i = 0; i < bD->blockIdLen; i++){ if(LOG){ debug(HIGH, 1, "%d ", bD->blockId[i]); } }
    debug(HIGH, 0, "\n");
    printf("RPNK: ");
    for(i = 0; i < testRPCount; i++){
        printf("%d ", bD->allRP[i]);
    }
}

//if(testRP != NULL){ free(testRP); }
if(LOG){ debug(HIGH, 1, "\n(combineBlocks)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to cleanup all the allocations */
/* Input: */
/* Output: */
void cleanupSplitDataDecompress(SD* sD){
if(LOG){ debug(HIGH, 0, "(cleanupSplitDataDecompress)Start\n"); }
if(sD->rpZero != NULL){ free(sD->rpZero); }
if(sD->rpOne != NULL){ free(sD->rpOne); }
if(sD->splitId != NULL){ free(sD->splitId); }
if(sD->rpSplitId != NULL){ free(sD->rpSplitId); }
if(sD != NULL){ free(sD); }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to decode the gmpBuf */
/* Input: */
/* Output: */
void extractEncValuesFromBufNew(DD* id){
if(LOG){ debug(VERY_HIGH, 1, "\n(extractEncValuesFromBuf)Start\n"); }

int i = 0, j = 0, k = 0, l = 0, p = 0, encValueLen = 0, nCount = 0, rCount = 0, firstValLen = 0, splitChar = FALSE;
int partialBytesBufOffset = 0, outBufOffset = 0, splitCharOffset = 0, nCountRandomBufOffset = 0, splitDensityIdBitsOffset = 0, encValueOffset = 0;
unsigned char curByte = 0;
unsigned int splitN = 0, splitR = 0, nChar = 0, rChar = 0;
for(i = 0; i < id->charDataCount; i++){
    if(LOG){ debug(VERY_HIGH, 3, "PROCESSING CHAR %d (n: %d, r:%d)\n", id->charData[i], id->nCountValues[i], id->rCountBuf[i]); }
    if(id->splitCharsCount != 0){
        for(j = 0; j < id->splitCharsCount; j++){ 
            splitChar = FALSE;
            if(id->charData[i] == id->splitChars[j]){
                //if(LOG){ debug(HIGH, 1, "char %d to be decoded with split\n", id->charData[i]); } 
                if(LOG){ debug(VERY_HIGH, 1, "char %d to be decoded with split\n", id->charData[i]); }
                SD* sD = (SD*)malloc(sizeof(SD)); sD->rpZero = NULL; sD->rpOne = NULL; sD->splitId = NULL; sD->rpSplitId = NULL;
                sD->rpOneCount = 0; sD->rpOneN = 0; sD->rpZeroCount = 0; sD->rpZeroN = 0; sD->splitIdCount = 0; sD->splitIdZeroCount = 0; sD->rpSplitIdCount = 0; sD->rpSplitIdN = 0;

                for(l = 0; l < 3; l++){
                    splitN = nCount = id->nCountRandomBuf[nCountRandomBufOffset++];
                    splitR = rCount = id->nCountRandomBuf[nCountRandomBufOffset++];
                    if(LOG){ debug(HIGH, 2, "splitN: %d, splitR: %d\n", splitN, splitR); }
                    switch(l){
                        case 0: sD->rpZeroCount = splitR; sD->rpZeroN = splitN;
                                break;
                        case 1: sD->rpOneCount = splitR; sD->rpOneN = splitN;
                                break;
                        case 2: sD->rpSplitIdCount = splitR; sD->rpSplitIdN = splitN;
                                break;
                        default: printf("FATAL... out of known options\n"); break;
                    }
                    if(splitN != 0 && splitR != 0 && splitN != splitR){
                        if(LOG){ debug(HIGH, 2, "split%d n: %d, r: %d  ", l+1, nCount, rCount); }
                        if(id->splitDensityIdBits[splitDensityIdBitsOffset++] == 1){
                            //if(LOG){ debug(HIGH, 1, "split%d for char %d to be decoded by density\n", l+1, id->charData[i]); } 
                            if(LOG){ debug(HIGH, 2, "split%d for char %d to be decoded by density\n", l+1, id->charData[i]); }
                            int* rp[D_BLOCK_TYPES];
                            BD* bD = NULL;
                            initializeBlockDataDecompress(&bD);
                            bD->curChar = id->charData[i];
                            for(k = 0; k < D_BLOCK_TYPES; k++){
                                rp[k] = NULL;
                                nCount = id->nCountRandomBuf[nCountRandomBufOffset++]; rCount = id->nCountRandomBuf[nCountRandomBufOffset++];
                                //if(LOG){ debug(HIGH, 2, "block%d of density n: %d, r: %d\n", k, nCount, rCount); }
                                if(LOG){ debug(HIGH, 3, "block%d of density n: %d, r: %d\n", k, nCount, rCount); }
                                if(nCount != 0 && rCount != 0 && nCount != rCount){
                                    encValueLen = id->encValLen[encValueOffset]; firstValLen = id->firstValDec[encValueOffset++];
                                    if(LOG){ debug(HIGH, 2, "Decode block%d of density with n: %d, r: %d, encValueLen: %d, firstValLen: %d\n", k, nCount+1, rCount+1, encValueLen, firstValLen); }
                                    unsigned char* encValue = (unsigned char*)malloc(sizeof(unsigned char)*encValueLen);
                                    memset(encValue, 0, sizeof(unsigned char)*encValueLen);
                                    memcpy(encValue, id->outBuf + outBufOffset, sizeof(unsigned char)*encValueLen);
                                    outBufOffset += encValueLen;
                                    bD->blockRPCount[k] = rCount;
                                    bD->blockRPNCount[k] = nCount;
                                    superMagicDecodePosToPos(encValue, encValueLen, id->charData[i], nCount+1, rCount+1, firstValLen, &bD->blockRP[k], id);
                                    if(encValue != NULL){ free(encValue); }
                                } else {
                                    bD->blockRPCount[k] = rCount;
                                    bD->blockRPNCount[k] = nCount;
                                    if(LOG){ debug(HIGH, 2, "CHOK Encoding not necessary for n: %d, r: %d\n", nCount, rCount); }
                                    if(nCount == rCount){
                                        bD->blockRP[k] = (int*)malloc(sizeof(int)*rCount); memset(bD->blockRP[k], 0, sizeof(int)*rCount);
                                        int r = 0;
                                        for(r = 0; r < rCount; r++){
                                            bD->blockRP[k][r] = r+1;
                                        }
                                    }
                                }
                                //int q = 0;
                                //if(id->charData[i] == 0){ printf("BLOCKTYPE%d: ", k); for(q = 0; q < bD->blockRPCount[k]; q++){ printf("%d ", bD->blockRP[k][q]); } printf(" (%d)\n", bD->blockRPNCount[k]); }
                                if(rp[k] != NULL){ free(rp[k]); }
                            }
                            //for(k = 0; k < D_BLOCK_TYPES; k++){
                            //    if(LOG){ debug(HIGH, 3, "NEERAJ: block%d with n: %d, r: %d\n", k, bD->blockRPNCount[k], bD->blockRPCount[k]);  }
                            //}
                            bD->blockCount = (int)ceil((float)splitN/(float)bD->blockSize);
                            bD->blockIdLen = bD->blockCount;
                            bD->allBlocksRPCount = splitR;
                            //combineBlocks(bD, id);
                            combineBlocksSlow(bD, id);
                            switch(l){
                                case 0: sD->rpZero = (int*)malloc(sizeof(int)*bD->allBlocksRPCount); memset(sD->rpZero, 0, sizeof(int)*bD->allBlocksRPCount); 
                                        memcpy(sD->rpZero, bD->allRP, sizeof(int)*bD->allBlocksRPCount); sD->rpZeroCount = bD->allBlocksRPCount; sD->rpZeroN = splitN;
                                        break;
                                case 1: sD->rpOne = (int*)malloc(sizeof(int)*bD->allBlocksRPCount); memset(sD->rpOne, 0, sizeof(int)*bD->allBlocksRPCount); 
                                        memcpy(sD->rpOne, bD->allRP, sizeof(int)*bD->allBlocksRPCount); sD->rpOneCount = bD->allBlocksRPCount; sD->rpOneN = splitN;
                                        break;
                                case 2: sD->rpSplitId = (int*)malloc(sizeof(int)*bD->allBlocksRPCount); memset(sD->rpSplitId, 0, sizeof(int)*bD->allBlocksRPCount); 
                                        memcpy(sD->rpSplitId, bD->allRP, sizeof(int)*bD->allBlocksRPCount); sD->rpSplitIdCount = bD->allBlocksRPCount; sD->rpSplitIdN = splitN;
                                        break;
                                default: printf("FATAL... out of known options\n"); break;
                            }
                            cleanupBlockDataDecompress(bD);
                        } else {
                            int* rp = NULL;
                            //if(LOG){ debug(HIGH, 1, "split%d for char %d to be decoded without density\n", l+1, id->charData[i]); } 
                            if(LOG){ debug(HIGH, 2, "split%d for char %d to be decoded without density\n", l+1, id->charData[i]); }
                            encValueLen = id->encValLen[encValueOffset]; firstValLen = id->firstValDec[encValueOffset++];
                            if(LOG){ debug(HIGH, 2, "Decode split%d with n: %d, r: %d, encValueLen: %d, firstValLen: %d\n", l+1, nCount+1, rCount+1, encValueLen, firstValLen); }
                            unsigned char* encValue = (unsigned char*)malloc(sizeof(unsigned char)*encValueLen);
                            memset(encValue, 0, sizeof(unsigned char)*encValueLen);
                            memcpy(encValue, id->outBuf + outBufOffset, sizeof(unsigned char)*encValueLen);
                            outBufOffset += encValueLen;
                            superMagicDecodePosToPos(encValue, encValueLen, id->charData[i], nCount+1, rCount+1, firstValLen, &rp, id);
                            if(encValue != NULL){ free(encValue); }
                            switch(l){
                                case 0: sD->rpZero = (int*)malloc(sizeof(int)*rCount); memset(sD->rpZero, 0, sizeof(int)*rCount); 
                                        memcpy(sD->rpZero, rp, sizeof(int)*rCount); sD->rpZeroCount = rCount; sD->rpZeroN = nCount;
                                        break;
                                case 1: sD->rpOne = (int*)malloc(sizeof(int)*rCount); memset(sD->rpOne, 0, sizeof(int)*rCount); 
                                        memcpy(sD->rpOne, rp, sizeof(int)*rCount); sD->rpOneCount = rCount; sD->rpOneN = nCount;
                                        break;
                                case 2: sD->rpSplitId = (int*)malloc(sizeof(int)*rCount); memset(sD->rpSplitId, 0, sizeof(int)*rCount); 
                                        memcpy(sD->rpSplitId, rp, sizeof(int)*rCount); sD->rpSplitIdCount = rCount; sD->rpSplitIdN = nCount;
                                        break;
                                default: printf("FATAL... out of known options\n"); break;
                            }
                            if(rp != NULL){ free(rp); }
                        }
                    }
                }
                if(sD->rpZeroCount == sD->rpZeroN){
                    sD->rpZero = (int*)malloc(sizeof(int)*sD->rpZeroCount); memset(sD->rpZero, 0, sizeof(int)*sD->rpZeroCount); 
                    int r = 0;
                    for(r = 0; r < sD->rpZeroCount; r++){
                        sD->rpZero[r] = r+1;
                    }
                }
                if(sD->rpOneCount == sD->rpOneN){
                    sD->rpOne = (int*)malloc(sizeof(int)*sD->rpOneCount); memset(sD->rpOne, 0, sizeof(int)*sD->rpOneCount); 
                    int r = 0;
                    for(r = 0; r < sD->rpOneCount; r++){
                        sD->rpOne[r] = r+1;
                    }
                }
                if(sD->rpSplitIdCount != 0 && sD->rpSplitIdCount != sD->rpSplitIdN){
                    sD->splitId = (unsigned char*)malloc(sizeof(unsigned char)*sD->rpSplitIdN); memset(sD->splitId, 1, sizeof(unsigned char)*sD->rpSplitIdN);
                    for(p = 0; p < sD->rpSplitIdCount; p++){
                        sD->splitId[sD->rpSplitId[p]-1] = 0;
                    }    
                    sD->splitIdCount = sD->rpSplitIdN; sD->splitIdZeroCount = sD->rpSplitIdCount;
                } else {
                    if(sD->rpSplitIdCount == 0){ //all are ones
                        printf("rpSplitIdN: %d\n", sD->rpSplitIdN);
                        sD->splitId = (unsigned char*)malloc(sizeof(unsigned char)*sD->rpSplitIdN); memset(sD->splitId, 1, sizeof(unsigned char)*sD->rpSplitIdN);
                        sD->splitIdCount = sD->rpSplitIdN;
                        //sD->splitIdZeroCount = sD->rpSplitIdCount;
                        sD->splitIdZeroCount = 0;
                    }
                    if(sD->rpSplitIdCount == sD->rpSplitIdN){ //all are zeros
                        sD->splitId = (unsigned char*)malloc(sizeof(unsigned char)*sD->rpSplitIdN); memset(sD->splitId, 0, sizeof(unsigned char)*sD->rpSplitIdN);
                        sD->splitIdCount = sD->rpSplitIdN;
                        sD->splitIdZeroCount = sD->rpSplitIdCount;
                    }
                }
                combineSplit(sD);
                splitChar = TRUE;
                nChar = sD->rpZeroN + sD->rpOneN + sD->rpSplitIdN;
                rChar = sD->rpZeroCount + sD->rpOneCount + sD->rpSplitIdCount;
                id->rp[id->charData[i]] = (unsigned int*)malloc(sizeof(unsigned int)*id->rCountBuf[i]); memset(id->rp[id->charData[i]], 0, sizeof(unsigned int)*id->rCountBuf[i]);
                memcpy(id->rp[id->charData[i]], sD->allRP, sizeof(unsigned int)*id->rCountBuf[i]);
                id->rpCount[id->charData[i]] = id->rCountBuf[i];
                cleanupSplitDataDecompress(sD);
                break;
            }
        }
        if(splitChar == FALSE){
            nCount = id->nCountValues[i]; rCount = id->rCountBuf[i];
            //printf("nCount: %d, rCount: %d\n", nCount, rCount);
            if(nCount != 0 && rCount != 0 && nCount != rCount){
                encValueLen = id->encValLen[encValueOffset]; firstValLen = id->firstValDec[encValueOffset++];
                if(LOG){ debug(HIGH, 1, "(extractEncValuesFromBuf)For character %d, nCount: %d and rCount: %d, encValueLen: %d\n", id->charData[i], nCount+1, rCount+1, encValueLen); }
                unsigned char* encValue = (unsigned char*)malloc(sizeof(unsigned char)*encValueLen);
                memset(encValue, 0, sizeof(unsigned char)*encValueLen);
                memcpy(encValue, id->outBuf + outBufOffset, sizeof(unsigned char)*encValueLen);
                outBufOffset += encValueLen;
                superMagicDecodePos(encValue, encValueLen, id->charData[i], nCount+1, rCount+1, firstValLen, id);
                if(encValue != NULL){ free(encValue); }
            } else { 
                if(LOG){ debug(HIGH, 2, "CHOK Encoding not necessary for n: %d, r: %d\n", nCount, rCount); }
                if(nCount == rCount){
                    id->rpCount[id->charData[i]] = rCount;
                    id->rp[id->charData[i]] = (unsigned int*)malloc(sizeof(unsigned int)*rCount); memset(id->rp[id->charData[i]], 0, sizeof(unsigned int)*rCount);
                    int r = 0;
                    for(r = 0; r < rCount; r++){
                        id->rp[id->charData[i]][r] = r+1;
                    }
                    //printf("NNN ");
                    //for(r = 0; r < rCount; r++){
                    //    printf("%d ", id->rp[id->charData[i]][r]);
                    //} printf("\n");
                }
            }
            nChar = nCount; rChar = rCount;
        }
    } else { // No split chars at all and thus no density, only vajra encodings
        nCount = id->nCountValues[i]; rCount = id->rCountBuf[i];
        if(nCount != 0 && rCount != 0 && nCount != rCount){
            encValueLen = id->encValLen[encValueOffset]; firstValLen = id->firstValDec[encValueOffset++];
            if(LOG){ debug(HIGH, 1, "(extractEncValuesFromBuf)For character %d, nCount: %d and rCount: %d, encValueLen: %d\n", id->charData[i], nCount+1, rCount+1, encValueLen); }
            unsigned char* encValue = (unsigned char*)malloc(sizeof(unsigned char)*encValueLen);
            memset(encValue, 0, sizeof(unsigned char)*encValueLen);
            memcpy(encValue, id->outBuf + outBufOffset, sizeof(unsigned char)*encValueLen);
            outBufOffset += encValueLen;
            superMagicDecodePos(encValue, encValueLen, id->charData[i], nCount+1, rCount+1, firstValLen, id);
            if(encValue != NULL){ free(encValue); }
        } else { 
            if(LOG){ debug(HIGH, 2, "CHOK Encoding not necessary for n: %d, r: %d\n", nCount, rCount); }
            if(nCount == rCount){
                id->rpCount[id->charData[i]] = rCount;
                id->rp[id->charData[i]] = (unsigned int*)malloc(sizeof(unsigned int)*rCount); memset(id->rp[id->charData[i]], 0, sizeof(unsigned int)*rCount);
                int r = 0;
                for(r = 0; r < rCount; r++){
                    id->rp[id->charData[i]][r] = r+1;
                }
                //printf("NNN ");
                //for(r = 0; r < rCount; r++){
                //    printf("%d ", id->rp[id->charData[i]][r]);
                //} printf("\n");
            }
        }
        nChar = nCount; rChar = rCount;
    }
    /*
    int z = 0;
    printf("RRPP(%d)(n:%d)(r:%d) - ", id->charData[i], nChar, rChar);
    for(z = 0; z < rChar; z++){
        printf("%d ", id->rp[id->charData[i]][z]);
    }
    printf("\n");
    */
    /*
    unsigned char* encValue = (unsigned char*)malloc(sizeof(unsigned char)*encValueLen);
    memset(encValue, 0, sizeof(unsigned char)*encValueLen);
    memcpy(encValue, id->outBuf + outBufOffset, sizeof(unsigned char)*encValueLen);
    outBufOffset += encValueLen;

    printf("first byte extract : %u\n", encValue[0]); 

    superMagicDecodePos(encValue, encValueLen, id->charData[i], nCount, rCount, firstValLen, id);
    id->nCountValues[i] -= 1;

    if(LOG){ debug(id->logLevel, 0, "\n"); }
    if(encValue != NULL){ free(encValue); }
    */
}
if(LOG){ debug(HIGH, 1, "\n(extractEncValuesFromBuf)End\n"); }
//if(TL){ printf("tG: %lld\n", id->tG); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to decode the gmpBuf */
/* Input: */
/* Output: */
void extractEncValuesFromBuf(DD* id){
if(LOG){ debug(HIGH, 1, "\n(extractEncValuesFromBuf)Start\n"); }

int i = 0, j = 0, encValueLen = 0, nCount = 0, rCount = 0, firstValLen = 0;
int partialBytesBufOffset = 0, outBufOffset = 0;
unsigned char curByte = 0;
for(i = 0; i < id->charDataCount; i++){
    nCount = id->nCountValues[i] + 1; rCount = id->rCountBuf[i] + 1;
    encValueLen = id->encValLen[i];
    firstValLen = id->firstValDec[i];
    
    if(LOG){ debug(HIGH, 1, "(extractEncValuesFromBuf)For character %d, nCount: %d and rCount: %d, encValueLen: %d\n", id->charData[i], nCount, rCount, encValueLen); }

    unsigned char* encValue = (unsigned char*)malloc(sizeof(unsigned char)*encValueLen);
    memset(encValue, 0, sizeof(unsigned char)*encValueLen);
    memcpy(encValue, id->outBuf + outBufOffset, sizeof(unsigned char)*encValueLen);
    outBufOffset += encValueLen;

    printf("first byte extract : %u\n", encValue[0]); 

    superMagicDecodePos(encValue, encValueLen, id->charData[i], nCount, rCount, firstValLen, id);
    id->nCountValues[i] -= 1;

    if(LOG){ debug(id->logLevel, 0, "\n"); }
    if(encValue != NULL){ free(encValue); }
}
if(LOG){ debug(HIGH, 1, "\n(extractEncValuesFromBuf)End\n"); }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to decompress the blockIds metadata
/* Input: */
/* Output: */
void decompressBlockIdBuf(DD* id){
if(LOG){ debug(HIGH, 1, "\n(decompressBlockIdBuf)Start\n"); }
int i = 0;
//printf("blockIdCompressMode: %d\n", id->blockIdCompressMode);
if(id->blockIdCompressMode == EXPANDED_WITH_MTF){
    unsigned short blockIdBufOffset = 0, encValueLen = 0;
    unsigned char firstValLen = 0;
    memcpy(&id->blockIdN, id->blockIdBuf + blockIdBufOffset, sizeof(unsigned int)); blockIdBufOffset += sizeof(unsigned int);
    if(LOG){ debug(HIGH, 1, "blockIdN: %d\n", id->blockIdN); }
    memcpy(&id->blockIdCount, id->blockIdBuf + blockIdBufOffset, sizeof(unsigned int)); blockIdBufOffset += sizeof(unsigned int);
    if(LOG){ debug(HIGH, 1, "blockIdCount: %d\n", id->blockIdCount); }
    memcpy(&encValueLen, id->blockIdBuf + blockIdBufOffset, sizeof(short)); blockIdBufOffset += sizeof(short);
    if(LOG){ debug(HIGH, 1, "encValueLen: %d\n", encValueLen); }
    memcpy(&firstValLen, id->blockIdBuf + blockIdBufOffset, sizeof(unsigned char)); blockIdBufOffset += sizeof(unsigned char);
    if(LOG){ debug(HIGH, 1, "firstValLen: %d\n", firstValLen); }
    unsigned char* encValue = (unsigned char*)malloc(sizeof(unsigned char)*encValueLen); memset(encValue, 0, sizeof(unsigned char)*encValueLen);
    memcpy(encValue, id->blockIdBuf + blockIdBufOffset, sizeof(char)*encValueLen); blockIdBufOffset += sizeof(char)*encValueLen;
    //printf("blockIdN: %d, blockIdCount: %d, encValueLen: %d, firstValLen: %d\n", id->blockIdN, id->blockIdCount, encValueLen, firstValLen);
    int* rp = NULL;
    superMagicDecodePosToPos(encValue, encValueLen, 0, id->blockIdN+1, id->blockIdCount+1, firstValLen, &rp, id);

    //Apply rmtf
    unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char)*id->blockIdCount); memset(temp, 0, sizeof(unsigned char)*id->blockIdCount);
    unsigned char* tempRMtf = (unsigned char*)malloc(sizeof(unsigned char)*id->blockIdCount); memset(tempRMtf, 0, sizeof(unsigned char)*id->blockIdCount);
    temp[0] = rp[0]-1;
    for(i = 1; i < id->blockIdCount; i++){
        temp[i] = rp[i] - rp[i-1] - 1;
    }
    //printf("decompressed temp before rmtf blockIds: ");
    //for(i = 0; i < id->blockIdCount; i++){
    //    printf("%d ", temp[i]);
    //}
    normalRMtf(temp, id->blockIdCount, &id->blockIds);
    //printf("decompressed after rmtf blockIds: ");
    //for(i = 0; i < id->blockIdCount; i++){
    //    printf("%d ", id->blockIds[i]);
    //}

    if(temp != NULL){ free(temp); }
    if(rp != NULL){ free(rp); }
    if(encValue != NULL){ free(encValue); }
}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to combine relative positions of all the characters
/* Input: */
/* Output: */
void combineRelativePositionsSmart(DD* id){
if(LOG){ debug(HIGH, 1, "\n(combineRelativePositionsSmart)Start\n"); }
int i = 0, j = 0, k = 0, curChar = -1;
unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char)*id->inCharCount); memset(data, 0, sizeof(unsigned char)*id->inCharCount);
int dataLen = 0, curPos = 0, startPos = 1, prevStartPos = 1, prevPos = 1, runningPos = 1, nextLink = 0;
unsigned int* dataLinks = (unsigned int*)malloc(sizeof(unsigned int)*(id->inCharCount+1)); memset(dataLinks, 0, sizeof(unsigned int)*(id->inCharCount+1));
for(i = id->charDataCount-1; i >= 0; i--){
    curChar = id->charData[i]; prevPos = 1;
    printf("char: %d, rCount: %d (startPos: %d)  ", curChar, id->rpCount[curChar], startPos);
    for(j = 0; j < id->rpCount[curChar]; j++){ 
        curPos = id->rp[curChar][j];
        data[dataLen] = curChar;
        if(j == 0 && curPos == 1){
            printf("%d (%d-%d)", curPos, prevPos, curPos-1); 
            prevStartPos = startPos;
            startPos = dataLen+1;
            printf("startPos: %d # ", startPos); 
            if(startPos == 1){ //first element
                dataLinks[startPos] = 0;
            } else {
                dataLinks[startPos] = prevStartPos;
                runningPos = prevStartPos;
            }
        } else {
            printf("%d (%d-%d)", curPos, prevPos, curPos-1); 
            nextLink = startPos;
            for(k = prevPos; k < curPos-1; k++){
                nextLink = dataLinks[nextLink];
            }
            dataLinks[nextLink] = dataLen+1;
            printf("(%d-%d) ", curPos, dataLen+1);
            dataLinks[dataLinks[nextLink]] = nextLink;
            //printf("(%d-%d) ", dataLinks );
        }
        prevPos = curPos+1;
        dataLen++;
    }
    printf("\n");
}

printf("\n"); for(i = 0; i < dataLen; i++){ printf("%d ", data[i]); } printf("\n");

if(data != NULL){ free(data); }
if(dataLinks != NULL){ free(dataLinks); }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to combine relative positions of all the characters
/* Input: */
/* Output: */
void combineRelativePositionsFast(DD* id){ //Incomplete
if(LOG){ debug(HIGH, 1, "\n(combineRelativePositionsFast)Start\n"); }
int i = 0, j = 0, k = 0, curChar = -1;
unsigned int* tempBuf = (unsigned int*)malloc(sizeof(unsigned int)*id->inCharCount); for(i = 0; i < id->inCharCount; i++){ tempBuf[i] = i+1; }
id->decCharData = (unsigned char*)malloc(sizeof(unsigned char)*id->inCharCount); memset(id->decCharData, 0, sizeof(unsigned char)*id->inCharCount);
//unsigned char* finalBuf = (unsigned char*)malloc(sizeof(unsigned char)*id->inCharCount); memset(finalBuf, 0, sizeof(unsigned char)*id->inCharCount);
unsigned int tempBufLen = id->inCharCount, newCounter = 0;
for(i = 0; i < id->charDataCount; i++){
    curChar = id->charData[i]; newCounter = 0;
    for(j = 0; j < id->rpCount[curChar]; j++){
        id->decCharData[tempBuf[id->rp[curChar][j]-1]] = curChar;
        //finalBuf[tempBuf[id->rp[curChar][j]-1]] = curChar;
        if(j == 0){
            if(id->rp[curChar][j] > 1){
                newCounter = id->rp[curChar][j]-1; //tempBufLen = id->rp[curChar][j]-1;
            }
        } else {
            for(k = 1; k <= id->rp[curChar][j]-id->rp[curChar][j-1]-1; k++){
                tempBuf[newCounter++] = tempBuf[id->rp[curChar][j-1]-1+k]; //tempBufLen++;
            }
        }
    }
    if(id->rp[curChar][id->rpCount[curChar]-1] < tempBufLen){
        for(k = 1; k <= tempBufLen - id->rp[curChar][id->rpCount[curChar]-1]; k++){
            tempBuf[newCounter++] = tempBuf[id->rp[curChar][id->rpCount[curChar]-1]-1+k];
        }
    }
    tempBufLen = newCounter;
    printf("tempBufLen after %d char is %d\n", curChar, tempBufLen);
}
id->decDataLen = id->inCharCount;
if(tempBuf != NULL){ free(tempBuf); }
//if(finalBuf != NULL){ free(finalBuf); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to combine relative positions of all the characters
/* Input: */
/* Output: */
void combineRelativePositions(DD* id){
if(LOG){ debug(HIGH, 1, "\n(combineRelativePositions)Start\n"); }
register int i = 0, j = 0, tempCount = 0, rpCount = 0;
//Display all the positions
/*
for(i = 0; i < id->charDataCount; i++){
    printf("KNKNrp[%d](%d): ", id->charData[i], id->rpCount[id->charData[i]]);
    for(j = 0; j < id->rpCount[id->charData[i]]; j++){
        printf("%d ", id->rp[id->charData[i]][j]);
    }
    printf(" (%d)\n", id->nCountValues[i]);
}
*/
register unsigned char curChar = 0;
id->decData = (unsigned short*)malloc(sizeof(unsigned short)*id->inCharCount); //memset(id->decData, 0, sizeof(unsigned short)*id->inCharCount);
id->decCharData = (unsigned char*)malloc(sizeof(unsigned char)*id->inCharCount); memset(id->decCharData, 0, sizeof(unsigned char)*id->inCharCount);
for(i = 0; i < id->inCharCount; i++){ id->decData[i] = 256; }
for(i = 0; i < id->charDataCount; i++){
    curChar = id->charData[i];
    tempCount = 0; rpCount = 0;
    //for(j = 0; j < id->nCountValues[i]; j++){
    for(j = 0; j < id->inCharCount && rpCount < id->rpCount[id->charData[i]]; j++){
        if(id->decData[j] == 256){
            tempCount++;
        }
        if(tempCount == id->rp[curChar][rpCount]){
            id->decData[j] = curChar;
            rpCount++;
        }
    }
    //if(LOG){ debug(id->logLevel, 0, "\n"); }
    //if(LOG){ for(j = 0; j < id->inCharCount; j++){ if(LOG){ debug(id->logLevel, 1, "(%d)%d ", j+1, id->decData[j]); } } }
    //if(LOG){ debug(id->logLevel, 0, "\n"); }
}

//id->decFile = (FILE*)openFile(id->logLevel, "outDec.txt", "wb");
//if(LOG){ debug(HIGH, 1, "File opened for writing decoded data\n"); }
for(i = 0; i < id->inCharCount; i++){
    //fputc((unsigned char)id->decData[i], id->decFile);
    id->decCharData[i] = (unsigned char)id->decData[i];
    (id->decDataLen)++;
}
//fwrite(id->decData, sizeof(unsigned char), id->inCharCount, id->decFile);
//id->decDataLen = id->inCharCount;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to cleanup all the allocations */
/* Input: */
/* Output: */
void decCleanup(DD* id){
if(LOG){ debug(HIGH, 1, "\n(decCleanup)Start\n"); }
int i = 0;
for(i = 0; i < MAX_CHARS; i++){
    if(id->rp[i] != NULL && id->rpCount[i] != 0){ free(id->rp[i]); if(LOG){ debug(LOW, 1, "(cleanup)After freeing rp[%d]\n", i); } }
}
if(id->charData != NULL){ free(id->charData); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->charData\n"); } }
if(id->nCountBytes != NULL){ free(id->nCountBytes); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->nCountBytes\n"); } }
if(id->blockIdBuf != NULL){ free(id->blockIdBuf); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->blockIdBuf\n"); } }
if(id->blockIds != NULL){ free(id->blockIds); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->blockIds\n"); } }
if(id->nCountValues != NULL){ free(id->nCountValues); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->nCountValues\n"); } }
if(id->rCountBuf != NULL){ free(id->rCountBuf); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->rCountBuf\n"); } }
if(id->nCountRandomBuf != NULL){ free(id->nCountRandomBuf); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->nCountRandomBuf\n"); } }
if(id->splitChars != NULL){ free(id->splitChars); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->splitChars\n"); } }
if(id->splitDensityIdBits != NULL){ free(id->splitDensityIdBits); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->splitDensityIdBits\n"); } }
if(id->splitDensityIdBytes != NULL){ free(id->splitDensityIdBytes); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->splitDensityIdBytes\n"); } }
if(id->partialBytes != NULL){ free(id->partialBytes); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->partialBytes\n"); } }
if(id->partialBytesBuf != NULL){ free(id->partialBytesBuf); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->partialBytesBuf\n"); } }
if(id->outBuf != NULL){ free(id->outBuf); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->outBuf\n"); } }
//if(id->decData != NULL){ free(id->decData); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->decData\n"); } }
if(id->firstValDec != NULL){ free(id->firstValDec); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->firstValDec\n"); } }
if(id->encValLen != NULL){ free(id->encValLen); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id->encValLen\n"); } }
//fclose(id->decFile);
//if(id != NULL){ free(id); if(LOG){ debug(LOW, 0, "(cleanup)After freeing id\n"); } }
}

void initializeDecodeData(DD* dd){
dd->inCharCount = 0;
dd->charData = NULL; dd->charDataCount = 0;
dd->outBuf = NULL; dd->outBufLen = 0;
dd->blockIdCompressMode = 0;
dd->blockIdBuf = NULL; dd->blockIdBufLen = 0;
dd->blockIds = NULL; dd->blockIdCount = 0; dd->blockIdN = 0; dd->blockIdOffset = 0;
dd->partialBytesBuf = NULL; dd->partialBytesBufLen = 0;
dd->partialBytes = NULL;
dd->emptyBytesCount = 0;
dd->distinctCharCount = 0;
dd->nCountBuf = NULL; dd->nCountBufLen = 0;
dd->nCountBytes = NULL;
dd->densityEncodedBufCount = 0;
dd->nCountRandomBuf = NULL; dd->nCountRandomLen = 0;
dd->nCountValues = NULL;
dd->rCountBuf = NULL; dd->rCountBufLen = 0;
dd->splitChars = NULL; dd->splitCharsCount = 0;
dd->splitDensityIdBits = NULL; dd->splitDensityIdBitsCount = 0;
dd->splitDensityIdBytes = NULL;
int i = 0;
for(i = 0; i < MAX_CHARS; i++){
    dd->rp[i] = NULL;
    dd->rpCount[i] = 0;
}
dd->decData = NULL; dd->decDataLen = 0;
//dd->filename = NULL; dd->filenameLength = 0;
dd->firstValDec = NULL; dd->firstValDecLen = 0;
dd->encValLen = NULL; dd->encValCount = 0;
dd->tG = 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to decode a character buffer by vajra */
/* Input: */
/* Output: */
void decodeBuffer(DD* id){
if(LOG){ debug(HIGH, 1, "\n(decodeBuffer)len: %d\n", id->inBufLen); }

int i = 0, inBufOffset = 2;
//Initialize the inData structure
//id = (DD*)malloc(sizeof(DD));
initializeDecodeData(id);

//Read the total number of characters which were encoded
memcpy(&id->inCharCount, id->inBuf + inBufOffset, sizeof(unsigned int)); inBufOffset += sizeof(unsigned int);
if(LOG){ debug(id->logLevel, 1, "inCharCount: %d\n", id->inCharCount); }

//Read the total number of distinct characters which were encoded
memcpy(&id->charDataCount, id->inBuf + inBufOffset, sizeof(unsigned short)); inBufOffset += sizeof(unsigned short);
if(LOG){ debug(id->logLevel, 1, "charDataCount: %d\n", id->charDataCount); }

//Read the distinct characters which were encoded
id->charData = (unsigned char*)malloc(sizeof(unsigned char)*id->charDataCount); memset(id->charData, 0, sizeof(unsigned char)*id->charDataCount);
memcpy(id->charData, id->inBuf + inBufOffset, sizeof(unsigned char)*id->charDataCount); inBufOffset += sizeof(unsigned char)*id->charDataCount;
if(LOG){ debug(id->logLevel, 1, "Read %d distinct characters from %d bytes\n", id->charDataCount, sizeof(unsigned char)*id->charDataCount); }
if(LOG){ displayCharData(id); }

//Read the nCountBufLen for nCount
memcpy(&id->nCountBufLen, id->inBuf + inBufOffset, sizeof(unsigned short)); inBufOffset += sizeof(unsigned short);
if(LOG){ debug(id->logLevel, 1, "\nRead nCountBufLen %d using %d bytes\n", id->nCountBufLen, sizeof(unsigned short)); }

//Read the nCountBytes for nCount
id->nCountBytes = (unsigned char*)malloc(sizeof(unsigned char)*id->nCountBufLen); memset(id->nCountBytes, 0, sizeof(unsigned char)*id->nCountBufLen);
memcpy(id->nCountBytes, id->inBuf + inBufOffset, sizeof(unsigned char)*(int)ceil((float)id->nCountBufLen/(float)8)); inBufOffset += sizeof(unsigned char)*(int)ceil((float)id->nCountBufLen/(float)8);
if(LOG){ debug(id->logLevel, 1, "Read nCountBytes using %d bytes\n", sizeof(unsigned char)*(int)ceil((float)id->nCountBufLen/(float)8)); }

//Extract nCount values from nCountBytes
extractNCountFromBuf(id->nCountBytes, id->nCountBufLen, id->inCharCount, id);
if(LOG){ displayNCountValues(id); }


//Read the rCountBuf for rCount
id->rCountBufLen = id->charDataCount;
id->rCountBuf = (unsigned int*)malloc(sizeof(unsigned int)*id->rCountBufLen); memset(id->rCountBuf, 0, sizeof(unsigned int)*id->rCountBufLen);
memcpy(id->rCountBuf, id->inBuf + inBufOffset, sizeof(unsigned int)*id->rCountBufLen); inBufOffset += sizeof(unsigned int)*id->rCountBufLen;
if(LOG){ debug(id->logLevel, 1, "Read rCountBuf using %d bytes\n", sizeof(unsigned int)*id->rCountBufLen); }
if(LOG){ displayRCountValues(id); }

//Read the number of chars encoded by split
memcpy(&id->splitCharsCount, id->inBuf + inBufOffset, sizeof(unsigned char)); inBufOffset += sizeof(unsigned char);
if(LOG){ debug(id->logLevel, 1, "Read the number of chars encoded by split as %d using %d bytes\n", id->splitCharsCount, sizeof(unsigned char)); }

//Read the chars encoded by split
id->splitChars = (unsigned char*)malloc(sizeof(unsigned char)*id->splitCharsCount); memset(id->splitChars, 0, sizeof(unsigned char)*id->splitCharsCount);
memcpy(id->splitChars, id->inBuf + inBufOffset, sizeof(unsigned char)*id->splitCharsCount); inBufOffset += sizeof(unsigned char)*id->splitCharsCount;
if(LOG){ debug(id->logLevel, 1, "Read splitChars using %d bytes\n", sizeof(unsigned char)*id->splitCharsCount); }
if(LOG){ displaySplitChars(id); }

//Read the splitDensityIdBytesLen
memcpy(&id->splitDensityIdBytesLen, id->inBuf + inBufOffset, sizeof(unsigned char)); inBufOffset += sizeof(unsigned char);
if(LOG){ debug(id->logLevel, 1, "Read splitDensityIdBytesLen as %d using %d bytes\n", id->splitDensityIdBytesLen, sizeof(unsigned char)); }

//Read the splitDensityIdBitsCount
memcpy(&id->splitDensityIdBitsCount, id->inBuf + inBufOffset, sizeof(unsigned short)); inBufOffset += sizeof(unsigned short);
if(LOG){ debug(id->logLevel, 1, "Read splitDensityIdBitsCount as %d using %d bytes\n", id->splitDensityIdBitsCount, sizeof(unsigned short)); }

//Read the chars encoded by split
id->splitDensityIdBytes = (unsigned char*)malloc(sizeof(unsigned char)*id->splitDensityIdBytesLen);
memset(id->splitDensityIdBytes, 0, sizeof(unsigned char)*id->splitDensityIdBytesLen);
memcpy(id->splitDensityIdBytes, id->inBuf + inBufOffset, sizeof(unsigned char)*id->splitDensityIdBytesLen); 
inBufOffset += sizeof(unsigned char)*id->splitDensityIdBytesLen;
if(LOG){ debug(id->logLevel, 1, "Read splitDensityIdBytes using %d bytes\n", sizeof(unsigned char)*id->splitDensityIdBytesLen); }
extractSplitDensityIdBits(id);
//if(LOG){ displaySplitChars(id); }

//Read the number of density encoded bufs
memcpy(&id->densityEncodedBufCount, id->inBuf + inBufOffset, sizeof(unsigned char)); inBufOffset += sizeof(unsigned char);
if(LOG){ debug(id->logLevel, 1, "Read the number of density encoded bufs as %d using %d bytes\n", id->densityEncodedBufCount, sizeof(unsigned char)); }

//Read the nCountRandomLen
memcpy(&id->nCountRandomLen, id->inBuf + inBufOffset, sizeof(unsigned short)); inBufOffset += sizeof(unsigned short);
if(LOG){ debug(id->logLevel, 1, "Read the nCountRandomLen of density encoded bufs as %d using %d bytes\n", id->nCountRandomLen, sizeof(unsigned short)); }

//Read the nCountRandomBuf of density encoded bufs
//if(id->densityEncodedBufCount != 0){
//int tTemp = id->splitCharsCount*3*2+id->densityEncodedBufCount*D_BLOCK_TYPES*2;
id->nCountRandomBuf = (unsigned int*)malloc(sizeof(unsigned int)*(id->nCountRandomLen)); memset(id->nCountRandomBuf, 0, sizeof(unsigned int)*(id->nCountRandomLen));
memcpy(id->nCountRandomBuf, id->inBuf + inBufOffset, sizeof(unsigned int)*(id->nCountRandomLen)); inBufOffset += sizeof(unsigned int)*(id->nCountRandomLen);
if(LOG){ debug(id->logLevel, 1, "Read nCountRandomBuf using %d bytes\n", sizeof(unsigned int)*id->nCountRandomLen); }
//id->nCountRandomLen = tTemp;
if(LOG){ displayNCountRandomValues(id); }
//}

//if(id->densityEncodedBufCount != 0){
//Read the mode for blockIds compression
memcpy(&id->blockIdCompressMode, id->inBuf + inBufOffset, sizeof(unsigned char)); inBufOffset += sizeof(unsigned char);
if(LOG){ debug(id->logLevel, 1, "Read the mode of blockIds compression as %d using %d bytes\n", id->blockIdCompressMode, sizeof(unsigned char)); }
//}


//if(id->densityEncodedBufCount != 0){
//Read the length of blockIds metadata
memcpy(&id->blockIdBufLen, id->inBuf + inBufOffset, sizeof(unsigned int)); inBufOffset += sizeof(unsigned int);
if(LOG){ debug(id->logLevel, 1, "Read the length of blockIds metadata as %d using %d bytes\n", id->blockIdBufLen, sizeof(unsigned int)); }
//}


//if(id->densityEncodedBufCount != 0){
//Read the blockIds metadata
id->blockIdBuf = (unsigned char*)malloc(sizeof(unsigned char)*id->blockIdBufLen); memset(id->blockIdBuf, 0, sizeof(unsigned char)*id->blockIdBufLen);
memcpy(id->blockIdBuf, id->inBuf + inBufOffset, sizeof(unsigned char)*id->blockIdBufLen); inBufOffset += sizeof(unsigned char)*id->blockIdBufLen;
if(LOG){ debug(id->logLevel, 1, "Read blockIdBuf using %d bytes\n", sizeof(unsigned char)*id->blockIdBufLen); }
//}


if(FULL_GMP){
    //Read the length of the partialBytesBuf
    memcpy(&id->partialBytesBufLen, id->inBuf + inBufOffset, sizeof(unsigned short)); inBufOffset += sizeof(unsigned short);
    if(LOG){ debug(id->logLevel, 1, "Read partialBytesBufLen %d using %d bytes\n", id->partialBytesBufLen, sizeof(unsigned short)); }


    //Read the partialBytesBuf for partial bytes
    id->partialBytes = (unsigned char*)malloc(sizeof(unsigned char)*(int)ceil((float)id->partialBytesBufLen/(float)8)); 
    memset(id->partialBytes, 0, sizeof(unsigned char)*(int)ceil((float)id->partialBytesBufLen/(float)8));
    memcpy(id->partialBytes, id->inBuf + inBufOffset, sizeof(unsigned char)*(int)ceil((float)id->partialBytesBufLen/(float)8)); 
    inBufOffset += sizeof(unsigned char)*(int)ceil((float)id->partialBytesBufLen/(float)8);
    if(LOG){ debug(id->logLevel, 1, "Read partialBytes using %d bytes\n", sizeof(unsigned char)*(int)ceil((float)id->partialBytesBufLen/(float)8)); }
    extractPartialBytesBufFromBytes(id);
}

if(!FULL_GMP){
    //Read the length of firstValDecLen
    memcpy(&id->firstValDecLen, id->inBuf + inBufOffset, sizeof(unsigned short)); inBufOffset += sizeof(unsigned short);
    if(LOG){ debug(id->logLevel, 1, "Read the number of encoded values %u using %d bytes\n", id->firstValDecLen, sizeof(unsigned short)); }


    //Read the firstValDec values
    id->firstValDec = (unsigned char*)malloc(sizeof(unsigned char)*id->firstValDecLen); memset(id->firstValDec, 0, sizeof(unsigned char)*id->firstValDecLen);
    memcpy(id->firstValDec, id->inBuf + inBufOffset, sizeof(unsigned char)*id->firstValDecLen); inBufOffset += sizeof(unsigned char)*id->firstValDecLen;
    if(LOG){ debug(id->logLevel, 1, "Read the firstValDec data using %d bytes\n", sizeof(unsigned char)*id->firstValDecLen); }


    //Read the length of encoded values 
    id->encValLen = (unsigned int*)malloc(sizeof(unsigned int)*id->firstValDecLen); memset(id->encValLen, 0, sizeof(unsigned int)*id->firstValDecLen);
    memcpy(id->encValLen, id->inBuf + inBufOffset, sizeof(unsigned int)*id->firstValDecLen); inBufOffset += sizeof(unsigned int)*id->firstValDecLen;
    if(LOG){ debug(id->logLevel, 1, "Read the lengths of encoded values using %d bytes\n", sizeof(unsigned int)*id->firstValDecLen); }
}

//Read the length of gmp encoded outBuf
memcpy(&id->outBufLen, id->inBuf + inBufOffset, sizeof(unsigned int)); inBufOffset += sizeof(unsigned int);
if(LOG){ debug(id->logLevel, 1, "Read length of gmp encoded outBuf %d using %d bytes\n", id->outBufLen, sizeof(unsigned int)); }


//Read the gmp encoded outBuf
id->outBuf = (unsigned char*)malloc(sizeof(unsigned char)*id->outBufLen); memset(id->outBuf, 0, sizeof(unsigned char)*id->outBufLen);
memcpy(id->outBuf, id->inBuf + inBufOffset, sizeof(unsigned char)*id->outBufLen); inBufOffset += sizeof(unsigned char)*id->outBufLen;
if(LOG){ debug(id->logLevel, 1, "Read gmp encoded outBuf using %d bytes\n", sizeof(unsigned char)*id->outBufLen); }

id->decDataLen = 0;
id->blockIdOffset = 0;
decompressBlockIdBuf(id);

//Extract the gmp values from the combined gmpBuf
//if(FULL_GMP){
//    extractGmpValuesFromBuf(id);
//}
//if(!FULL_GMP){
    extractEncValuesFromBufNew(id);
//}
//Combine Relative Positions
//Combine Relative Positions
struct timeval t1, t2;
id->decData = (unsigned char*)malloc(sizeof(unsigned char)*id->inCharCount); //memset(id->decData, 0, sizeof(unsigned short)*id->inCharCount);
if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } }
//combineRelativePositions(id);
combineRelativePosNew(id->decData,id->inCharCount,id->charDataCount,id->charData,id->rp,id->rpCount);
id->decDataLen=id->inCharCount;
if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } }
//if(TL){ printf("combine TIME: %lld\n", (uint64_t)timeval_diff(NULL, &t2, &t1)); }
//combineRelativePositionsSmart(id);


//printf("%s file with %d bytes successfully decompressed to %d bytes\n", id->filename, id->inBufLen, id->decDataLen);

//Cleanup all the allocations
decCleanup(id);
if(LOG){ debug(HIGH, 0, "Everything cleaned up\n"); }

}



