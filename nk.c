#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "gmp.h"
#include "misc.h"
#include "encodeOps.h"


//Global variables for outbuffer and outfile
//CD* od; Moved to main.c

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to compare the ORDER structure based on the count member */
/* Input: */
/* Output: */
int compareORDER(const void* left, const void* right){
if(((ORDER*)left)->count < ((ORDER*)right)->count){ return 1; }
else if(((ORDER*)left)->count > ((ORDER*)right)->count){ return -1; }
else { return 0; }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to find the order of characters as per their frequency */
/* Input: */
/* Output: */
void findEncodingOrder(int logging, unsigned char* inBuf, int len, ORDER vj_order[]){
if(LOG){ debug(HIGH, 1, "(findEncodingOrder)len: %d\n", len); }

int i = 0, j = 0, k = 0, l = 0;
for(i = 0; i < MAX_CHARS; i++){ vj_order[i].ch = i; vj_order[i].count = 0; }
for(i = 0; i < len; i++){ vj_order[inBuf[i]].count++; }
if(LOG){
    if(LOG){ debug(LOW, 1, "(findEncodingOrder)Character and count before sorting\n"); }
    for(i = 0; i < MAX_CHARS; i++){ if(vj_order[i].count != 0){ if(LOG){ debug(LOW, 2, "ch: %d, count: %d\n", vj_order[i].ch, vj_order[i].count); } } }
}
qsort(vj_order, MAX_CHARS, sizeof(ORDER), compareORDER);
if(LOG){
    if(LOG){ debug(LOW, 1, "(findEncodingOrder)Order of characters after sorting based on their count: "); }
    for(i = 0; i < MAX_CHARS; i++){ 
        if(vj_order[i].count != 0){ 
              if(LOG){ debug(LOW, 2, "ch: %d, count: %d\n", vj_order[i].ch, vj_order[i].count); } 
              if(LOG){ debug(LOW, 1, "%d ", vj_order[i].ch); } } 
    }
    if(LOG){ debug(LOW, 0, "\n"); }
}

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to initialize the relative positions */
/* Input: */
/* Output: */
void initializeRelativePos(int logging, int* rp[MAX_CHARS], int rpCount[MAX_CHARS], ORDER vj_order[]){
if(LOG){ debug(HIGH, 0, "(initializeRelativePos)Start\n"); }
int i = 0, curChar = -1;
for(i = 0; i < MAX_CHARS; i++){
	rpCount[i] = 0;
        if(vj_order[i].count != 0){
                rp[vj_order[i].ch] = (int*)malloc(sizeof(int)*(vj_order[i].count + 1));
                memset(rp[vj_order[i].ch], 0, sizeof(int)*(vj_order[i].count + 1));
        } else { rp[vj_order[i].ch] = NULL; }
}
if(LOG){ debug(HIGH, 0, "(initializeRelativePos)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to record the relative position of characters as per their frequency order */
/* Input: */
/* Output: */
void fillRelativePos(int logging, unsigned char* inBuf, int len, ORDER vj_order[], int* rp[MAX_CHARS], int rpCount[]){
if(LOG){ debug(HIGH, 1, "\n(fillRelativePos)Start:len: %d\n", len); }

int charsSeen[MAX_CHARS], charsSeenCount = 0, charsRank[MAX_CHARS], charsCount[MAX_CHARS], tempSum = 0;
unsigned char charsList[MAX_CHARS], curChar = 0;
int i = 0, j = 0, prevChar = -1;
memset(charsSeen, -1, sizeof(charsSeen));
memset(charsList, FALSE, sizeof(charsList));
memset(charsRank, 0, sizeof(charsRank));
memset(charsCount, 0, sizeof(charsCount));

//Obtain the rank of characters as per their frequency order, i.e character with most occurance gets the rank 1 and so on...
for(i = 0; i < MAX_CHARS; i++){ charsRank[vj_order[i].ch] = i + 1;  }
if(LOG){ debug(LOW, 0, "(fillRelativePos)Ranks: "); for(i = 0; i < MAX_CHARS; i++){ if(LOG){ debug(LOW, 1, "%d ", charsRank[i]); } } if(LOG){ debug(LOW, 0, "\n"); } }

//Normal Relative Positions as per rank
for(i = 0; i < len; i++){
        curChar = inBuf[i];
        if(charsList[curChar] == FALSE){ charsSeen[charsSeenCount++] = curChar; }
        charsList[curChar] = TRUE;
        charsCount[curChar]++;
        if(curChar == prevChar){
                tempSum += 1;
        } else {
                tempSum = 0;
                for(j = 0; j < charsSeenCount; j++){
                        if(charsRank[charsSeen[j]] >= charsRank[curChar]){
                                tempSum = tempSum + charsCount[charsSeen[j]];
                        }
                }
        }
        if(LOG){ debug(VERY_LOW, 2, "curChar: %d, relPos: %d\n", inBuf[i], tempSum); }
        rp[curChar][rpCount[curChar]] = tempSum;
        rpCount[curChar]++;
        prevChar = curChar;
}
if(LOG){ debug(logging, 0, "(fillRelativePos)End\n"); }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to display the relative positions of all the characters */
/* Input: */
/* Output: */
void displayRelativePos(int logging, int* rp[MAX_CHARS], int rpCount[], ORDER vj_order[], int len){
if(LOG){ debug(logging, 0, "\n(dispayRelativePos)Start"); }
int i = 0, j = 0, charsNCount[MAX_CHARS], tempNActual = 0;
for(i = 0; i < MAX_CHARS; i++){
        if(vj_order[i].count != 0){
                if(LOG){
		    if(LOG){ debug(LOW, 2, "\nrp[%d](%d): ", vj_order[i].ch, rpCount[vj_order[i].ch]); }
                    for(j = 0; j < rpCount[vj_order[i].ch]; j++){ if(LOG){ debug(LOW, 1, "%d ", rp[vj_order[i].ch][j]); } }
		}
                charsNCount[vj_order[i].ch] = len - tempNActual;
                tempNActual += rpCount[vj_order[i].ch];
                if(LOG){ debug(LOW, 1, " (%d)", charsNCount[vj_order[i].ch]); }
        }
}
if(LOG){ debug(logging, 0, "\n(dispayRelativePos)End\n"); }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to find the N Count for all the characters */
/* Input: */
/* Output: */
void findNCount(int logging, ORDER vj_order[], int len, int rpCount[], int charsNCount[]){
if(LOG){ debug(HIGH, 1, "\n(findNCount)Start:len: %d\n", len); }

int i = 0, tempNActual = 0;
for(i = 0; i < MAX_CHARS; i++){
    if(vj_order[i].count != 0){
        charsNCount[vj_order[i].ch] = len - tempNActual;
        tempNActual += rpCount[vj_order[i].ch];
        if(LOG){ debug(LOW, 1, " (%d)", charsNCount[vj_order[i].ch]); }
    }
}
if(LOG){ debug(HIGH, 1, "\n(findNCount)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to display the N Count for all the characters */
/* Input: */
/* Output: */
void displayNCount(int logging, ORDER vj_order[], int charsNCount[]){
if(LOG){ debug(logging, 1, "\n(displayNCount)Start\n"); }

int i = 0;
for(i = 0; i < MAX_CHARS; i++){
    if(LOG){
        if(vj_order[i].count != 0){
            if(LOG){ debug(LOW, 1, " char: %d, N count: %d\n", vj_order[i].ch, charsNCount[vj_order[i].ch]); }
        }
    }
}
if(LOG){ debug(logging, 1, "(displayNCount)End\n"); }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to get the bit count */
/* Input: */
/* Output: */
int bitCount(int n){
int result;
mpz_t bV; mpz_init(bV); mpz_set_ui(bV, n); result = mpz_sizeinbase(bV, 2); mpz_clear(bV); return result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to the distinct characters to the output buffer in the order of their encoding */
/* Input: */
/* Output: */
void writeCharDataToBuf(unsigned char curChar, CD* od){
if(LOG){ debug(HIGH, 1, "\n(writeCharDataToBuf)Start: curChar: %d\n", curChar); }
od->charData[od->charDataCount++] = curChar;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to add r value to output buffer short values */
/* Input: */
/* Output: */
void writeRCountToBuf(unsigned int rCount, CD* od){
if(LOG){ debug(HIGH, 1, "\n(writeRCountToBuf)Start: rCount: %d\n", rCount); }
od->rCountBuf[od->rCountBufLen++] = rCount;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to add n value to output buffer */
/* Input: */
/* Output: */
void writeNCountToBuf(int nCount, CD* od){
if(LOG){ debug(HIGH, 1, "\n(writeNCountToBuf)Start: nCount: %d\n", nCount); }
static int lastN = 0;
int i = 0, tempNCount = nCount, bcLastN = 0, bcCurN = bitCount(nCount);
if(od->nCountBufLen == 0){ lastN = od->inCharCount; od->nCountBuf[(od->nCountBufLen)++] = 1; } //For the first n
else { //For subsequent n
    for(i = 0; i < bcCurN; i++){
        if(tempNCount & 1){
            od->nCountBuf[(od->nCountBufLen)++] = 1;
            if(LOG){ debug(VERY_LOW, 0, "ONE "); }
        } else {
            od->nCountBuf[(od->nCountBufLen)++] = 0;
            if(LOG){ debug(VERY_LOW, 0, "ZERO "); }
        }
        tempNCount >>= 1;
    }
    bcLastN = bitCount(lastN);
    for(i = bcCurN; i < bcLastN; i++){
        od->nCountBuf[(od->nCountBufLen)++] = 0;
        if(LOG){ debug(VERY_LOW, 0, "ZERO "); }
    }
}

if(LOG){ debug(od->logLevel, 1, "lastN: %d, bitCount: %d\n", lastN, bitCount(lastN)); }
if(LOG){ debug(LOW, 1, "Current nCountBufLen: %d\n", od->nCountBufLen); }
lastN = nCount;
if(LOG){ debug(HIGH, 1, "\n(writeNCountToBuf)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to convert an nCount array of 0 and 1 into bytes */
/* Input: */
/* Output: */
void finalizeNCountInBuf(CD* od){
if(LOG){ debug(HIGH, 1, "\n(finalizeNCountInBuf)Start: nCountBufLen: %d\n", od->nCountBufLen); }
unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char)*(od->nCountBufLen));
memset(temp, 0, sizeof(unsigned char)*(od->nCountBufLen));
int tempLen = 0;
int i = 0, j = 0;
unsigned char curByte = 0, tempByte = 0;
if(LOG){ for(i = 0; i < od->nCountBufLen; i++){ if(LOG){ debug(LOW, 1, "%d ", od->nCountBuf[i]); } } }
//for(i = 0; i < (od->nCountBufLen/8)*8;){
for(i = 0; i < od->nCountBufLen;){
    curByte = 0;
    for(j = 7; j >= 0 && i < od->nCountBufLen; j--){
        tempByte = od->nCountBuf[i++];
        tempByte <<= j;
        curByte = curByte | tempByte;
    }
    if(LOG){ debug(LOW, 1, "curByte: %d\n", curByte); }
    temp[tempLen++] = curByte;
}
od->nCountBytes = temp;

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to convert partialBytesBuf array of 0 and 1 into bytes */
/* Input: */
/* Output: */
void finalizePartialBytes(CD* od){
if(LOG){ debug(HIGH, 1, "\n(finalizePatialBytes)Start: partialBytesBufLen: %d\n", od->partialBytesBufLen); }
unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char)*(od->partialBytesBufLen));
memset(temp, 0, sizeof(unsigned char)*(od->partialBytesBufLen));
int tempLen = 0;
int i = 0, j = 0;
unsigned char curByte = 0, tempByte = 0;
if(LOG){ for(i = 0; i < od->partialBytesBufLen; i++){ if(LOG){ debug(LOW, 1, "%d ", od->partialBytesBuf[i]); } } }
for(i = 0; i < od->partialBytesBufLen;){
    curByte = 0;
    for(j = 7; j >= 0 && i < od->partialBytesBufLen; j--){
        tempByte = od->partialBytesBuf[i++];
        tempByte <<= j;
        curByte = curByte | tempByte;
    }
    if(LOG){ debug(LOW, 1, "curByte: %d\n", curByte); }
    temp[tempLen++] = curByte;
}
od->partialBytes = temp;

}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to encode a set of positions using vajra without using density with full GMP operations*/
/* Input: */
/* Output: */
void displayEncCharData(CD* od){
if(LOG){ debug(od->logLevel, 1, "(displayEncCharData)Start\n"); }
int i = 0;
for(i = 0; i < od->charDataCount; i++){
    if(LOG){ debug(od->logLevel, 1, "%d ", od->charData[i]); }
}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to encode a set of positions using vajra without using density with full GMP operations*/
/* Input: */
/* Output: */
void encodePosByFullGmp(int logging, int curChar, int* rp, int rCount, int nCount, int len){
if(LOG){ debug(HIGH, 1, "\n(encodePos)StartcurChar: %d, nCount: %d, rCount: %d, len: %d\n", curChar, nCount, rCount, len); }

int i = 0, j = 0, k = 0, l = 0;
int n = 0, r = 0;
mpz_t bV, bigSum;
if(LOG){ debug(LOW, 0, "RP: "); for(i = 0; i < rCount; i++){ if(LOG){ debug(LOW, 1, "%d ", rp[i]); } } if(LOG){ debug(LOW, 0, "\n"); } }
if(rCount > 0 && nCount > 0){
    mpz_init(bigSum);
    r = rCount;
    for(i = 0 ; i < rCount; i++){
        n = nCount - rp[i] + 1;
        if(LOG){ debug(LOW, 2, "n: %d, r: %d\n", n, r); }
        mpz_init(bV); mpz_bin_uiui(bV, n, r); //gmp_printf(" bV: %Zd\n", bV);
        mpz_add(bigSum, bigSum, bV);
        r--; 
    }
}
//gmp_printf("bigSum: %Zd\n", bigSum);

mpz_clear(bV); mpz_clear(bigSum);
if(LOG){ debug(logging, 1, "\n(encodePos)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to determine the block type */
/* Input: */
/* Output: */
int findBlockType(int blockSize, int blockTypesCount, int posCount){
int curDensity = (posCount * 100) / blockSize;
if(LOG){ debug(HIGH, 1, "\n(findBlockType)Start: blockSize: %d, blockTypesCount: %d, posCount: %d, curDensity: %d\n", blockSize, blockTypesCount, posCount, curDensity); }
int blockType = -1;

if(posCount == 0){ printf("EMPTY\n"); } else { printf("NONEMPTY\n"); }

if(blockTypesCount == BLOCK_TYPES_COUNT_8){
    if(curDensity >= 90){ blockType = TYPE_8; }
    else if(curDensity >= 80){ blockType = TYPE_7; }
    else if(curDensity >= 65){ blockType = TYPE_6; }
    else if(curDensity >= 50){ blockType = TYPE_5; }
    else if(curDensity >= 35){ blockType = TYPE_4; }
    else if(curDensity >= 20){ blockType = TYPE_3; }
    else if(curDensity >= 10){ blockType = TYPE_2; }
    else { blockType = TYPE_1; }
}

if(LOG){ debug(HIGH, 1, "(findBlockType)End blockType: %d\n", blockType); }
return blockType;
}

void displayBlocks(unsigned int* blockRP[], unsigned int* blockRPCount, unsigned int* blockRPTypesCount, unsigned int* blockRPNCount){
int i = 0, j = 0;
for(i = 0; i < 8; i++){
    printf("Type%d: blockCount: %d, nCount: %d -- ", i, blockRPTypesCount[i], blockRPNCount[i]);
    for(j = 0; j < blockRPCount[i]; j++){
        printf("%d ", blockRP[i][j]);
    }
    printf("\n");
}
}


void verifyBlocksBad(int* rp, int rCount, unsigned int* blockRP[], unsigned int* blockRPCount, unsigned int* blockRPNCount, unsigned int* blockRPTypesCount, unsigned char* blockId, int blockIdLen){
if(LOG){ debug(HIGH, 1, "\n(verifyBlocksBad)Start\n"); }
int i = 0, j = 0;
unsigned char* tempBuf[8]; int tempBufLen[8], tempTypesCount[8];
int offset[8], blockType = -1, finalBufLen = 0, finalBufOffset = 0;
//Initialize
for(i = 0; i < 8; i++){
    if(blockRPNCount[i] != 0){
    tempBuf[i] = (unsigned char*)malloc(sizeof(unsigned char)*blockRPNCount[i]);
    memset(tempBuf[i], 1, sizeof(unsigned char)*blockRPNCount[i]);
    finalBufLen += blockRPNCount[i];
    } else { tempBuf[i] = NULL; }
    tempBufLen[i] = 0; offset[i] = 0; tempTypesCount[i] = 0;
}

unsigned char* finalBuf = (unsigned char*)malloc(sizeof(unsigned char)*finalBufLen);
memset(finalBuf, 0, sizeof(unsigned char)*finalBufLen);
finalBufLen = 0;

//Convert them as 0 and 1
for(i = 0; i < 8; i++){
    if(blockRPNCount[i] != 0){
        for(j = 0; j < blockRPCount[i]; j++){
            tempBuf[i][blockRP[i][j]-1] = 0;
        }
        tempBufLen[i] = blockRPNCount[i];
    }
}

//Display
for(i = 0; i < 8; i++){
    printf("Type%d: ", i); for(j = 0; j < tempBufLen[i]; j++){ printf("%d", tempBuf[i][j]); } printf("\n");
}

printf("blockId:\n");
for(i = 0; i < blockIdLen; i++){
    printf("%d ", blockId[i]);
}
printf("\n");

for(i = 0; i < blockIdLen; i++){
    blockType = blockId[i]; tempTypesCount[blockType]++;
    if(tempTypesCount[blockType] == blockRPTypesCount[blockType]){
        memcpy(finalBuf + finalBufOffset, tempBuf[blockType] + offset[blockType], sizeof(unsigned char)*(blockRPNCount[blockType]-(blockRPTypesCount[blockType]-1)*64));
        finalBufOffset += sizeof(unsigned char)*(blockRPNCount[blockType]-(blockRPTypesCount[blockType]-1)*64);
    } else {
        memcpy(finalBuf + finalBufOffset, tempBuf[blockType] + offset[blockType], sizeof(unsigned char)*64);
        finalBufOffset += sizeof(unsigned char)*64;
    }
    offset[blockType] += 64;
}


int* testRP = (int*)malloc(sizeof(int)*finalBufOffset); memset(testRP, 0, sizeof(int)*finalBufOffset);
int testRPCount = 0;
printf("FINAL:");
for(i = 0; i < finalBufOffset; i++){
    printf("%d", finalBuf[i]);
    if(finalBuf[i] == 0){
        testRP[testRPCount++] = i+1;
    }
}
printf("\n");

for(i = 0; i < rCount; i++){
    printf("%d - %d \n", rp[i], testRP[i]);
    if(rp[i] != testRP[i]){ printf("verify failure...\n"); }
}
//Free
for(i = 0; i < 8; i++){
    if(tempBuf[i] != NULL && blockRPNCount[i] != 0){ free(tempBuf[i]); }
}
if(finalBuf != NULL){ free(finalBuf); }
if(testRP != NULL){ free(testRP); }
}


void verifyBlocks(int* rp, int rCount, unsigned int* blockRP[], unsigned int* blockRPCount, unsigned char* blockId, int blockIdLen){
if(LOG){ debug(HIGH, 1, "\n(verifyBlocks)Start\n"); }
int i = 0, j = 0;
int* testRP = (int*)malloc(sizeof(int)*rCount); memset(testRP, 0, sizeof(int)*rCount);
int testRPCount = 0, blockType = -1;
int blockRPOffset[8]; memset(blockRPOffset, 0, sizeof(blockRPOffset));
int blockTypesCount[8]; memset(blockTypesCount, 0, sizeof(blockTypesCount));
for(i = 0; i < blockIdLen; i++){
    blockType = blockId[i];
    blockTypesCount[blockType]++;
    for(j = blockRPOffset[blockType]; j < blockRPOffset[blockType]+64; j++){
        if(blockRP[blockType][j] <= 64*blockTypesCount[blockType]){
            testRP[testRPCount++] = blockRP[blockType][j];
            printf("testRPCount: %d, pos: %d\n", testRPCount, blockRP[blockType][j]);
            blockRPOffset[blockType]++;
        } else { break; }
    }
}
if(testRP != NULL){ free(testRP); }
}

void updateBlockRP(int blockType, int blockSize, unsigned int* blockRP[], unsigned int* blockRPCount, unsigned int* blockRPTypesCount, unsigned int* blockRPNCount, unsigned int* tempBlock, unsigned int tempBlockCount, int blockStartPos){
if(LOG){ debug(HIGH, 1, "\n(updateBlockRP)Start: blockType: %d, blockStartPos: %d\n", blockType, blockStartPos); }
int i = 0, j = 0, k = 0, offset = (blockRPTypesCount[blockType]-1)*64;
blockRPNCount[blockType] += blockSize;
printf("offset: %d\n", offset);
for(i = 0; i < tempBlockCount; i++){
    blockRP[blockType][blockRPCount[blockType]++] = offset + tempBlock[i] - blockStartPos + 1;
    printf("*%d*", offset + tempBlock[i] - blockStartPos + 1);
}
if(LOG){ debug(HIGH, 1, "\n(updateBlockRP)End\n"); }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to apply density on positions and make different sets of positions */
/* Input: */
/* Output: */
void applyDensity(int curChar, int* rp, int rCount, int nCount, CD* od){
if(LOG){ debug(HIGH, 1, "\n(applyDensity)Start: curChar: %d, nCount: %d, rCount: %d\n", curChar, nCount, rCount); }

int blockSize = 64, blockTypesCount = 8, blockCount = 1, posCount = 0, blockType = -1, blockStartPos = 0;
int i = 0, j = 0, k = 0, l = 0;
int check = 0;

unsigned int tempBlock[blockSize], tempBlockCount = 0; memset(tempBlock, 0, sizeof(tempBlock));
unsigned int* blockRP[blockTypesCount], blockRPCount[blockTypesCount], blockRPTypesCount[blockTypesCount], blockRPNCount[blockTypesCount];
for(i = 0; i < blockTypesCount; i++){
    blockRP[i] = (unsigned int*)malloc(sizeof(unsigned int)*nCount); memset(blockRP[i], 0, sizeof(unsigned int)*nCount);
    blockRPCount[i] = 0; blockRPTypesCount[i] = 0; blockRPNCount[i] = 0;
}
unsigned char* blockId = (unsigned char*)malloc(sizeof(unsigned char)*((nCount/blockSize)+1)); memset(blockId, 0, sizeof(unsigned char)*((nCount/blockSize)+1));
int blockIdLen = 0;

if(LOG){ debug(LOW, 1, "%d - ", 1); } blockStartPos = 1;
for(i = 0; i < rCount; i++){
    if(rp[i] <= blockSize*blockCount){
        posCount++; tempBlock[tempBlockCount++] = rp[i];
        if(LOG){ debug(LOW, 1, "%d ", rp[i]); }
    } else {
        if(LOG){ debug(LOW, 1, " - %d  (%d) \n", blockSize*blockCount, posCount); }
        blockType = findBlockType(blockSize, blockTypesCount, posCount);
        blockId[blockIdLen++] = blockType;
        blockRPTypesCount[blockType]++;
        for(j = 0; j < tempBlockCount; j++){ printf("#%d#", tempBlock[j]); } printf("\n");
        updateBlockRP(blockType, blockSize, blockRP, blockRPCount, blockRPTypesCount, blockRPNCount, tempBlock, tempBlockCount, blockStartPos);

        if(LOG){ debug(LOW, 0, "************************************************\n"); }
        if(LOG){ debug(LOW, 1, "%d - ", blockSize*blockCount+1); }
        tempBlockCount = 0; blockStartPos = blockSize*blockCount+1;
        blockCount++; check += posCount;
        posCount = 0; i--;
    }
    if(i == rCount - 1){
        if(LOG){ debug(LOW, 1, " - %d  (%d) \n", blockSize*blockCount, posCount); }
        if(nCount > blockSize*blockCount){
            blockType = findBlockType(blockSize, blockTypesCount, posCount);
            blockId[blockIdLen++] = blockType;
            blockRPTypesCount[blockType]++;
            for(j = 0; j < tempBlockCount; j++){ printf("#%d#", tempBlock[j]); } printf("\n");
            updateBlockRP(blockType, blockSize, blockRP, blockRPCount, blockRPTypesCount, blockRPNCount, tempBlock, tempBlockCount, blockStartPos);
            if(LOG){ debug(LOW, 0, "************************************************\n"); }
            if(LOG){ debug(LOW, 1, "%d - ", blockSize*blockCount+1); }
            tempBlockCount = 0; blockStartPos = blockSize*blockCount+1;
            blockCount++; check += posCount; posCount = 0;

            //For the empty blocks in the end
            int emptyFullCount = (nCount-blockSize*(blockCount-1))/blockSize;
            for(k = 0; k < emptyFullCount; k++){
                if(LOG){ debug(LOW, 1, " - %d  (%d) \n", blockSize*blockCount, posCount); }
                blockType = findBlockType(blockSize, blockTypesCount, 0);
                blockId[blockIdLen++] = blockType;
                blockRPTypesCount[blockType]++;
                for(j = 0; j < tempBlockCount; j++){ printf("#%d#", tempBlock[j]); } printf("\n");
                updateBlockRP(blockType, blockSize, blockRP, blockRPCount, blockRPTypesCount, blockRPNCount, tempBlock, tempBlockCount, blockStartPos);
                if(LOG){ debug(LOW, 0, "************************************************\n"); }
                if(LOG){ debug(LOW, 1, "%d - ", blockSize*blockCount+1); }
                tempBlockCount = 0; blockStartPos = blockSize*blockCount+1;
                blockCount++; check += posCount; posCount = 0;
            }

            //Last empty incomplete block
            if(nCount-blockSize*(blockCount-1) > 0){
                if(LOG){ debug(LOW, 1, " - %d  (%d) \n", blockSize*blockCount, posCount); }
                blockType = findBlockType(nCount-blockSize*(blockCount-1), blockTypesCount, 0);
                blockId[blockIdLen++] = blockType;
                blockRPTypesCount[blockType]++;
                for(j = 0; j < tempBlockCount; j++){ printf("#%d#", tempBlock[j]); } printf("\n");
                updateBlockRP(blockType, nCount-blockSize*(blockCount-1), blockRP, blockRPCount, blockRPTypesCount, blockRPNCount, tempBlock, tempBlockCount, blockStartPos);
                if(LOG){ debug(LOW, 0, "************************************************\n"); }
                if(LOG){ debug(LOW, 1, "%d - ", blockSize*blockCount+1); }
                tempBlockCount = 0; blockStartPos = blockSize*blockCount+1;
                blockCount++; check += posCount; posCount = 0;
            }

        } else {
            blockType = findBlockType(nCount-blockSize*(blockCount-1), blockTypesCount, posCount);
            blockId[blockIdLen++] = blockType;
            blockRPTypesCount[blockType]++;
            for(j = 0; j < tempBlockCount; j++){ printf("#%d#", tempBlock[j]); } printf("\n");
            updateBlockRP(blockType, nCount-blockSize*(blockCount-1), blockRP, blockRPCount, blockRPTypesCount, blockRPNCount, tempBlock, tempBlockCount, blockStartPos);
        }

        if(LOG){ debug(LOW, 0, "************************************************\n"); }
        check += posCount;
    }
    
}
printf("rCount: %d, check: %d\n", rCount, check); if(check != rCount){ printf("density failure...\n"); }
displayBlocks(blockRP, blockRPCount, blockRPTypesCount, blockRPNCount);
//verifyBlocks(rp, rCount, blockRP, blockRPCount, blockId, blockIdLen);
verifyBlocksBad(rp, rCount, blockRP, blockRPCount, blockRPNCount, blockRPTypesCount, blockId, blockIdLen);


if(LOG){ debug(HIGH, 1, "\n(applyDensity)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to encode a set of positions using vajra without using density */
/* Input: */
/* Output: */
void encodePos(int curChar, int* rp, int rCount, int nCount, int len, CD* od){
if(LOG){ debug(HIGH, 1, "\n(encodePos)Start: curChar: %d, nCount: %d, rCount: %d, len: %d\n", curChar, nCount, rCount, len); }

uint64_t tAll = 0LL; //Time logging

int i = 0, j = 0, k = 0, l = 0;
int n = 0, r = 0, maxBitLen = 0;
int prevN = 0, prevR = 0, curN = 0, curR = 0;
mpz_t bigSum;
if(LOG){ debug(LOW, 0, "RP: "); for(i = 0; i < rCount; i++){ if(LOG){ debug(LOW, 1, "%d ", rp[i]); } } if(LOG){ debug(LOW, 0, "\n"); } }
if(rCount > 0 && nCount > 0){
    mpz_init(bigSum);
    //Find the first value using gmp, find n+1Cr so that we get the max number of bits possible for the encoded value and that will be used while writing the value to the file
    struct timeval t1, t2; uint64_t t12 = 0LL;
    if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
    r = rCount; prevR = r;
    n = nCount + 1; prevN = n;
    mpz_t advBV; mpz_init(advBV); mpz_bin_uiui(advBV, n, r);
    mpz_t bVByMultiDiv; mpz_init2(bVByMultiDiv, mpz_sizeinbase(advBV, 2));
    mpz_t bV; mpz_init(bV); mpz_bin_uiui(bV, n, r);
    maxBitLen = mpz_sizeinbase(bV, 2);
    if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
    if(TL){ t12 = (uint64_t)timeval_diff(NULL, &t2, &t1); printf("For char %d time taken for %d C %d is %lld\n", curChar, n, r, t12); tAll += t12; }
    if(LOG){ debug(LOW, 2, "n+1: %d, r: %d\n", n, r); } //gmp_printf(" bV: %Zd, maxBitLen: %d\n", bV, maxBitLen);
    //For successive binomial values, use multiplication and division
    uint64_t tMultiDivSum = 0LL, tAddSum = 0LL;
    for(i = 0 ; i < rCount; i++){
        struct timeval t1, t2, t3, t4; uint64_t t12 = 0LL, t34 = 0LL;
        n = nCount - rp[i] + 1; curN = n; curR = r;
        if(LOG){ debug(LOW, 2, "n: %d, r: %d ", n, r); }
        if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
        nCrByMultiDiv(od->logLevel, bV, bVByMultiDiv, prevN, prevR, curN, curR);
        if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
        tMultiDivSum += t12 = (uint64_t)timeval_diff(NULL, &t2, &t1); tAll += t12;
        if(TL){ if(gettimeofday(&t3, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
        prevN = curN; prevR = curR; mpz_set(bV, bVByMultiDiv);
        mpz_add(bigSum, bigSum, bVByMultiDiv);
        if(TL){ if(gettimeofday(&t4, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
        if(TL){ tAddSum += t34 = (uint64_t)timeval_diff(NULL, &t4, &t3); tAll += t34; }
        r--; 
    }
    if(TL){ printf("For char %d time taken for subsequent multi and div is %lld\n", curChar, tMultiDivSum); }
    if(TL){ printf("For char %d time taken for subsequent add is %lld\n", curChar, tAddSum); }
    mpz_clear(bV); mpz_clear(bVByMultiDiv);
}
//gmp_printf("bigSum: %Zd\n, NK: %d\n", bigSum, mpz_sizeinbase(bigSum, 2));
//gmp_fprintf(of, "%Zd\n", bigSum);
writeGmpToFile(bigSum, maxBitLen, od);
writeNCountToBuf(nCount, od);
writeRCountToBuf((unsigned int)rCount, od);
writeCharDataToBuf((unsigned char)curChar, od);


if(TL){ printf("For char %d time taken for all steps is %lld\n", curChar, tAll); }

if(LOG){ debug(HIGH, 1, "(encodePos)End *******************************************\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to cleanup all the allocations */
/* Input: */
/* Output: */
void encCleanup(CD* od){
if(LOG){ debug(HIGH, 0, "(encCleanup)Start\n"); }
fclose(od->of);
if(od->charData != NULL){ free(od->charData); } if(LOG){ debug(LOW, 0, "After freeing charData\n"); }
if(od->fileBuf != NULL){ free(od->fileBuf); } if(LOG){ debug(LOW, 0, "After freeing fileBuf\n"); }
if(od->outBuf != NULL){ free(od->outBuf); } if(LOG){ debug(LOW, 0, "After freeing outBuf\n"); }
if(od->partialBytesBuf != NULL){ free(od->partialBytesBuf); } if(LOG){ debug(LOW, 0, "After freeing partialBytesBuf\n"); }
if(od->partialBytes != NULL){ free(od->partialBytes); } if(LOG){ debug(LOW, 0, "After freeing partialBytes\n"); }
if(od->nCountBuf != NULL){ free(od->nCountBuf); } if(LOG){ debug(LOW, 0, "After freeing nCountBuf\n"); }
if(od->nCountBytes != NULL){ free(od->nCountBytes); } if(LOG){ debug(LOW, 0, "After freeing nCountBytes\n"); }
if(od->rCountBuf != NULL){ free(od->rCountBuf); } if(LOG){ debug(LOW, 0, "After freeing rCountBuf\n"); }
if(od->firstValDec != NULL){ free(od->firstValDec); } if(LOG){ debug(LOW, 0, "After freeing firstValDec\n"); }
if(od->encValLen != NULL){ free(od->encValLen); } if(LOG){ debug(LOW, 0, "After freeing encValLen\n"); }
if(od != NULL){ free(od); } if(LOG){ debug(LOW, 0, "After freeing od\n"); }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to encode a character buffer by Vajra */
/* Input: */
/* Output: */
void encodeBuffer(CD* od){
if(LOG){ debug(HIGH, 1, "(encodeBuffer)len: %d\n", od->inBufLen); }

int i = 0, j = 0, k = 0, l = 0, curChar = -1, distinctCharCount = 0;

//Find the order for encoding characters, based on their frequency
ORDER vj_order[MAX_CHARS];
findEncodingOrder(od->logLevel, od->inBuf, od->inBufLen, vj_order);

if(LOG){
    if(LOG){ debug(LOW, 1, "(encodeBuffer)Order of characters as per their decreasing order of frequency: \n"); }
    for(i = 0; i < MAX_CHARS; i++){ if(vj_order[i].count != 0){ if(LOG){ debug(LOW, 2, "ch: %d, count: %d ", vj_order[i].ch, vj_order[i].count); } if(LOG){ debug(LOW, 1, "order: %d\n", vj_order[i].ch); } } }
    if(LOG){ debug(LOW, 1, "(encodeBuffer)Order of characters as per their decreasing order of frequency: \n"); }
    for(i = 0; i < MAX_CHARS; i++){ if(vj_order[i].count != 0){ if(LOG){ debug(LOW, 1, "%d ", vj_order[i].ch); } } }
    if(LOG){ debug(LOW, 1, "\n"); }
}

//Obtain the relative positions based on the above frequency based order
int* rp[MAX_CHARS];
int rpCount[MAX_CHARS];

//Allocate and Initialise the Relative Positions array
initializeRelativePos(od->logLevel, rp, rpCount, vj_order);
if(LOG){
	for(i = 0; i < MAX_CHARS; i++){
		if(LOG){ debug(LOW, 1, "(encodeBuffer)rpCount[%d]: %d\n", i, rpCount[i]); }
	}
}

//Fill up the Relative Positions
fillRelativePos(od->logLevel, od->inBuf, od->inBufLen, vj_order, rp, rpCount);

//Display the Relative Positions
if(LOG){ displayRelativePos(od->logLevel, rp, rpCount, vj_order, od->inBufLen); }

//Find the Rank for every character
int charsRank[MAX_CHARS];
memset(charsRank, 0, sizeof(charsRank));
for(i = 0; i < MAX_CHARS; i++){ charsRank[vj_order[i].ch] = i + 1; if(vj_order[i].count != 0){ distinctCharCount++; } }

//Find the N count for every character
int charsNCount[MAX_CHARS];
findNCount(od->logLevel, vj_order, od->inBufLen, rpCount, charsNCount);
if(LOG){ displayNCount(od->logLevel, vj_order, charsNCount); }

//Update outfile and outbuffer pointers
//od = (CD*)malloc(sizeof(CD));
od->inCharCount = od->inBufLen;
od->charData = (unsigned char*)malloc(sizeof(unsigned char)*od->inCharCount); memset(od->charData, 0, sizeof(unsigned char)*od->inCharCount); od->charDataCount = 0;
od->of = (FILE*)openFile(od->logLevel, "outEnc.txt", "wb");
od->ofDataLen = 0;
od->fileBuf = (unsigned char*)malloc(sizeof(unsigned char)*od->inBufLen*2); memset(od->fileBuf, 0, sizeof(unsigned char)*od->inBufLen*2); od->fileBufLen = 0;
od->outBuf = (unsigned char*)malloc(sizeof(unsigned char)*od->inBufLen*2); memset(od->outBuf, 0, sizeof(unsigned char)*od->inBufLen*2); od->outBufLen = 0;
od->partialBytesBuf = (unsigned char*)malloc(sizeof(unsigned char)*od->inBufLen); //check len*2
memset(od->partialBytesBuf, 0, sizeof(unsigned char)*od->inBufLen); od->partialBytesBufLen = 0;
od->emptyBytesCount = 0;
od->distinctCharCount = distinctCharCount;
od->nCountBuf = (unsigned char*)malloc(sizeof(unsigned int)*8*od->distinctCharCount);
memset(od->nCountBuf, 0, sizeof(unsigned int)*od->distinctCharCount);
od->nCountBufLen = 0;
od->rCountBuf = (unsigned int*)malloc(sizeof(unsigned int)*(od->distinctCharCount+1));
memset(od->rCountBuf, 0, sizeof(unsigned int)*(od->distinctCharCount+1));
od->rCountBufLen = 0;
od->firstValDec = (unsigned char*)malloc(sizeof(unsigned char)*od->distinctCharCount*4);
memset(od->firstValDec, 0, sizeof(unsigned char)*od->distinctCharCount*4);
od->firstValDecLen = 0;
od->encValLen = (unsigned int*)malloc(sizeof(unsigned int)*od->distinctCharCount*4);
memset(od->encValLen, 0, sizeof(unsigned int)*od->distinctCharCount*4);
od->encValCount = 0;


struct timeval t1, t2, tDiff;
//long long tt = 0LL, ttSum = 0LL;
uint64_t tt = 0LL, ttSum = 0LL;
//Encode the relative positions by vajra without using density
for(i = 0; i < MAX_CHARS; i++){
    if(vj_order[i].count != 0){
        curChar = vj_order[i].ch;
        if(FULL_GMP){
            if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
            //printf("Current time is %ld seconds and %ld microseconds\n", t1.tv_sec, t1.tv_usec);
            //if(curChar == 130){
                applyDensity(curChar, rp[curChar], rpCount[curChar], charsNCount[curChar], od);
            //}
            encodePos(curChar, rp[curChar], rpCount[curChar], charsNCount[curChar], od->inBufLen, od);
            if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
            //printf("Current time is %ld seconds and %ld microseconds\n", t2.tv_sec, t2.tv_usec);
            if(TL){ ttSum += tt = timeval_diff(NULL, &t2, &t1); }
            if(TL){ printf("For char %d TIME: %lld ", curChar, tt); printf("sum: %lld\n", ttSum); }
        }

        if(!FULL_GMP){
        //if(curChar == 0){
        //if(curChar == 188){
        //if(curChar == 32){
        //if(curChar == 244){
        {
            int* nRP = (int*)malloc(sizeof(int)*(rpCount[curChar]+1)); memset(nRP, 0, sizeof(int)*(rpCount[curChar]+1));
            int l = 0; nRP[0] = 1; for(l = 0; l < rpCount[curChar]; l++){ nRP[l+1] = rp[curChar][l]+1; }
            struct timeval t1, t2; 
            if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
            superEncodePos(od, curChar, nRP, rpCount[curChar]+1, charsNCount[curChar]+1);
            //superEncodePos(od->logLevel, rp[curChar], rpCount[curChar], charsNCount[curChar]);
            //superEncodePosSettle(od->logLevel, nRP, rpCount[curChar]+1, charsNCount[curChar]+1);
            if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
            //if(TL){ printf("For char %d time taken by super : %lld\n", curChar, (uint32_t)timeval_diff(NULL, &t2, &t1)); }
            if(TL){ printf("For char %d time taken by super : %lld\n", curChar, (uint64_t)timeval_diff(NULL, &t2, &t1)); }
            if(nRP != NULL){ free(nRP); }
        }
        //}
        }
    }
}

finalizeNCountInBuf(od);
finalizePartialBytes(od);
//extractNCountFromBuf(od->nCountBytes, od->nCountBufLen, len);

//Encode the relative positions by vajra using density

if(LOG){ debug(od->logLevel, 1, "\n"); }

//Write vajra compression identifier
unsigned char vajraId[2] = { 'V', 'J'};
memcpy(od->fileBuf + od->fileBufLen, &vajraId[0], sizeof(unsigned char)); od->fileBufLen += sizeof(unsigned char);
memcpy(od->fileBuf + od->fileBufLen, &vajraId[1], sizeof(unsigned char)); od->fileBufLen += sizeof(unsigned char);
if(LOG){ debug(od->logLevel, 1, "WROTE vajra identifier %c%c using %d bytes\n", vajraId[0], vajraId[1], sizeof(unsigned char)*2); }

//Write total file size of outBuf
memcpy(od->fileBuf + od->fileBufLen, &od->inCharCount, sizeof(unsigned int)); od->fileBufLen += sizeof(unsigned int);
if(LOG){ debug(od->logLevel, 1, "WROTE fileSize %d using %d bytes\n", od->inCharCount, sizeof(unsigned int)); }

//Write total number of distinct characters
memcpy(od->fileBuf + od->fileBufLen, &od->charDataCount, sizeof(unsigned short)); od->fileBufLen += sizeof(unsigned short);
if(LOG){ debug(od->logLevel, 1, "WROTE number of distinct characters %d using %d bytes\n", od->charDataCount, sizeof(unsigned short)); }

//Write the distinct characters
memcpy(od->fileBuf + od->fileBufLen, od->charData, sizeof(unsigned char)*od->charDataCount); od->fileBufLen += sizeof(unsigned char)*od->charDataCount;
if(LOG){ debug(od->logLevel, 1, "WROTE charData using %d bytes\n", sizeof(unsigned char)*od->charDataCount); }
if(LOG){ displayEncCharData(od); }

//Write the length of nCount(nCountBufLen) as the number of bits
memcpy(od->fileBuf + od->fileBufLen, &od->nCountBufLen, sizeof(unsigned short)); od->fileBufLen += sizeof(unsigned short);
if(LOG){ debug(od->logLevel, 1, "WROTE nCountBufLen %d using %d bytes\n", od->nCountBufLen, sizeof(unsigned short)); }

//Write the nCountBytes 
memcpy(od->fileBuf + od->fileBufLen, od->nCountBytes, sizeof(unsigned char)*(int)ceil((float)od->nCountBufLen/(float)8)); 
od->fileBufLen += sizeof(unsigned char)*(int)ceil((float)od->nCountBufLen/(float)8);
if(LOG){ debug(od->logLevel, 1, "WROTE nCountBytes %d bytes\n", sizeof(unsigned char)*(int)ceil((float)od->nCountBufLen/(float)8)); }

//Write the rCountBuf
memcpy(od->fileBuf + od->fileBufLen, od->rCountBuf, sizeof(unsigned int)*od->rCountBufLen); 
od->fileBufLen += sizeof(unsigned int)*od->rCountBufLen;
if(LOG){ debug(od->logLevel, 1, "WROTE rCountBuf %d bytes\n", sizeof(unsigned int)*od->rCountBufLen); }

if(FULL_GMP){
    //Write the length of partialBytesBuf as the number of bits
    memcpy(od->fileBuf + od->fileBufLen, &od->partialBytesBufLen, sizeof(unsigned short)); od->fileBufLen += sizeof(unsigned short);
    if(LOG){ debug(od->logLevel, 1, "WROTE partialBytesBufLen %d using %d bytes\n", od->partialBytesBufLen, sizeof(unsigned short)); }

    //Write the partialBytes 
    memcpy(od->fileBuf + od->fileBufLen, od->partialBytes, sizeof(unsigned char)*(int)ceil((float)od->partialBytesBufLen/(float)8)); 
    od->fileBufLen += sizeof(unsigned char)*(int)ceil((float)od->partialBytesBufLen/(float)8);
    if(LOG){ debug(od->logLevel, 1, "WROTE partialBytes %d bytes\n", sizeof(unsigned char)*(int)ceil((float)od->partialBytesBufLen/(float)8)); }
}

if(!FULL_GMP){
    //Write the firstValDecLen
    memcpy(od->fileBuf + od->fileBufLen, &od->firstValDecLen, sizeof(unsigned short)); od->fileBufLen += sizeof(unsigned short);
    if(LOG){ debug(od->logLevel, 1, "WROTE the number of encoded gmp values %d using %d bytes\n", od->firstValDecLen, sizeof(unsigned short)); }

    //Write the firstValDec values
    memcpy(od->fileBuf + od->fileBufLen, od->firstValDec, sizeof(unsigned char)*(od->firstValDecLen)); od->fileBufLen += sizeof(unsigned char)*(od->firstValDecLen);
    if(LOG){ debug(od->logLevel, 1, "WROTE gmp firstValDec data using %d bytes\n", sizeof(unsigned char)*(od->firstValDecLen)); }

    //Write the lengths of encoded values
    memcpy(od->fileBuf + od->fileBufLen, od->encValLen, sizeof(unsigned int)*(od->encValCount)); od->fileBufLen += sizeof(unsigned int)*(od->encValCount);
    if(LOG){ debug(od->logLevel, 1, "WROTE the lengths of encoded values using %d bytes\n", sizeof(unsigned int)*(od->encValCount)); }
}

//Write the length of gmp encoded outBuf
memcpy(od->fileBuf + od->fileBufLen, &od->outBufLen, sizeof(unsigned int)); od->fileBufLen += sizeof(unsigned int);
if(LOG){ debug(od->logLevel, 1, "WROTE length of gmp encoded outBuf %d using %d bytes\n", od->outBufLen, sizeof(unsigned int)); }

printf("first Byte: %u\n", od->outBuf[0]);

//Write the gmp encoded outbuf
memcpy(od->fileBuf + od->fileBufLen, od->outBuf, sizeof(unsigned char)*(od->outBufLen)); od->fileBufLen += sizeof(unsigned char)*(od->outBufLen);
if(LOG){ debug(od->logLevel, 1, "WROTE gmp encoded outBuf using %d bytes\n", sizeof(unsigned char)*(od->outBufLen)); }

//Finally write the fileBuf into the file
fwrite(od->fileBuf, sizeof(unsigned char), od->fileBufLen, od->of);
if(LOG){ debug(HIGH, 1, "\n%d bytes of encoded data written to file\n", od->fileBufLen); }
printf("%s file with %d bytes successfully compressed to %d bytes\n", od->filename, od->inBufLen, od->fileBufLen);

if(LOG){ debug(od->logLevel, 1, "\nWROTE inCharCount: %d, distinctCharCount: %d, ofDataLen: %d, fileBufLen: %d, outBufLen: %d, partialBytesBufLen in bits: %d, emptyBytesCount: %d, rCountBufLen: %d\n", od->inCharCount, distinctCharCount, od->ofDataLen, od->fileBufLen, od->outBufLen, od->partialBytesBufLen, od->emptyBytesCount, od->rCountBufLen*2); }

//Free the relative positions
if(LOG){ 
    if(LOG){ debug(od->logLevel, 0, "\n"); }
    for(i = 0; i < MAX_CHARS; i++){
        if(rpCount[i] != 0 && rp[i] != NULL){ if(LOG){ debug(LOW, 1, "(encodeBuffer)Before Freeing rp[%d] ", i); } free(rp[i]); if(LOG){ debug(LOW, 1, "(encodeBuffer)After Freed rp[%d]\n", i); } }
    }
}

//Cleanup all the allocations
encCleanup(od);
if(LOG){ debug(HIGH, 0, "Everything cleaned up\n"); }
}
