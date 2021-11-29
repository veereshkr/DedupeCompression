#include <stdio.h>
#include <math.h>
#include "gmp.h"
#include "encodeOps.h"
#include "decodeOps.h"
#include "misc.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to find the nCr len using gmp */
/* Input: */
/* Output: */
int nCrBitLen(int n, int r){
mpz_t bV; mpz_init(bV); mpz_bin_uiui(bV, n, r); return mpz_sizeinbase(bV, 2);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to calculate binomial value by multiplying and dividing a previous value */
/* Input: */
/* Output: */
void nCrByMultiDiv(int logging, mpz_t bV, mpz_t bVByMultiDiv, int prevN, int prevR, int curN, int curR){
if(LOG){ debug(LOW, 4, "(nCrByMultiDiv)(prevN, prevR)(%d, %d) (curN, curR)(%d, %d)\n", prevN, prevR, curN, curR); }

int i = 0, j = 0;
if(curR == prevR - 1 && prevN > curN){
    mpz_init(bVByMultiDiv);
    //mpz_set_ui(bVByMultiDiv, 0);
    mpz_mul_ui(bVByMultiDiv, bV, prevR); //gmp_printf("after mul: %Zd\n", bVByMultiDiv);
    mpz_divexact_ui(bVByMultiDiv, bVByMultiDiv, prevN); //gmp_printf("after div: %Zd\n", bVByMultiDiv);
    for(i = 1; i < prevN - curN; i++){
        mpz_mul_ui(bVByMultiDiv, bVByMultiDiv, (prevN - i)-(prevR - 1) ); //gmp_printf("after mul: %Zd\n", bVByMultiDiv);
        mpz_divexact_ui(bVByMultiDiv, bVByMultiDiv, prevN - i); //gmp_printf("after div: %Zd\n", bVByMultiDiv);
    }
}
if(curR == prevR && prevN > curN){
    mpz_init(bVByMultiDiv);
    //mpz_set_ui(bVByMultiDiv, 0);
    mpz_set(bVByMultiDiv, bV);
    for(i = 0; i < prevN - curN; i++){
        mpz_mul_ui(bVByMultiDiv, bVByMultiDiv, (prevN - i)-(prevR)); //gmp_printf("after mul: %Zd\n", bVByMultiDiv);
        mpz_divexact_ui(bVByMultiDiv, bVByMultiDiv, prevN - i); //gmp_printf("after div: %Zd\n", bVByMultiDiv);
    }
}
if(curN == prevN && curR == prevR){
    mpz_init(bVByMultiDiv);
    //mpz_set_ui(bVByMultiDiv, 0);
    mpz_set(bVByMultiDiv, bV);
}
//if(logging == VERY_LOW){ gmp_printf("bvByMultiDiv: %Zd\n", bVByMultiDiv); }
if(LOG){ debug(LOW, 4, "(nCrByMultiDiv)End\n"); }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to write a gmp value into file using the words */
/* Input: */
/* Output: */
void writeGmpToFile(mpz_t bV, int maxBitLen, CD* od){
int bVBitLen = mpz_sizeinbase(bV, 2);
if(LOG){ debug(HIGH, 0, "(writeGmpToFile)Start: gmpBits: %d, maxBitLen: %d\n", bVBitLen, maxBitLen); }

//Step 1, export the gmp words into an array
size_t size = 1; //Number of bytes in each gmp word
size_t count_p = 0; //Number of gmp words
int order = 1;
int endian = 1;
size_t nails = 0;
void* rop = NULL; //So that gmp allocates the stack and a pointer is returned to mpzWords
void* mpzWords = mpz_export(rop, &count_p, order, size, endian, nails, bV);
unsigned char lastByte = NULL, emptyByte = 0;
int i = 0, j = 0, totalBytesToFile = 0, emptyBytesCount = 0;
if(LOG){ debug(LOW, 1, "Number of gmp words: %d\n", count_p); }

if(maxBitLen % 8 != 0){
    //Check for the empty partial byte as per the maxBitLen
    if(maxBitLen % 8 != 0 && bVBitLen <= (maxBitLen/8)*8){
        if(LOG){ debug(LOW, 1, "Most significant byte to be written to file is partial and is empty, i.e. only zeros with length of %d\n", maxBitLen % 8); }
        //Fill the partial bits as zeros into partialBytesBuf
        for(i = 0; i < maxBitLen % 8; i++){ 
            od->partialBytesBuf[(od->partialBytesBufLen)++] = 0;  
            if(LOG){ debug(VERY_LOW, 0, "ZERO "); }
        }
        if(ceil((float)bVBitLen/(float)8) < maxBitLen/8){ //e.g. bVBitLen is 114 and maxBitLen is 132, i.e. 15-16th byte is empty
            //Full empty bytes are present
            emptyBytesCount = maxBitLen/8 - (int)ceil((float)bVBitLen/(float)8);
            if(LOG){ debug(LOW, 1, "\nemptyBytesCount %d full bytes are empty from %d to %d\n", emptyBytesCount, (int)ceil((float)bVBitLen/(float)8), maxBitLen/8); }
            for(i = 0; i < emptyBytesCount; i++){
                memcpy(od->outBuf + od->outBufLen, &emptyByte, sizeof(unsigned char)); od->outBufLen++;
                //\\fwrite(&emptyByte, sizeof(unsigned char), 1, od->of); 
                totalBytesToFile++;
            }
            memcpy(&lastByte, (char*)mpzWords, sizeof(unsigned char));
            if(LOG){ debug(VERY_LOW, 1, "lastByte after full empty bytes: %d\n", lastByte); }
        } else {
            memcpy(&lastByte, (char*)mpzWords, sizeof(unsigned char));
            if(LOG){ debug(VERY_LOW, 1, "lastByte without any empty bytes: %d\n", lastByte); }
        }
        //After writing the empty partial byte and full empty bytes seperately, dump the rest of the gmp exported words into file
        memcpy(od->outBuf + od->outBufLen, (char*)mpzWords, sizeof(unsigned char)*size*count_p); od->outBufLen += size*count_p;
        //\\fwrite((char*)mpzWords, size*count_p, 1, od->of); 
        totalBytesToFile += size*count_p;
        if(LOG){ debug(LOW, 1, "Wrote %d gmp exported words to file\n", size*count_p); }
    } else {
        if(maxBitLen % 8 != 0){ //i.e. bVBitLen > (maxBitLen/8)*8
            //Most significant byte is partial but not empty
            if(LOG){ debug(LOW, 1, "Most significant byte to be written to file is partial and is not empty with length of %d\n", maxBitLen % 8); }
            //Fill the partial bits into partialBytesBuf
            memcpy(&lastByte, (char*)mpzWords, sizeof(unsigned char));
            if(LOG){ debug(VERY_LOW, 1, "lastByte to be moved to partialBytesBuf: %d\n", lastByte); }
            for(i = 0; i < bVBitLen % 8; i++){
                if(lastByte & 1){ 
                    od->partialBytesBuf[(od->partialBytesBufLen)++] = 1;
                    if(LOG){ debug(VERY_LOW, 0, "ONE "); }
                }
                else { 
                    od->partialBytesBuf[(od->partialBytesBufLen)++] = 0;
                    if(LOG){ debug(VERY_LOW, 0, "ZERO "); }
                }
                lastByte >>= 1;
            }
            for(i = 0; i < maxBitLen % 8 - bVBitLen % 8; i++){
                od->partialBytesBuf[(od->partialBytesBufLen)++] = 0;
                if(LOG){ debug(VERY_LOW, 0, "ZERO "); }
            }
        }
        //After writing the partial byte seperately, dump the rest of the gmp exported words into file
        memcpy(od->outBuf + od->outBufLen, (char*)mpzWords + 1, sizeof(unsigned char)*size*count_p - 1); od->outBufLen += size*count_p - 1;
        //\\fwrite((char*)mpzWords + 1, size*count_p - 1, 1, od->of); 
        totalBytesToFile += size*count_p - 1;
        if(LOG){ debug(LOW, 1, "\nWrote %d gmp exported words to file\n", size*count_p - 1); }
    }
    if(LOG){ debug(LOW, 1, "Number of current partial bits: %d\n", od->partialBytesBufLen); }
} else { //last byte is not partial
    //All Bytes are full
    if((int)ceil((float)bVBitLen/(float)8) < maxBitLen/8){ //i.e. bVBitLen is 70 and maxBitLen is 80, therefore last Byte is not exported thus needs to be put manually
        if(LOG){ debug(LOW, 1, "emptyBytesCount Inserting %d fully empty bytes manually\n", maxBitLen/8 - (int)ceil((float)bVBitLen/(float)8)); }
        emptyBytesCount = maxBitLen/8 - (int)ceil((float)bVBitLen/(float)8);
    }
    for(i = 0; i < emptyBytesCount; i++){
        memcpy(od->outBuf + od->outBufLen, &emptyByte, sizeof(unsigned char)); od->outBufLen++;
        //\\fwrite(&emptyByte, sizeof(unsigned char), 1, od->of); 
        totalBytesToFile++;
    }
    memcpy(od->outBuf + od->outBufLen, (char*)mpzWords, sizeof(unsigned char)*size*count_p); od->outBufLen += size*count_p;
    //\\fwrite(mpzWords, size*count_p, 1, od->of); 
    totalBytesToFile += size*count_p;
    if(LOG){ debug(LOW, 1, "Wrote %d gmp exported words to file\n", size*count_p); }

}
if(LOG){ debug(LOW, 2, "totalBytesToFile Actual: %d, totalBytesToFile Expected: %d\n", totalBytesToFile, (int)ceil((float)maxBitLen/(float)8)); }
if(maxBitLen % 8 != 0){ 
    if(totalBytesToFile != (int)ceil((float)maxBitLen/(float)8) - 1){ if(LOG){ debug(HIGH, 0, "Fatal Error\n"); } }
} else {
    if(totalBytesToFile != (int)ceil((float)maxBitLen/(float)8)){ if(LOG){ debug(HIGH, 0, "Fatal Error\n"); } }
}
od->ofDataLen += totalBytesToFile;
od->emptyBytesCount += emptyBytesCount;
if(emptyBytesCount < 0){ if(LOG){ debug(HIGH, 0, "Fatal Error with emptyBytesCount\n"); } }

if(mpzWords != NULL){ free(mpzWords); }
if(LOG){ debug(HIGH, 0, "(writeGmpToFile)End\n"); }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to write a gmp value into file using the words, without using partial bytes */
/* Input: */
/* Output: */
void writeGmpToFileFull(mpz_t bV, int maxBitLen, CD* od){
int bVBitLen = mpz_sizeinbase(bV, 2);
if(LOG){ debug(VERY_HIGH, 0, "(writeGmpToFileFull)Start: gmpBits: %d, maxBitLen: %d\n", bVBitLen, maxBitLen); }

//Step 1, export the gmp words into an array
size_t size = 1; //Number of bytes in each gmp word
size_t count_p = 0; //Number of gmp words
int order = 1;
int endian = 1;
size_t nails = 0;
void* rop = NULL; //So that gmp allocates the stack and a pointer is returned to mpzWords
void* mpzWords = mpz_export(rop, &count_p, order, size, endian, nails, bV);
unsigned char lastByte = NULL, emptyByte = 0;
int i = 0, j = 0, totalBytesToFile = 0, emptyBytesCount = 0;
if(LOG){ debug(LOW, 1, "Number of gmp words: %d, first byte %d\n", count_p, ((unsigned char*)mpzWords)[0]); }

while(od->outBufLen + size*count_p >= od->outBufAllocSize){ doubleUpChar(&(od->outBuf), &(od->outBufAllocSize), od->outBufLen); }
memcpy(od->outBuf + od->outBufLen, (unsigned char*)mpzWords, sizeof(unsigned char)*size*count_p); 
od->outBufLen += size*count_p;

if(LOG){ debug(LOW, 1, "first byte again: %u\n", od->outBuf[0]); }


if(mpzWords != NULL){ free(mpzWords); }
if(LOG){ debug(VERY_HIGH, 1, "(writeGmpToFile)End Wrote %d bytes\n", sizeof(unsigned char)*size*count_p); }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to write a gmp value into file using the words, without using partial bytes */
/* Input: */
/* Output: */
void writeGmpToBuf(mpz_t bV, int maxBitLen, unsigned char** buf){
int bVBitLen = mpz_sizeinbase(bV, 2);
if(LOG){ debug(HIGH, 0, "(writeGmpToBuf)Start: gmpBits: %d, maxBitLen: %d\n", bVBitLen, maxBitLen); }

//Step 1, export the gmp words into an array
size_t size = 1; //Number of bytes in each gmp word
size_t count_p = 0; //Number of gmp words
int order = 1;
int endian = 1;
size_t nails = 0;
void* rop = NULL; //So that gmp allocates the stack and a pointer is returned to mpzWords
*buf = mpz_export(rop, &count_p, order, size, endian, nails, bV);
if(LOG){ debug(HIGH, 0, "(writeGmpToFile)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to return the gmpBitLen */
/* Input: */
/* Output: */
int gmpBitLen(int logging, int nCount, int rCount){
if(LOG){ debug(LOW, 2, "(gmpBitLen)nCount: %d, rCount: %d\n", nCount, rCount); }
mpz_t bV; mpz_init(bV);
int result = 0;
mpz_bin_uiui(bV, nCount, rCount);
result = mpz_sizeinbase(bV, 2);
mpz_clear(bV);
return result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to decode the gmpBuf */
/* Input: */
/* Output: */
void decodePosFromGmp(int curChar, mpz_t encValue, int nCount, int rCount, DD* id){
if(LOG){ debug(HIGH, 1, "\n(decodePosFromGmp)Start: curChar: %d, nCount: %d, rCount: %d\n", curChar, nCount, rCount); }

int i = 0, j = 0, k = 0, curN = 0, curR = 0, prevN = 0, prevR = 0;

id->rp[curChar] = (unsigned int*)malloc(sizeof(unsigned int)*nCount); memset(id->rp[curChar], 0, sizeof(unsigned int)*nCount);

//mpz_t bV, bVByMultiDiv; mpz_init(bV); mpz_bin_uiui(bV, nCount, rCount);
mpz_t bV; mpz_init(bV); mpz_bin_uiui(bV, nCount, rCount);
curN = nCount; curR = rCount; prevN = nCount; prevR = rCount;
//gmp_printf("(%d,%d): %Zd\n", nCount, rCount, bV);

for(i = nCount; i > 0; i--){
    mpz_t bVByMultiDiv;
    nCrByMultiDiv(id->logLevel, bV, bVByMultiDiv, prevN, prevR, curN, curR);
    prevN = curN; prevR = curR; mpz_set(bV, bVByMultiDiv);
    if(mpz_cmp(encValue, bVByMultiDiv) >= 0){ //Pos is valid
        if(LOG){ debug(LOW, 1, "%d position is valid\n", curN); }
        id->rp[curChar][(id->rpCount[curChar])++] = nCount - curN + 1;
        curR--;
        mpz_sub(encValue, encValue, bVByMultiDiv);
    }
    curN--; 
    mpz_clear(bVByMultiDiv);
}
if(LOG){ debug(LOW, 0, "\nRP: "); }
for(i = 0; i < id->rpCount[curChar]; i++){
    if(LOG){ debug(LOW, 1, "%d ", id->rp[curChar][i]); }
}
if(LOG){ debug(LOW, 0, "\n"); }
mpz_clear(bV); //mpz_clear(bVByMultiDiv);
if(LOG){ debug(HIGH, 1, "\n(decodePosFromGmp)End ******************************************* \n"); }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to decode the gmpBuf */
/* Input: */
/* Output: */
void extractGmpValuesFromBuf(DD* id){
if(LOG){ debug(HIGH, 1, "\n(extractGmpValuesFromBuf)Start\n"); }

int i = 0, j = 0, maxBitLen = 0, nCount = 0, rCount = 0, looseBitsCount = 0;
int partialBytesBufOffset = 0, outBufOffset = 0;
unsigned char curByte = 0;
for(i = 0; i < id->charDataCount; i++){
    nCount = id->nCountValues[i]; rCount = id->rCountBuf[i];

    //Get the maximum bit length for n+1 and r
    maxBitLen = gmpBitLen(id->logLevel, nCount+1, rCount); looseBitsCount = maxBitLen % 8;
    if(LOG){ debug(HIGH, 1, "(extractGmpValuesFromBuf)For character %d, nCount: %d and rCount: %d, maxBitLen: %d, number of loose bits: %d\n", id->charData[i], nCount, rCount, maxBitLen, looseBitsCount); }

    curByte = 0;
    //Read the loose bits from partialBytesBuf
    if(looseBitsCount != 0){
        for(j = 0; j < looseBitsCount; j++){
            curByte = curByte | (id->partialBytesBuf[j + partialBytesBufOffset] << j);
            if(LOG){ debug(LOW, 1, "%d ", id->partialBytesBuf[j + partialBytesBufOffset]); }
        }
        if(LOG){ debug(LOW, 0, "\n"); }
        if(LOG){ debug(LOW, 1, "curByte: %d\n", curByte); }
        partialBytesBufOffset += looseBitsCount;
    }

    //Read the gmp exported words
    if(LOG){ debug(id->logLevel, 1, "Number of full gmp words to be read: %d\n", (maxBitLen - looseBitsCount)/8); }
    //Step 1, import the gmp words into an array
    size_t size = 1; //Number of bytes in each gmp word
    size_t count_p = (maxBitLen - looseBitsCount)/8; //Number of gmp words
    int order = 1;
    int endian = 1;
    size_t nails = 0;
    unsigned char* tempGmpBuf = (unsigned char*)malloc(sizeof(unsigned char)*(int)ceil((float)maxBitLen/(float)8)+1);
    memset(tempGmpBuf, 0, sizeof(unsigned char)*(int)ceil((float)maxBitLen/(float)8)+1);
    mpz_t bV;
    mpz_init(bV);
    //Copy the partial byte into the array to be imported by gmp
    memcpy(tempGmpBuf, &curByte, sizeof(unsigned char));
    //Copy the rest of the gmp words
    memcpy(tempGmpBuf + sizeof(unsigned char), id->outBuf + outBufOffset, sizeof(unsigned char)*count_p);
    mpz_import(bV, count_p+1, order, size, endian, nails, tempGmpBuf);
    //mpz_import(bV, count_p, order, size, endian, nails, id->outBuf + outBufOffset);
    if(tempGmpBuf != NULL){ free(tempGmpBuf); }
    outBufOffset += count_p;
    //gmp_printf("gmp: %Zd\n", bV);

    decodePosFromGmp(id->charData[i], bV, nCount, rCount, id);
    
    if(LOG){ debug(id->logLevel, 0, "\n"); }
    mpz_clear(bV);
}
if(LOG){ debug(HIGH, 1, "\n(extractGmpValuesFromBuf)End\n"); }

}


