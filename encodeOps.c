#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include "gmp.h"
#include "misc.h"
#include "encodeOps.h"
#include "ncrValues.h"


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
void initializeRelativePos(int logging, unsigned int* rp[MAX_CHARS], unsigned int rpCount[MAX_CHARS], ORDER vj_order[]){
if(LOG){ debug(HIGH, 0, "(initializeRelativePos)Start\n"); }
int i = 0, curChar = -1;
for(i = 0; i < MAX_CHARS; i++){
	rpCount[i] = 0;
        if(vj_order[i].count != 0){
                rp[vj_order[i].ch] = (unsigned int*)malloc(sizeof(unsigned int)*(vj_order[i].count + 1));
                memset(rp[vj_order[i].ch], 0, sizeof(unsigned int)*(vj_order[i].count + 1));
        } else { rp[vj_order[i].ch] = NULL; }
}
if(LOG){ debug(HIGH, 0, "(initializeRelativePos)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to record the relative position of characters as per their frequency order */
/* Input: */
/* Output: */
void fillRelativePos(int logging, unsigned char* inBuf, int len, ORDER vj_order[], int* rp[MAX_CHARS], int rpCount[]){
if(LOG){ debug(VERY_HIGH, 1, "\n(fillRelativePos)Start:len: %d\n", len); }

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
        if(curChar == vj_order[0].ch){
            tempSum = i+1;
        } else {
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
        }
        if(LOG){ debug(VERY_LOW, 2, "curChar: %d, relPos: %d\n", inBuf[i], tempSum); }
        rp[curChar][rpCount[curChar]] = tempSum;
        rpCount[curChar]++;
        prevChar = curChar;
}
if(LOG){ debug(VERY_HIGH, 0, "(fillRelativePos)End\n"); }

}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to record the relative position of characters as per their frequency order */
/* Input: */
/* Output: */
void fillRelativePosFast(int logging, unsigned char* inBuf, int len, ORDER vj_order[], unsigned int* rp[MAX_CHARS], unsigned int rpCount[]){
//void fillRelativePosFast(int logging, unsigned char* inBuf, int len, ORDER vj_order[]){
if(LOG){ debug(VERY_HIGH, 1, "\n(fillRelativePos)Start:len: %d\n", len); }

int charsRank[MAX_CHARS], charsCount[MAX_CHARS], uniqueCharCount = 0;
unsigned char charsAsPerRank[MAX_CHARS];
register int i = 0, j = 0, tempSum = 0, prevChar = -1;
register unsigned char curChar = 0;
memset(charsAsPerRank, 0, sizeof(charsAsPerRank));
memset(charsRank, 0, sizeof(charsRank));
memset(charsCount, 0, sizeof(charsCount));

for(i = 0; i < MAX_CHARS; i++){
    if(vj_order[i].count != 0){
       charsAsPerRank[uniqueCharCount++] = vj_order[i].ch;
    } 
}

//Obtain the rank of characters as per their frequency order, i.e character with most occurance gets the rank 1 and so on...
for(i = 0; i < MAX_CHARS; i++){ charsRank[vj_order[i].ch] = i + 1;  }
//if(LOG){ debug(LOW, 0, "(fillRelativePos)Ranks: "); for(i = 0; i < MAX_CHARS; i++){ if(LOG){ debug(LOW, 1, "%d ", charsRank[i]); } } if(LOG){ debug(LOW, 0, "\n"); } }

//Normal Relative Positions as per rank
for(i = 0; i < len; i++){
    curChar = inBuf[i];
    charsCount[curChar]++;
    if(curChar == vj_order[0].ch){ //Fast: if first char, then skip the steps of adding the character counts, is redundant
       tempSum = i+1;
    } else {
       if(curChar == prevChar){ //Fast: if prev and cur character are same, then just increment the rp by 1
          tempSum += 1;
       } else {
          tempSum = 0;
          if(charsRank[curChar] <= uniqueCharCount - (charsRank[curChar]-1)){ //Fast: add the character counts in the direction where less additions are required
             for(j = 0; j < charsRank[curChar]-1; j++){                       //e.g. 0,1,2,3,4,5..if curChar is 2, then it is better to add 0 and 1 subtract it from the current i
                tempSum += charsCount[charsAsPerRank[j]];                     //rather than adding 2,3,4 and 5
             }
             tempSum = i+1 - tempSum;
          } else {
             for(j = charsRank[curChar]-1; j < uniqueCharCount; j++){
                tempSum += charsCount[charsAsPerRank[j]];
             }
          }
    }
}
//if(LOG){ debug(VERY_LOW, 2, "curChar: %d, relPos: %d\n", inBuf[i], tempSum); }
rp[curChar][rpCount[curChar]++] = tempSum;
//if(rpCount[curChar] > vj_order[curChar].count){ printf("FATAL ERROR: rpCount exceeds vj_order.count\n"); }
prevChar = curChar;
}
if(LOG){ debug(VERY_HIGH, 0, "(fillRelativePos)End\n"); }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to display the relative positions of all the characters */
/* Input: */
/* Output: */
void displayRelativePos(int logging, int* rp[MAX_CHARS], int rpCount[], ORDER vj_order[], int len){
if(LOG){ debug(VERY_HIGH, 0, "\n(dispayRelativePos)Start"); }
int i = 0, j = 0, charsNCount[MAX_CHARS], tempNActual = 0;
for(i = 0; i < MAX_CHARS; i++){
        if(vj_order[i].count != 0){
                if(LOG){
		    if(LOG){ debug(HIGH, 2, "\nKNKNrp[%d](%d): ", vj_order[i].ch, rpCount[vj_order[i].ch]); }
                    for(j = 0; j < rpCount[vj_order[i].ch]; j++){ if(LOG){ debug(HIGH, 1, "%d ", rp[vj_order[i].ch][j]); } }
		}
                charsNCount[vj_order[i].ch] = len - tempNActual;
                tempNActual += rpCount[vj_order[i].ch];
                if(LOG){ debug(HIGH, 1, " (%d)", charsNCount[vj_order[i].ch]); }
        }
}
if(LOG){ debug(VERY_HIGH, 0, "\n(dispayRelativePos)End\n"); }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to find the N Count for all the characters */
/* Input: */
/* Output: */
void findNCount(int logging, ORDER vj_order[], int len, int rpCount[], int charsNCount[]){
if(LOG){ debug(VERY_HIGH, 1, "\n(findNCount)Start:len: %d\n", len); }

int i = 0, tempNActual = 0;
for(i = 0; i < MAX_CHARS; i++){
    if(vj_order[i].count != 0){
        charsNCount[vj_order[i].ch] = len - tempNActual;
        tempNActual += rpCount[vj_order[i].ch];
        if(LOG){ debug(LOW, 1, " (%d)", charsNCount[vj_order[i].ch]); }
    }
}
if(LOG){ debug(VERY_HIGH, 1, "\n(findNCount)End\n"); }
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
if(od->charDataCount > od->charDataAllocSize){ printf("FATAL ERROR: charDataCount exceeds charDataAllocSize\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to add r value to output buffer short values */
/* Input: */
/* Output: */
void writeRCountToBuf(unsigned int rCount, CD* od){
#ifdef DEBUG
if(LOG){ debug(HIGH, 1, "\n(writeRCountToBuf)Start: rCount: %d\n", rCount); }
#endif
od->rCountBuf[od->rCountBufLen++] = rCount;
if(od->rCountBufLen*sizeof(unsigned int) > od->rCountBufAllocSize){ printf("FATAL ERROR: rCountBufLen exceeds rCountBufAllocSize\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to add n value to output buffer */
/* Input: */
/* Output: */
void writeNCountToBuf(int nCount, CD* od){
#ifdef DEBUG
if(LOG){ debug(HIGH, 1, "\n(writeNCountToBuf)Start: nCount: %d\n", nCount); }
#endif
static int lastN = 0;
int i = 0, tempNCount = nCount, bcLastN = 0, bcCurN = bitCount(nCount);
if(od->nCountBufLen == 0){ lastN = od->inCharCount; od->nCountBuf[(od->nCountBufLen)++] = 1; } //For the first n
else { //For subsequent n
    for(i = 0; i < bcCurN; i++){
        if(tempNCount & 1){
            od->nCountBuf[(od->nCountBufLen)++] = 1;
            if(od->nCountBufLen > od->nCountBufAllocSize){ printf("FATAL ERROR: nCountBufLen exceeds nCountBufAllocSize\n"); }
            if(LOG){ debug(VERY_LOW, 0, "ONE "); }
        } else {
            od->nCountBuf[(od->nCountBufLen)++] = 0;
            if(od->nCountBufLen > od->nCountBufAllocSize){ printf("FATAL ERROR: nCountBufLen exceeds nCountBufAllocSize\n"); }
            if(LOG){ debug(VERY_LOW, 0, "ZERO "); }
        }
        tempNCount >>= 1;
    }
    bcLastN = bitCount(lastN);
    for(i = bcCurN; i < bcLastN; i++){
        od->nCountBuf[(od->nCountBufLen)++] = 0;
        if(od->nCountBufLen > od->nCountBufAllocSize){ printf("FATAL ERROR: nCountBufLen exceeds nCountBufAllocSize\n"); }
        if(LOG){ debug(VERY_LOW, 0, "ZERO "); }
    }
}
#ifdef DEBUG
if(LOG){ debug(MEDIUM, 1, "lastN: %d, bitCount: %d\n", lastN, bitCount(lastN)); }
if(LOG){ debug(LOW, 1, "Current nCountBufLen: %d\n", od->nCountBufLen); }
if(LOG){ debug(HIGH, 1, "\n(writeNCountToBuf)End\n"); }
#endif
lastN = nCount;
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
void finalizeSplitDensityIdBits(CD* od){
if(LOG){ debug(HIGH, 1, "\n(finalizeSplitDensityIdBits)Start\n"); }
int i = 0, tempCount = 0;
//printf("splitCharsCount: %d\n", od->splitCharsCount);
//printf("splitDensityIdBitsCount: %d\n", od->splitDensityIdBitsCount);
//for(i = 0; i < od->splitDensityIdBitsCount; i++){ printf("%d ", od->splitDensityIdBits[i]); } printf("\n");
od->splitDensityIdBytesLen = 0;
od->splitDensityIdBytes = (unsigned char*)malloc(sizeof(unsigned char)*(od->splitCharsCount*3));
memset(od->splitDensityIdBytes, 0, sizeof(unsigned char)*(od->splitCharsCount*3));
unsigned char tempChar = 0, tempByte = 0;
for(i = od->splitDensityIdBitsCount-1; i >= 0; i--){
    if(LOG){ debug(HIGH, 1, "%d ", od->splitDensityIdBits[i]); }
    if(od->splitDensityIdBits[i] == 1){
        tempChar = 1;
    } else { tempChar = 0; }
    tempChar <<= tempCount;
    tempCount++; tempByte += tempChar;
    if(tempCount == 8){ 
        od->splitDensityIdBytes[od->splitDensityIdBytesLen++] = tempByte;
        tempByte = 0; tempCount = 0;
    } else {
        if(i == 0){ //write the partial byte
            od->splitDensityIdBytes[od->splitDensityIdBytesLen++] = tempByte;
            tempByte = 0; tempCount = 0;
        }
    }
}
//printf("splitDensityIdBytesLen: %d\n", od->splitDensityIdBytesLen);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to convert partialBytesBuf array of 0 and 1 into bytes */
/* Input: */
/* Output: */
void finalizeSplitDensityIdBitsOld(CD* od){
if(LOG){ debug(HIGH, 1, "\n(finalizeSplitDensityIdBits)Start\n"); }
int i = 0, tempCount = 0;
printf("splitCharsCount: %d\n", od->splitCharsCount);
printf("splitDensityIdBitsCount: %d\n", od->splitDensityIdBitsCount);
for(i = 0; i < od->splitDensityIdBitsCount; i++){ printf("%d ", od->splitDensityIdBits[i]); } printf("\n");
od->splitDensityIdBytesLen = 0;
od->splitDensityIdBytes = (unsigned char*)malloc(sizeof(unsigned char)*(int)ceil((float)(od->splitCharsCount*3)/(float)8));
memset(od->splitDensityIdBytes, 0, sizeof(unsigned char)*(int)ceil((float)(od->splitCharsCount*3)/(float)8));
unsigned char tempChar = 0;
for(i = od->splitDensityIdBitsCount-1; i >= 0; i--){
    if(LOG){ debug(HIGH, 1, "%d ", od->splitDensityIdBits[i]); }
    if(od->splitDensityIdBits[i] == 1){
        tempChar = 1;
        if((i+1) % 8 != 0){ tempChar <<= ((i+1) % 8) - 1; } else { tempChar <<= 7; }
        od->splitDensityIdBytes[(int)ceil((float)(i+1)/(float)8) - 1] += tempChar;
    }
}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to convert partialBytesBuf array of 0 and 1 into bytes */
/* Input: */
/* Output: */
/*
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
*/


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

if(posCount == 0){ if(LOG){ debug(LOW, 0, "EMPTY\n");}  } else { if(LOG){ debug(LOW, 0, "NONEMPTY\n"); } }

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
if(blockTypesCount == BLOCK_TYPES_COUNT_4){
    if(curDensity >= 75){ blockType = TYPE_4; }
    else if(curDensity >= 50){ blockType = TYPE_3; }
    else if(curDensity >= 25){ blockType = TYPE_2; }
    else { blockType = TYPE_1; }
}

if(LOG){ debug(HIGH, 1, "(findBlockType)End blockType: %d\n", blockType); }
return blockType;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to display the contents of a block */
/* Input: */
/* Output: */
void displayBlocks(BD* bD){
int i = 0, j = 0;
for(i = 0; i < bD->blockTypesCount; i++){
    printf("Type%d: blockCount: %d, nCount: %d, rCount: %d -- ", i, bD->blockRPTypesCount[i], bD->blockRPNCount[i], bD->blockRPCount[i]);
    for(j = 0; j < bD->blockRPCount[i]; j++){
        printf("%d ", bD->blockRP[i][j]);
    }
    printf("\n");
}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to verify the blocks by combining them and comparing them with the original as 0 and 1(slow) */
/* Input: */
/* Output: */
void verifyBlocksSlow(int* rp, int rCount, BD* bD){
if(LOG){ debug(HIGH, 1, "\n(verifyBlocksBad)Start\n"); }
int i = 0, j = 0;
unsigned char* tempBuf[8]; int tempBufLen[8], tempTypesCount[8];
int offset[8], blockType = -1, finalBufLen = 0, finalBufOffset = 0;
//Initialize
for(i = 0; i < 8; i++){
    if(bD->blockRPNCount[i] != 0){
    tempBuf[i] = (unsigned char*)malloc(sizeof(unsigned char)*bD->blockRPNCount[i]);
    memset(tempBuf[i], 1, sizeof(unsigned char)*bD->blockRPNCount[i]);
    finalBufLen += bD->blockRPNCount[i];
    } else { tempBuf[i] = NULL; }
    tempBufLen[i] = 0; offset[i] = 0; tempTypesCount[i] = 0;
}

unsigned char* finalBuf = (unsigned char*)malloc(sizeof(unsigned char)*finalBufLen);
memset(finalBuf, 0, sizeof(unsigned char)*finalBufLen);
finalBufLen = 0;

//Convert them as 0 and 1
for(i = 0; i < 8; i++){
    if(bD->blockRPNCount[i] != 0){
        for(j = 0; j < bD->blockRPCount[i]; j++){
            tempBuf[i][bD->blockRP[i][j]-1] = 0;
        }
        tempBufLen[i] = bD->blockRPNCount[i];
    }
}

//Display
for(i = 0; i < 8; i++){
    printf("Type%d: ", i); for(j = 0; j < tempBufLen[i]; j++){ printf("%d", tempBuf[i][j]); } printf("\n");
}

printf("\nblockIdNK:");
for(i = 0; i < bD->blockIdLen; i++){
    printf("%d ", bD->blockId[i]);
}
printf("\n");

for(i = 0; i < bD->blockIdLen; i++){
    blockType = bD->blockId[i]; tempTypesCount[blockType]++;
    if(tempTypesCount[blockType] == bD->blockRPTypesCount[blockType]){
        memcpy(finalBuf + finalBufOffset, tempBuf[blockType] + offset[blockType], sizeof(unsigned char)*(bD->blockRPNCount[blockType]-(bD->blockRPTypesCount[blockType]-1)*64));
        finalBufOffset += sizeof(unsigned char)*(bD->blockRPNCount[blockType]-(bD->blockRPTypesCount[blockType]-1)*64);
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
        printf("%d - %d ", rp[testRPCount-1], testRP[testRPCount-1]);
        if(rp[testRPCount-1] != testRP[testRPCount-1]){ printf(" verify failure...\n"); }
    }
}
printf("\n");

/*for(i = 0; i < rCount; i++){
    //printf("%d - %d \n", rp[i], testRP[i]);
    if(rp[i] != testRP[i]){ printf("verify failure...\n"); }
}*/
//Free
for(i = 0; i < 8; i++){
    if(tempBuf[i] != NULL && bD->blockRPNCount[i] != 0){ free(tempBuf[i]); }
}
if(finalBuf != NULL){ free(finalBuf); }
if(testRP != NULL){ free(testRP); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to verify the blocks by combining them and comparing them with the original using only positions(fast) */
/* Input: */
/* Output: */
void verifyBlocks(int* rp, int rCount, BD* bD){
if(LOG){ debug(HIGH, 1, "\n(verifyBlocks)Start allBlocksRPCount: %d\n", bD->allBlocksRPCount); }
int i = 0, j = 0;
int* testRP = (int*)malloc(sizeof(int)*rCount); memset(testRP, 0, sizeof(int)*rCount);
int testRPCount = 0, blockType = -1;
int tempTypesCount[bD->blockTypesCount]; memset(tempTypesCount, 0, sizeof(tempTypesCount));
int offset[bD->blockTypesCount]; memset(offset, 0, sizeof(offset));
int tempRunningCount = 0, tempRCount = 0;

for(i = 0; i < bD->blockIdLen; i++){
    if(LOG){ debug(VERY_LOW, 0, "\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"); }
    blockType = bD->blockId[i]; tempTypesCount[blockType]++; tempRunningCount++;
    if(LOG){ debug(VERY_LOW, 1, "-->%d -- ", (tempTypesCount[blockType]-1)*bD->blockSize + 1); }
    if(tempRunningCount == bD->blockCount){
        while((bD->blockRP[blockType][offset[blockType]] > (tempTypesCount[blockType]-1)*bD->blockSize && bD->blockRP[blockType][offset[blockType]] <= (bD->blockRPNCount[blockType]-(bD->blockRPTypesCount[blockType]-1)*bD->blockSize)) && tempRCount < bD->allBlocksRPCount){
            tempRCount++;
            if(LOG){ debug(VERY_LOW, 3, "(%d) (%d) (%d)", bD->blockRP[blockType][offset[blockType]], bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize, (tempRunningCount-1)*bD->blockSize + (bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize)); }
            testRP[testRPCount++] = (tempRunningCount-1)*bD->blockSize + (bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize);
            if(rp[testRPCount-1] != testRP[testRPCount-1]){ printf("%d - %d verify failure...\n", rp[testRPCount-1], testRP[testRPCount-1]); }
            offset[blockType]++;
        }
    } else {
        while(bD->blockRP[blockType][offset[blockType]] > (tempTypesCount[blockType]-1)*bD->blockSize && bD->blockRP[blockType][offset[blockType]] <= tempTypesCount[blockType]*bD->blockSize){
            tempRCount++;
            if(LOG){ debug(VERY_LOW, 3, "(%d) (%d) (%d) ", bD->blockRP[blockType][offset[blockType]], bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize, (tempRunningCount-1)*bD->blockSize + (bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize)); }
            testRP[testRPCount++] = (tempRunningCount-1)*bD->blockSize + (bD->blockRP[blockType][offset[blockType]] - (tempTypesCount[blockType]-1)*bD->blockSize);
            if(rp[testRPCount-1] != testRP[testRPCount-1]){ printf("%d - %d verify failure...\n", rp[testRPCount-1], testRP[testRPCount-1]); }
            offset[blockType]++;
        }
    }
    if(LOG){ debug(VERY_LOW, 2, " -- %d<-- %d ", tempTypesCount[blockType]*bD->blockSize, tempRunningCount*bD->blockSize); }
}

if(LOG){ 
    debug(VERY_LOW, 0, "\nblockId:");
    for(i = 0; i < bD->blockIdLen; i++){ if(LOG){ debug(VERY_LOW, 1, "%d ", bD->blockId[i]); } }
    debug(VERY_LOW, 0, "\n");
}

if(testRP != NULL){ free(testRP); }
if(LOG){ debug(HIGH, 1, "\n(verifyBlocks)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to update the blocks */
/* Input: */
/* Output: */
void updateBlockRP(int blockType, int blockSize, BD* bD, unsigned int* tempBlock, unsigned int tempBlockCount){
if(LOG){ debug(LOW, 1, "\n(updateBlockRP)Start: blockType: %d, blockStartPos: %d, tempBlockCount: %d\n", blockType, bD->blockStartPos, tempBlockCount); }
int i = 0, j = 0, k = 0, offset = (bD->blockRPTypesCount[blockType]-1)*bD->blockSize;
bD->blockRPNCount[blockType] += blockSize;
bD->allBlocksRPNCount += blockSize;
for(i = 0; i < tempBlockCount; i++){
    bD->blockRP[blockType][bD->blockRPCount[blockType]++] = offset + tempBlock[i] - bD->blockStartPos + 1;
    if(LOG){ debug(VERY_LOW, 1, "*%d*", offset + tempBlock[i] - bD->blockStartPos + 1); }
}
bD->allBlocksRPCount += tempBlockCount;
if(LOG){ debug(LOW, 1, "\n(updateBlockRP)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to display the temp block */
/* Input: */
/* Output: */
void displayTempBlock(unsigned int tempBlock[], unsigned int tempBlockCount){
int i = 0; 
//for(i = 0; i < tempBlockCount; i++){ printf("#%d#", tempBlock[i]); } 
/*
printf("\n");
unsigned char temp[D_BLOCK_SIZE+1]; memset(temp, 1, sizeof(temp));
int offset = (tempBlock[0]/D_BLOCK_SIZE)*D_BLOCK_SIZE;
for(i = 0; i < tempBlockCount; i++){ temp[tempBlock[i]-offset] = 0; }
for(i = 1; i < D_BLOCK_SIZE+1; i++){ 
    printf("%d", temp[i]); 
    if(i == D_BLOCK_SIZE/2){ printf(" | "); }
}
printf(" density: %d", tempBlockCount*100/D_BLOCK_SIZE);
*/
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to cleanup the block data */
/* Input: */
/* Output: */
void blockDataCleanup(BD* bD){
int i = 0;
if(LOG){ debug(LOW, 0, "Free blockData\n"); }
if(bD->blockId != NULL){ free(bD->blockId); }
for(i = 0; i < bD->blockTypesCount; i++){ if(bD->blockRP[i] != NULL){ free(bD->blockRP[i]); } }
if(bD->blockRP != NULL){ free(bD->blockRP); }
if(bD->blockRPCount != NULL){ free(bD->blockRPCount); }
if(bD->blockRPTypesCount != NULL){ free(bD->blockRPTypesCount); }
if(bD->blockRPNCount != NULL){ free(bD->blockRPNCount); }
if(bD != NULL){ free(bD); }
if(LOG){ debug(LOW, 0, "blockData freed\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to initialize the blockData */
/* Input: */
/* Output: */
void initializeBlockData(BD** bD, int nCount){
int i = 0;
(*bD) = (BD*)malloc(sizeof(BD));
(*bD)->blockSize = D_BLOCK_SIZE;
(*bD)->blockTypesCount = D_BLOCK_TYPES;
(*bD)->blockCount = 1;
(*bD)->blockId = (unsigned char*)malloc(sizeof(unsigned char)*((nCount/(*bD)->blockSize)+1)); memset((*bD)->blockId, 0, sizeof(unsigned char)*((nCount/(*bD)->blockSize)+1));
(*bD)->blockIdLen = 0;
(*bD)->blockRP = (unsigned int**)malloc(sizeof(unsigned int*)*((*bD)->blockTypesCount)); memset((*bD)->blockRP, 0, sizeof(unsigned int*)*((*bD)->blockTypesCount));
(*bD)->blockRPCount = (unsigned int*)malloc(sizeof(unsigned int)*(*bD)->blockTypesCount);
(*bD)->blockRPTypesCount = (unsigned int*)malloc(sizeof(unsigned int)*(*bD)->blockTypesCount);
(*bD)->blockRPNCount = (unsigned int*)malloc(sizeof(unsigned int)*(*bD)->blockTypesCount);
for(i = 0; i < (*bD)->blockTypesCount; i++){
    (*bD)->blockRP[i] = (unsigned int*)malloc(sizeof(unsigned int)*nCount); memset((*bD)->blockRP[i], 0, sizeof(unsigned int)*nCount);
    (*bD)->blockRPCount[i] = 0;
    (*bD)->blockRPTypesCount[i] = 0;
    (*bD)->blockRPNCount[i] = 0;
}
(*bD)->blockStartPos = 0;
(*bD)->allBlocksRPCount = 0;
(*bD)->allBlocksRPNCount = 0;
}


void displayBlockIds(BD* bD){
printf("(displayBlockIds)\n");
int i = 0, counts[bD->blockTypesCount];
for(i = 0; i < bD->blockTypesCount; i++){ counts[i] = 0; }
printf("BlockIds: ");
for(i = 0; i < bD->blockIdLen; i++){ 
    if(LOG){ printf("%d ", bD->blockId[i]); } 
    counts[bD->blockId[i]]++;
}
printf("\n");
printf("Total blocks: %d\n", bD->blockIdLen);
for(i = 0; i < bD->blockTypesCount; i++){
    printf("Type%d count: %d\n", i, counts[i]);
}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to apply density on positions and make different sets of positions */
/* Input: */
/* Output: */
void applyDensity(int curChar, int* rp, int rCount, int nCount, CD* od, BD* bD){
if(LOG){ debug(VERY_HIGH, 1, "\n(applyDensity)Start: curChar: %d, nCount: %d, rCount: %d\n", curChar, nCount, rCount); }

int blockSize = 64, blockTypesCount = 8, posCount = 0, blockType = -1;
int i = 0, j = 0, k = 0, l = 0;
int check = 0;

unsigned int tempBlock[blockSize], tempBlockCount = 0; memset(tempBlock, 0, sizeof(tempBlock));

bD->curChar = curChar;
if(LOG){ debug(LOW, 1, "%d - ", 1); }
bD->blockStartPos = 1;
for(i = 0; i < rCount; i++){
    if(rp[i] <= bD->blockSize*bD->blockCount){
        posCount++; tempBlock[tempBlockCount++] = rp[i];
        if(LOG){ debug(LOW, 1, "%d ", rp[i]); }
    } else {
        if(LOG){ debug(LOW, 1, " - %d  (%d) \n", bD->blockSize*bD->blockCount, posCount); }
        blockType = findBlockType(bD->blockSize, bD->blockTypesCount, posCount);
        bD->blockId[bD->blockIdLen++] = blockType;
        bD->blockRPTypesCount[blockType]++;
        displayTempBlock(tempBlock, tempBlockCount);
        //printf(" blockType: %d", blockType);
        updateBlockRP(blockType, blockSize, bD, tempBlock, tempBlockCount);

        if(LOG){ debug(LOW, 0, "************************************************\n"); }
        if(LOG){ debug(LOW, 1, "%d - ", bD->blockSize*bD->blockCount+1); }
        tempBlockCount = 0; 
        bD->blockStartPos = bD->blockSize*bD->blockCount+1;
        bD->blockCount++; 
        check += posCount;
        posCount = 0; i--;
    }
    if(i == rCount - 1){
        if(LOG){ debug(LOW, 1, " - %d  (%d) \n", bD->blockSize*bD->blockCount, posCount); }
        if(nCount > bD->blockSize*bD->blockCount){
            blockType = findBlockType(bD->blockSize, bD->blockTypesCount, posCount);
            bD->blockId[bD->blockIdLen++] = blockType;
            bD->blockRPTypesCount[blockType]++;
            displayTempBlock(tempBlock, tempBlockCount);
            //printf(" blockType: %d", blockType);
            updateBlockRP(blockType, blockSize, bD, tempBlock, tempBlockCount);
            if(LOG){ debug(LOW, 0, "************************************************\n"); }
            if(LOG){ debug(LOW, 1, "%d - ", bD->blockSize*bD->blockCount+1); }
            tempBlockCount = 0; 
            bD->blockStartPos = bD->blockSize*bD->blockCount+1;
            bD->blockCount++; 
            check += posCount; posCount = 0;

            //For the empty blocks in the end
            int emptyFullCount = (nCount-bD->blockSize*(bD->blockCount-1))/bD->blockSize;
            for(k = 0; k < emptyFullCount; k++){
                if(LOG){ debug(LOW, 1, " - %d  (%d) \n", bD->blockSize*bD->blockCount, posCount); }
                blockType = findBlockType(bD->blockSize, bD->blockTypesCount, 0);
                bD->blockId[bD->blockIdLen++] = blockType;
                bD->blockRPTypesCount[blockType]++;
                displayTempBlock(tempBlock, tempBlockCount);
                //printf(" blockType: %d", blockType);
                updateBlockRP(blockType, blockSize, bD, tempBlock, tempBlockCount);
                if(LOG){ debug(LOW, 0, "************************************************\n"); }
                if(LOG){ debug(LOW, 1, "%d - ",bD->blockSize*bD->blockCount+1); }
                tempBlockCount = 0; 
                bD->blockStartPos = bD->blockSize*bD->blockCount+1;
                bD->blockCount++; 
                check += posCount; posCount = 0;
            }

            //Last empty incomplete block
            if(nCount-bD->blockSize*(bD->blockCount-1) > 0){
                if(LOG){ debug(LOW, 1, " - %d  (%d) \n", bD->blockSize*bD->blockCount, posCount); }
                blockType = findBlockType(nCount-bD->blockSize*(bD->blockCount-1), bD->blockTypesCount, 0);
                bD->blockId[bD->blockIdLen++] = blockType;
                bD->blockRPTypesCount[blockType]++;
                displayTempBlock(tempBlock, tempBlockCount);
                //printf(" blockType: %d", blockType);
                updateBlockRP(blockType, nCount-bD->blockSize*(bD->blockCount-1), bD, tempBlock, tempBlockCount);
                if(LOG){ debug(LOW, 0, "************************************************\n"); }
                if(LOG){ debug(LOW, 1, "%d - ", bD->blockSize*bD->blockCount+1); }
                tempBlockCount = 0; 
                bD->blockStartPos = bD->blockSize*bD->blockCount+1;
                bD->blockCount++; 
                check += posCount; posCount = 0;
            }

        } else {
            blockType = findBlockType(nCount-bD->blockSize*(bD->blockCount-1), bD->blockTypesCount, posCount);
            bD->blockId[bD->blockIdLen++] = blockType;
            bD->blockRPTypesCount[blockType]++;
            displayTempBlock(tempBlock, tempBlockCount);
            //printf(" blockType: %d", blockType);
            updateBlockRP(blockType, nCount-bD->blockSize*(bD->blockCount-1), bD, tempBlock, tempBlockCount);
        }

        if(LOG){ debug(LOW, 0, "************************************************\n"); }
        check += posCount;
    }
    
}
if(check != rCount){ printf("check(%d), rCount(%d) density failure...\n", check, rCount); }

//displayBlocks(bD);
#ifdef DEBUG
displayBlockIds(bD);
#endif

//struct timeval t1, t2, t3, t4;
//if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
//verifyBlocks(rp, rCount, bD);
//if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
//if(TL){ printf("VERIFY GOOD TIME : %lld\n", (uint64_t)timeval_diff(NULL, &t2, &t1)); }
//
//if(TL){ if(gettimeofday(&t3, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
//verifyBlocksSlow(rp, rCount, bD);
//if(TL){ if(gettimeofday(&t4, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
//if(TL){ printf("VERIFY BAD TIME : %lld\n", (uint64_t)timeval_diff(NULL, &t4, &t3)); }

//Display block statistics
//for(i = 0; i < bD->blockTypesCount; i++){
//    if(LOG){ debug(MEDIUM, 3, "BlockType%d: nCount: %d, rCount: %d\n", i, bD->blockRPNCount[i], bD->blockRPCount[i]); }
//}

if(curChar == 0){
int z = 0;
for(z = 0; z < bD->blockTypesCount; z++){
    unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char)*bD->blockRPNCount[z]); memset(temp, 1, sizeof(unsigned char)*bD->blockRPNCount[z]);
    int i = 0;
    for(i = 0; i < bD->blockRPCount[z]; i++){ temp[bD->blockRP[z][i]-1] = 0; }
    char filename[256]; memset(filename, 0, sizeof(filename)); sprintf(filename, "type%d.txt", z);
    //printf("filename: %s\n", filename);
    FILE* f1 = (FILE*)openFile(HIGH, filename, "ab"); fwrite(temp, sizeof(unsigned char), bD->blockRPNCount[z], f1); fclose(f1);
    if(temp != NULL){ free(temp); }
}
}

if(LOG){ debug(VERY_HIGH, 1, "\n(applyDensity)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to encode a set of positions using vajra without using density */
/* Input: */
/* Output: */
void encodePos(int curChar, int* rp, int rCount, int nCount, int len, CD* od){
if(LOG){ debug(VERY_HIGH, 1, "\n(encodePos)Start: curChar: %d, nCount: %d, rCount: %d, len: %d\n", curChar, nCount, rCount, len); }

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
/* Function to round off the density to 1 place after decimal */
/* Input: */
/* Output: */
float trimDensity(float f){
char ch[4]; memset(ch, 0, sizeof(ch));
sprintf(ch, "%.1f", f);
return (float)(atof(ch));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to map percent value to ncrMap */
/* Input: */
/* Output: */
int mapPos(float f){
f *= 10;
f -= 1;
return (int)f;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to check the applicability of density */
/* Input: */
/* Output: */
int checkDensityApplicability(BD* bD, int curChar, int nCount, int rCount){
if(LOG){ debug(VERY_HIGH, 0, "(checkDensityApplicability)Start curChar: %d, nCount: %d, rCount: %d\n", curChar, nCount, rCount); }
float overallDensity = (float)(rCount * 100)/ (float)nCount;
float trimOverallDensity = trimDensity(overallDensity);
int ncrMapPos = mapPos(overallDensity);
int overallBitLen = approxBitLenNCR(nCount, rCount);
if(LOG){ debug(HIGH, 0, "overallDensity: %.6f, trimOverallDensity: %f, ncrMapPos: %d, revDensity: %.2f, overallBitLen: %d\n", overallDensity, trimOverallDensity, ncrMapPos, ncrMap[ncrMapPos], overallBitLen); }
float blockDensity[bD->blockTypesCount], trimBlockDensity[bD->blockTypesCount];
int blockBitLen[bD->blockTypesCount], blockBitLenSum = 0;
int i = 0;
for(i = 0; i < bD->blockTypesCount; i++){
    if(bD->blockRPNCount[i] != 0){
        blockBitLen[i] = approxBitLenNCR(bD->blockRPNCount[i], bD->blockRPCount[i]);
        blockDensity[i] = (float)(bD->blockRPCount[i] * 100)/ (float)bD->blockRPNCount[i];
        trimBlockDensity[i] = trimDensity(blockDensity[i]);
        ncrMapPos = mapPos(blockDensity[i]);
        //blockBitLen[i] = (bD->blockRPNCount[i] * ncrMap[ncrMapPos]) / 100;
    } else {
        blockDensity[i] = 0;
        trimBlockDensity[i] = 0;
        blockBitLen[i] = 0;
        ncrMapPos = 0;
    }
    blockBitLenSum += blockBitLen[i];
if(LOG){ debug(HIGH, 0, "blockN: %d, blockR: %d, blockDensity[%d]: %.6f, trimBlockDensity[%d]: %f, ncrMap Pos: %d, revDensity: %.2f, blockBitLen[%d]: %d\n", bD->blockRPNCount[i], bD->blockRPCount[i], i, blockDensity[i], i, trimBlockDensity[i]*10, ncrMapPos, ncrMap[ncrMapPos], i, blockBitLen[i]); }
}
int gain = overallBitLen - blockBitLenSum;
if(LOG){ debug(HIGH, 0, "overallBitLen: %d, blockBitLenSum: %d, diff: (%d) %f\n", overallBitLen, blockBitLenSum, gain, ((float)(gain)*100)/(float)overallBitLen); }
int overhead = bD->blockTypesCount*(2+2+4)*8 + bD->blockCount*2;
if(LOG){ debug(HIGH, 0, "overhead: %d(for extra n and r and enc value) + %d(for block identifiers) = %d\n", bD->blockTypesCount*(2+2+4)*8, bD->blockCount*2, overhead); }

if(gain > (int)((float)overhead * 1.2)){
    if(LOG){ debug(HIGH, 1, "Eligible for density application. %d\n", (int)((float)overhead * 1.2)); }
    return TRUE;
} else {
    return FALSE;
    if(LOG){ debug(HIGH, 1, "Not Eligible for density application.\n"); }
}

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to get the approx bit len of ncr */
/* Input: */
/* Output: */
int approxBitLenNCR(int n, int r){
float density = (float)(r * 100)/ (float)n;
return ((n * ncrMap[mapPos(density)]) / 100);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to get the approx len of blockId stats */
/* Input: */
/* Output: */
int approxBitLenBlockId(int blockCount, int blockStat[], int totalLen){
if(LOG){ debug(HIGH, 0, "(approxBitLenBlockId)Start blockCount: %d, totalLen: %d\n", blockCount, totalLen); }
int i = 0, sum = 0, bitLen = 0, bitLenSum = 0;
for(i = 0; i < blockCount-1; i++){
    bitLenSum += bitLen = approxBitLenNCR(totalLen-sum, blockStat[i]);
    if(LOG){ debug(HIGH, 3, "n: %d, r: %d, approxBitLenNCR: %d\n", totalLen - sum, blockStat[i], bitLen); }
    sum += blockStat[i];
}
if(LOG){ debug(HIGH, 1, "bitLenSum: %d\n", bitLenSum); }
return bitLenSum;
}

void compressCombinedBlockId(unsigned char* combinedBlockId, unsigned int combinedBlockIdLen, CD* od){
if(LOG){ debug(VERY_HIGH, 0, "(compressCombinedBlockId)Start combinedBlockIdLen: %d\n", combinedBlockIdLen); }
int i = 0, origStat[D_BLOCK_TYPES], mtfStat[D_BLOCK_TYPES];
memset(origStat, 0, sizeof(origStat)); memset(mtfStat, 0, sizeof(mtfStat));
for(i = 0; i < combinedBlockIdLen; i++){ origStat[combinedBlockId[i]]++; }

unsigned char* mtfBuf = NULL;
normalMtf(combinedBlockId, combinedBlockIdLen, &mtfBuf); //Free mtfBuf later
unsigned char* rmtfBuf = NULL;
normalRMtfWithCheck(mtfBuf, combinedBlockIdLen, &rmtfBuf, combinedBlockId); //Free mtfBuf later

for(i = 0; i < combinedBlockIdLen; i++){ mtfStat[mtfBuf[i]]++; }
for(i = 0; i < D_BLOCK_TYPES; i++){ if(LOG){ debug(HIGH, 4, "origStat[%d]: %d \t mtfStat[%d]: %d\n", i, origStat[i], i, mtfStat[i]); } }
int origNCRBitLenOverhead = 0, origNCRBitLen = approxBitLenBlockId(D_BLOCK_TYPES, origStat, combinedBlockIdLen);
origNCRBitLenOverhead = D_BLOCK_TYPES*(2+4+2)*8;
int mtfNCRBitLenOverhead = 0, mtfNCRBitLen = approxBitLenBlockId(D_BLOCK_TYPES, mtfStat, combinedBlockIdLen);
mtfNCRBitLenOverhead = D_BLOCK_TYPES*(2+4+2)*8; //nValue + encValue + encValueLen
int combineN = 0;
for(i = 0; i < D_BLOCK_TYPES; i++){ combineN += mtfStat[i]*(i+1); }
if(LOG){ debug(VERY_HIGH, 1, "combineN: %d\n", combineN); }
int mtfCombineNCRBitLenOverhead = 0, mtfCombineNCRBitLen = approxBitLenNCR(combineN, combinedBlockIdLen);
mtfCombineNCRBitLenOverhead = (2+2+4+2)*8; //combineN + rValue + encValue + encValueLen
if(LOG){ debug(VERY_HIGH, 2, "mtfCombineNCRBitLen: %d, mtfCombineNCRBitLenOverhead: %d\n", mtfCombineNCRBitLen, mtfCombineNCRBitLenOverhead); }
int crudeBitLen = 0;
if(D_BLOCK_TYPES == 4){ crudeBitLen = combinedBlockIdLen*2; }
if(D_BLOCK_TYPES == 8){ crudeBitLen = combinedBlockIdLen*3; }
if(LOG){ debug(VERY_HIGH, 1, "crudeBitLen: %d\n", crudeBitLen); }
if(LOG){ debug(VERY_HIGH, 1, "origNCRBitLen + Overhead: %d\n", origNCRBitLen = origNCRBitLen + origNCRBitLenOverhead); }
if(LOG){ debug(VERY_HIGH, 1, "mtfNCRBitLen + Overhead: %d\n", mtfNCRBitLen = mtfNCRBitLen + mtfNCRBitLenOverhead); }
if(LOG){ debug(VERY_HIGH, 1, "mtfCombineNCRBitLen + Overhead: %d\n", mtfCombineNCRBitLen = mtfCombineNCRBitLen + mtfCombineNCRBitLenOverhead); }

int bestMethod = compareBlockBitLen(crudeBitLen, origNCRBitLen, mtfNCRBitLen, mtfCombineNCRBitLen);
if(LOG){ debug(VERY_HIGH, 1, "bestMethod: %d\n", bestMethod); }
switch(bestMethod){
    case 1: //crudeBlockCompress(bD->blockTypesCount, bD->blockId, bD->blockIdLen);
            expandBlockCompress(D_BLOCK_TYPES, mtfBuf, combinedBlockIdLen, od);
            od->blockIdCompressMode = EXPANDED_WITH_MTF;
            break;
    case 2: //normalBlockCompress(D_BLOCK_TYPES, combinedBlockId, combinedBlockIdLen, origStat, od);
            //od->blockIdCompressMode = NORMAL_WITHOUT_MTF;
            expandBlockCompress(D_BLOCK_TYPES, mtfBuf, combinedBlockIdLen, od);
            od->blockIdCompressMode = EXPANDED_WITH_MTF;
            break;
    case 3: //normalBlockCompress(D_BLOCK_TYPES, mtfBuf, combinedBlockIdLen, mtfStat, od);
            //od->blockIdCompressMode = NORMAL_WITH_MTF;
            expandBlockCompress(D_BLOCK_TYPES, mtfBuf, combinedBlockIdLen, od);
            od->blockIdCompressMode = EXPANDED_WITH_MTF;
            break;
    case 4: expandBlockCompress(D_BLOCK_TYPES, mtfBuf, combinedBlockIdLen, od);
            od->blockIdCompressMode = EXPANDED_WITH_MTF;
            break;
    default: break;
}


if(rmtfBuf != NULL){ free(rmtfBuf); }
if(mtfBuf != NULL){ free(mtfBuf); }
}


int compareBlockBitLen(int crudeBitLen, int origNCRBitLen, int mtfNCRBitLen, int mtfCombineNCRBitLen){
int part1smaller = -1, part2smaller = -1, smallest = -1;
if(crudeBitLen <= origNCRBitLen){
    part1smaller = 1;
} else {
    part1smaller = 2;
}
if(mtfNCRBitLen < mtfCombineNCRBitLen){
    part2smaller = 3;
} else {
    part2smaller = 4;
}
if(part1smaller == 1){
    if(part2smaller == 3){
        if(crudeBitLen <= mtfNCRBitLen){
            smallest = 1;
        } else {
            smallest = 3;
        }
    } else { //part2smaller == 4
        if(crudeBitLen <= mtfCombineNCRBitLen){
            smallest = 1;
        } else {
            smallest = 4;
        }
    }
} else { //part1smaller == 2
    if(part2smaller == 3){
        if(origNCRBitLen <= mtfNCRBitLen){
            smallest = 2;
        } else {
            smallest = 3;
        }
    } else { //part2smaller == 4
        if(origNCRBitLen <= mtfCombineNCRBitLen){
            smallest = 2;
        } else {
            smallest = 4;
        }
    }
}
return smallest;
}


void expandBlockCompress(int blockTypesCount, unsigned char* blockId, int blockIdLen, CD* od){
if(LOG){ debug(VERY_HIGH, 0, "(expandBlockCompress)Start blockIdLen: %d\n", blockIdLen); }
//printf("(expandBlockCompress)Start blockIdLen: %d\n", blockIdLen);
int i = 0, j = blockId[0]+1; 
unsigned int totalN = 0, totalR = blockIdLen;
unsigned int* pos = (unsigned int*)malloc(sizeof(unsigned int)*blockIdLen);
pos[0] = j; totalN += j;
for(i = 1; i < blockIdLen; i++){
    j += blockId[i] + 1;
    pos[i] = j;
    totalN += blockId[i]+1;
    //pos[i] = j++;
    //j += blockId[i];
    //totalN += blockId[i]+1;
}
//printf("blockIds(%d): ", blockIdLen);
//for(i = 0; i < blockIdLen; i++){
  //  printf("%d ", blockId[i]);
//}
/*
printf("\nexpanded: ");
for(i = 0; i < blockIdLen; i++){
    printf("%d ", pos[i]);
}
printf("\n");
*/
if(LOG){ debug(VERY_HIGH, 2, "totalN: %d, totalR: %d\n", totalN, blockIdLen); }
od->blockIdBuf = (unsigned char*)malloc(sizeof(unsigned char)*(blockIdLen*D_BLOCK_TYPES+16)); memset(od->blockIdBuf, 0, sizeof(unsigned char)*(blockIdLen*D_BLOCK_TYPES+16));
od->blockIdBufLen = 0; od->blockIdBufAllocSize = blockIdLen*D_BLOCK_TYPES+16;
if(!FULL_GMP){ 
    unsigned char* encValue = NULL;
    unsigned int encValueLen = 0;
    unsigned char* firstValueLen = 0;
    int* nRP = (int*)malloc(sizeof(int)*(totalN+1)); memset(nRP, 0, sizeof(int)*(totalN+1));
    int l = 0; nRP[0] = 1; for(l = 0; l < totalR; l++){ nRP[l+1] = pos[l]+1; }
    //superEncodePosToBuf(HIGH, pos, blockIdLen, totalN, &encValue, &encValueLen, &firstValueLen);
    superEncodePosToBuf(HIGH, nRP, totalR+1, totalN+1, &encValue, &encValueLen, &firstValueLen);
    //printf("totalN: %d, totalR: %d, encValueLen: %d, firstValueLen: %d\n", totalN, totalR, encValueLen, firstValueLen);
    if(nRP != NULL){ free(nRP); }
    if(LOG){ debug(VERY_HIGH, 5, "Block with len %d, expanded to %d(n) and %d(r), compressed to %d Bytes(%d Bits)\n", blockIdLen, totalN, blockIdLen, (encValueLen+2+1), (encValueLen+2+1)*8); }
    if(LOG){ debug(HIGH, 1, "firstValueLen: %d\n", firstValueLen); }
    memcpy(od->blockIdBuf + od->blockIdBufLen, &totalN, sizeof(unsigned int)); od->blockIdBufLen += sizeof(unsigned int); 
    memcpy(od->blockIdBuf + od->blockIdBufLen, &totalR, sizeof(unsigned int)); od->blockIdBufLen += sizeof(unsigned int);
    //printf("encValueLen: %d\n", encValueLen);
    if(encValueLen > USHRT_MAX){ printf("FATAL ERROR: encValueLen exceeds USHRT_MAX\n"); }
    unsigned short outEncValueLen = (unsigned short)encValueLen;
    memcpy(od->blockIdBuf + od->blockIdBufLen, &outEncValueLen, sizeof(unsigned short)); od->blockIdBufLen += sizeof(unsigned short);
    memcpy(od->blockIdBuf + od->blockIdBufLen, &firstValueLen, sizeof(unsigned char)); od->blockIdBufLen += sizeof(unsigned char);
    while(od->blockIdBufLen + encValueLen >= od->blockIdBufAllocSize){ doubleUpChar(&od->blockIdBuf, &od->blockIdBufAllocSize, od->blockIdBufLen); }
    memcpy(od->blockIdBuf + od->blockIdBufLen, encValue, sizeof(unsigned char)*encValueLen); od->blockIdBufLen += sizeof(unsigned char)*encValueLen;
    if(encValue != NULL){ free(encValue); }
}

if(pos != NULL){ free(pos); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to compress the blockIds */
/* Input: */
/* Output: */
void normalBlockCompress(int blockTypesCount, unsigned char* blockId, unsigned int blockIdLen, int blockIdStat[], CD* od){
if(LOG){ debug(HIGH, 2, "(normalBlockCompress)Start blockTypesCount: %d, blockIdLen: %d\n", blockTypesCount, blockIdLen); }
unsigned char* outBuf = (unsigned char*)malloc(sizeof(unsigned char)*blockIdLen*2); memset(outBuf, 0, sizeof(unsigned char)*blockIdLen*2);
int i = 0, j = 0, outBufLen = 0, runningCount = 0, runningSum = 0;
unsigned short totalN = 0;

unsigned char outBlockTypesCount = 0, outCurChar = 0;
for(i = 0; i < blockTypesCount; i++){ if(blockIdStat[i] != 0){ outBlockTypesCount++; } }
memcpy(outBuf + outBufLen, &outBlockTypesCount, sizeof(unsigned char)); outBufLen += sizeof(unsigned char);

unsigned short curN[outBlockTypesCount], curNCount = 0; memset(curN, 0, sizeof(curN));

for(i = 0; i < blockTypesCount; i++){ 
    if(blockIdStat[i] != 0){ 
        outCurChar = (unsigned char)i;
        memcpy(outBuf + outBufLen, &outCurChar, sizeof(unsigned char)); outBufLen += sizeof(unsigned char);
        curN[curNCount++] = blockIdStat[i];
    } 
    totalN += blockIdStat[i];
}

memcpy(outBuf + outBufLen, &totalN, sizeof(unsigned short)); outBufLen += sizeof(unsigned short);

memcpy(outBuf + outBufLen, &curN, sizeof(unsigned short)*(curNCount-1)); outBufLen += sizeof(unsigned short)*(curNCount-1);

int* rp[blockTypesCount], rpCount[blockTypesCount];
for(i = 0; i < blockTypesCount; i++){
    rp[i] = (int*)malloc(sizeof(int)*totalN); memset(rp[i], 0, sizeof(int)*totalN); rpCount[i] = 0;
}

//Update relative positions
int charsSeen[blockTypesCount], charsSeenCount = 0, charsCount[blockTypesCount], tempSum = 0;
unsigned char charsList[blockTypesCount], curChar = 0;
int prevChar = -1;
memset(charsSeen, -1, sizeof(charsSeen)); memset(charsList, FALSE, sizeof(charsList)); memset(charsCount, 0, sizeof(charsCount));

for(i = 0; i < blockIdLen; i++){
        curChar = blockId[i];
        if(charsList[curChar] == FALSE){ charsSeen[charsSeenCount++] = curChar; }
        charsList[curChar] = TRUE;
        charsCount[curChar]++;
        if(curChar == prevChar){
                tempSum += 1;
        } else {
                tempSum = 0;
                for(j = 0; j < charsSeenCount; j++){
                        if(charsSeen[j] >= curChar){
                                tempSum = tempSum + charsCount[charsSeen[j]];
                        }
                }
        }
        if(LOG){ debug(VERY_LOW, 2, "curChar: %d, relPos: %d\n", blockId[i], tempSum); }
        rp[curChar][rpCount[curChar]] = tempSum;
        rpCount[curChar]++;
        prevChar = curChar;
}

//Encode the relative positions
unsigned char* tempOutBuf = (unsigned char*)malloc(sizeof(unsigned char)*blockIdLen*2); memset(tempOutBuf, 0, sizeof(unsigned char)*blockIdLen*2);
int tempOutBufLen = 0; unsigned short outEncValueLen[outBlockTypesCount-1]; memset(outEncValueLen, 0, sizeof(outEncValueLen));
unsigned char outFirstValueLen[outBlockTypesCount-1]; memset(outFirstValueLen, 0, sizeof(outFirstValueLen));
for(i = 0; i < blockTypesCount; i++){
    if(blockIdStat[i] != 0 && runningCount < outBlockTypesCount-1){
        printf("Encoding char %d, with n(%d) and r(%d)\n", i, totalN - runningSum, blockIdStat[i]);
        if(!FULL_GMP){ 
            unsigned char* encValue = NULL;
            unsigned int encValueLen = 0;
            unsigned char* firstValueLen = 0;
            superEncodePosToBuf(HIGH, rp[i], blockIdStat[i], totalN - runningSum, &encValue, &encValueLen, &firstValueLen);
            printf("%d(n) and %d(r), compressed to %d Bytes(%d Bits) with firstValueLen as %d\n", totalN - runningSum, blockIdStat[i], encValueLen, encValueLen*8, (unsigned int)firstValueLen);
            outEncValueLen[runningCount] = encValueLen;
            outFirstValueLen[runningCount] = firstValueLen;
            memcpy(tempOutBuf + tempOutBufLen, encValue, sizeof(unsigned char)*encValueLen); tempOutBufLen += sizeof(unsigned char)*encValueLen;
            if(encValue != NULL){ free(encValue); }
        }
        runningSum += blockIdStat[i];
        runningCount++;
    }
    
}


memcpy(outBuf + outBufLen, &outEncValueLen, sizeof(unsigned short)*(outBlockTypesCount-1)); outBufLen += sizeof(unsigned short)*(outBlockTypesCount-1);
memcpy(outBuf + outBufLen, &outFirstValueLen, sizeof(unsigned short)*(outBlockTypesCount-1)); outBufLen += sizeof(unsigned short)*(outBlockTypesCount-1);
memcpy(outBuf + outBufLen, &tempOutBuf, sizeof(unsigned char)*tempOutBufLen); outBufLen += sizeof(unsigned char)*tempOutBufLen;

if(tempOutBuf != NULL){ free(tempOutBuf); }
for(i = 0; i < blockTypesCount; i++){ if(rp[i] != NULL){ free(rp[i]); } }
if(outBuf != NULL){ free(outBuf); }
if(LOG){ debug(HIGH, 2, "(normalBlockCompress)End %d BlockIds compressed to %d Bytes(%d Bits) including meta data\n", blockIdLen, outBufLen, outBufLen*8); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to compress the blockIds */
/* Input: */
/* Output: */
void crudeBlockCompress(int blockTypesCount, unsigned char* blockId, unsigned int blockIdLen){
if(LOG){ debug(HIGH, 0, "(crudeBlockCompress)Start blockIdLen: %d\n", blockIdLen); }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to compress the blockIds */
/* Input: */
/* Output: */
void compressBlockIds(BD* bD, CD* od){
if(LOG){ debug(HIGH, 0, "(compressBlockIds)Start curChar: %d, blockIdLen: %d\n", bD->curChar, bD->blockIdLen); }
displayBlockIds(bD);
int i = 0, origStat[bD->blockTypesCount], mtfStat[bD->blockTypesCount];
memset(origStat, 0, sizeof(origStat)); memset(mtfStat, 0, sizeof(mtfStat));
for(i = 0; i < bD->blockIdLen; i++){ origStat[bD->blockId[i]]++; }

//Apply modified move to front
unsigned char* mtfBuf = NULL;
//modifiedMtf(bD->blockId, bD->blockIdLen, &mtfBuf); //Free mtfBuf later
normalMtf(bD->blockId, bD->blockIdLen, &mtfBuf); //Free mtfBuf later
unsigned char* rmtfBuf = NULL;
normalRMtfWithCheck(mtfBuf, bD->blockIdLen, &rmtfBuf, bD->blockId); //Free mtfBuf later

for(i = 0; i < bD->blockIdLen; i++){ mtfStat[mtfBuf[i]]++; }
for(i = 0; i < bD->blockTypesCount; i++){ printf("origStat[%d]: %d \t mtfStat[%d]: %d\n", i, origStat[i], i, mtfStat[i]); }
int origNCRBitLenOverhead = 0, origNCRBitLen = approxBitLenBlockId(bD->blockTypesCount, origStat, bD->blockIdLen);
origNCRBitLenOverhead = bD->blockTypesCount*(2+4+2)*8;
printf("origNCRBitLenOverhead: %d\n", origNCRBitLenOverhead);
int mtfNCRBitLenOverhead = 0, mtfNCRBitLen = approxBitLenBlockId(bD->blockTypesCount, mtfStat, bD->blockIdLen);
mtfNCRBitLenOverhead = bD->blockTypesCount*(2+4+2)*8; //nValue + encValue + encValueLen
printf("mtfNCRBitLenOverhead: %d\n", mtfNCRBitLenOverhead);
int combineN = 0;
for(i = 0; i < bD->blockTypesCount; i++){ combineN += mtfStat[i]*(i+1); }
printf("combineN: %d\n", combineN);
int mtfCombineNCRBitLenOverhead = 0, mtfCombineNCRBitLen = approxBitLenNCR(combineN, bD->blockIdLen);
mtfCombineNCRBitLenOverhead = (2+2+4+2)*8; //combineN + rValue + encValue + encValueLen
printf("mtfCombineNCRBitLen: %d, mtfCombineNCRBitLenOverhead: %d\n", mtfCombineNCRBitLen, mtfCombineNCRBitLenOverhead);
int crudeBitLen = 0;
if(bD->blockTypesCount == 4){ crudeBitLen = bD->blockIdLen*2;}
if(bD->blockTypesCount == 8){ crudeBitLen = bD->blockIdLen*3;}
printf("crudeBitLen: %d\n", crudeBitLen);
printf("origNCRBitLen + Overhead: %d\n", origNCRBitLen = origNCRBitLen + origNCRBitLenOverhead);
printf("mtfNCRBitLen + Overhead: %d\n", mtfNCRBitLen = mtfNCRBitLen + mtfNCRBitLenOverhead);
printf("mtfCombineNCRBitLen + Overhead: %d\n", mtfCombineNCRBitLen = mtfCombineNCRBitLen + mtfCombineNCRBitLenOverhead);
int bestMethod = compareBlockBitLen(crudeBitLen, origNCRBitLen, mtfNCRBitLen, mtfCombineNCRBitLen);
printf("bestMethod: %d\n", bestMethod);
switch(bestMethod){
    case 1: crudeBlockCompress(bD->blockTypesCount, bD->blockId, bD->blockIdLen);
            expandBlockCompress(bD->blockTypesCount, mtfBuf, bD->blockIdLen, od);
            break;
    case 2: normalBlockCompress(bD->blockTypesCount, bD->blockId, bD->blockIdLen, origStat, od);
            break;
    case 3: normalBlockCompress(bD->blockTypesCount, mtfBuf, bD->blockIdLen, mtfStat, od);
            break;
    case 4: expandBlockCompress(bD->blockTypesCount, mtfBuf, bD->blockIdLen, od);
            break;
    default: break;
}


if(rmtfBuf != NULL){ free(rmtfBuf); }
if(mtfBuf != NULL){ free(mtfBuf); }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to verify the magic split */
/* Input: */
/* Output: */
void verifyMagicSplitNew(int* rpZero, int rpZeroN, int rpZeroCount, int* rpOne, int rpOneN, int rpOneCount, unsigned char* splitId, int splitIdCount, int* rpOrig){
if(LOG){ debug(HIGH, 0, "(verifyMagicSplitNew)Start rpZeroN: %d, rpZeroCount: %d, rpOneN: %d, rpOneCount: %d, splitIdCount: %d\n", rpZeroN, rpZeroCount, rpOneN, rpOneCount, splitIdCount); }
int* rpZeroNew = (int*)malloc(sizeof(int)*rpZeroN*2); memset(rpZeroNew, 0, sizeof(int)*rpZeroN*2); int rpZeroNewCount = 0, rpZeroNewN = 0;
int* rpOneNew = (int*)malloc(sizeof(int)*rpOneN*2); memset(rpOneNew, 0, sizeof(int)*rpOneN*2); int rpOneNewCount = 0, rpOneNewN = 0;
int i = 0, j = 0, curPos = -1, prevPos = -1;
int rpZeroCurPos = 0, rpOneCurPos = 0;

//Find new positions for zero
if(rpZeroCount > 0){
if(rpZero[0] == 1){
    rpZeroNewN++;
    rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
} else {
    if(rpZero[0] > 1){
        for(j = 1; j < rpZero[0]; j++){ //01
            rpZeroNewN++; //For zero
            rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
            rpZeroNewN++; //For one
        }
        rpZeroNewN++; //For zero
        rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
    }
}
prevPos = rpZero[0];
for(i = 1; i < rpZeroCount; i++){
    curPos = rpZero[i]; 
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
if(rpZeroN == rpZero[rpZeroCount-1]){
    rpZeroNewN++; //For zero
    rpZeroNew[rpZeroNewCount++] = rpZeroNewN;
}
for(j = 1; j <= rpZeroN - rpZero[rpZeroCount-1]; j++){
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
printf("rpZeroNewN: %d, rpZeroNewCount: %d\n", rpZeroNewN, rpZeroNewCount);

//Find new positions for one
if(rpOneCount > 0){
    rpOneNewN += rpOne[0] + 1;
    rpOneNew[rpOneNewCount++] = rpOneNewN;
    prevPos = rpOne[0];
    for(i = 1; i < rpOneCount; i++){
        curPos = rpOne[i];
        rpOneNewN += curPos - prevPos + 1;
        rpOneNew[rpOneNewCount++] = rpOneNewN;
        prevPos = curPos;
    }
    //rpOneNewN += rpOneN - rpOne[rpOneCount-1] + 1;
    rpOneNewN += rpOneN - rpOne[rpOneCount-1];
} else {
    if(rpOneN != 0 && rpOneCount == 0){
        rpOneNewN += rpOneN + 1;
        rpOneNewCount = 0;
    }
}
printf("rpOneNewN: %d, rpOneNewCount: %d\n", rpOneNewN, rpOneNewCount);

int* rp = (int*)malloc(sizeof(int)*(rpZeroNewN+rpOneNewN+1)*2); memset(rp, 0, sizeof(int)*(rpZeroNewN+rpOneNewN+1)*2); int rpCount = 0, runningN = 0;

for(i = 0; i < splitIdCount-1; i++){
    if(splitId[i] == 1){
        if(rpOneCurPos == 0){
            runningN += rpOneNew[rpOneCurPos++];
            rp[rpCount++] = runningN;
        } else {
            runningN += rpOneNew[rpOneCurPos] - rpOneNew[rpOneCurPos-1];
            rp[rpCount++] = runningN;
            rpOneCurPos++;
        }
    } else {
            runningN++;
            rp[rpCount++] = runningN;
            rpZeroCurPos++;
            while(rpZeroCurPos < rpZeroNewCount && rpZeroNew[rpZeroCurPos] - rpZeroNew[rpZeroCurPos-1] == 1){
                runningN++;
                rp[rpCount++] = runningN;
                rpZeroCurPos++;
            }
            runningN++;
    }
    //printf("rpCount: %d, runningN: %d\n", rpCount, runningN);
}
if(splitId[splitIdCount-1] == 1){
    if(rpOneCurPos == rpOneCount){
        runningN += rpOneNewN - rpOneNew[rpOneCurPos-1];
    } else {
        runningN += rpOneNew[rpOneCurPos] - rpOneNew[rpOneCurPos-1];
        rp[rpCount++] = runningN;
    }
} else {
            runningN++; rpZeroCurPos++;
            rp[rpCount++] = runningN;
            while(rpZeroCurPos < rpZeroNewCount && rpZeroNew[rpZeroCurPos] - rpZeroNew[rpZeroCurPos-1] == 1){
                runningN++;
                rp[rpCount++] = runningN;
                rpZeroCurPos++;
            }
            if(rpZeroNew[rpZeroNewCount-1] + 1 == rpZeroNewN){
                runningN++;
            }
}
printf("RP CHECK: "); 
for(i = 0; i < rpCount; i++){ 
    //printf("%d ", rp[i]); 
    if(rp[i] != rpOrig[i]){ printf("rp failure...\n"); }
} printf("\n");
printf("rpCount: %d, runningN: %d\n", rpCount, runningN);


if(rp != NULL){ free(rp); }
if(rpZeroNew != NULL){ free(rpZeroNew); }
if(rpOneNew != NULL){ free(rpOneNew); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to verify the magic split */
/* Input: */
/* Output: */
void verifyMagicSplit(int* rpZero, int rpZeroN, int rpZeroCount, int* rpOne, int rpOneN, int rpOneCount, unsigned char* splitId, int splitIdCount, int* rpOrig){
if(LOG){ debug(HIGH, 0, "(verifyMagicSplit)Start rpZeroN: %d, rpZeroCount: %d, rpOneN: %d, rpOneCount: %d, splitIdCount: %d\n", rpZeroN, rpZeroCount, rpOneN, rpOneCount, splitIdCount); }
int i = 0, runningN = 0, rpOneCurPos = 0, rpZeroCurPos = 0, rpZeroTempCurPos = 0, rpZeroTempMaxCurPos = 0, runOnForChar = -1, rpZeroFirstTime = TRUE;
int* rpNew = (int*)malloc(sizeof(int)*(rpZeroCount*2+rpOneCount*2)); memset(rpNew, 0, sizeof(int)*(rpZeroCount*2+rpOneCount*2)); int rpNewCount = 0;
for(i = 0; i < splitIdCount-1; i++){
    if(splitId[i] == 1){
        if(rpOneCurPos == 0){
            runningN += rpOne[rpOneCurPos];
            runningN++; //For adding the first dropped one
        } else {
            runningN += rpOne[rpOneCurPos] - rpOne[rpOneCurPos-1];
            runningN++; //For adding the first dropped one
        }
        rpOneCurPos++;
        rpNew[rpNewCount++] = runningN;
        if(rpNew[rpNewCount-1] != rpOrig[rpNewCount-1]){ printf("ERROR1\n"); }
    }
    if(splitId[i] == 0){
        //if(rpZeroCurPos == 0){
        if(rpZeroFirstTime == TRUE){
            if(rpZero[rpZeroCurPos] > 1){
                rpZeroTempMaxCurPos = rpZero[rpZeroCurPos] - 1; rpZeroTempCurPos = 0;
                runningN++; //For zero
                rpNew[rpNewCount++] = runningN;
                if(rpNew[rpNewCount-1] != rpOrig[rpNewCount-1]){ printf("ERROR2\n"); }
                runningN++; //For one
                rpZeroTempCurPos++;
            } else { //rpZero[rpZeroCurPos] == 1
                runningN++; //For zero
                rpNew[rpNewCount++] = runningN;
                if(rpNew[rpNewCount-1] != rpOrig[rpNewCount-1]){ printf("ERROR3\n"); }
                runOnForChar == 0;
            }
            rpZeroFirstTime = FALSE;
            //rpZeroCurPos++;
        } else {
            printf("rpZeroTempMaxCurPos: %d, rpZeroTempCurPos: %d, splitId[%d]: %d\n", rpZeroTempMaxCurPos, rpZeroTempCurPos, i, splitId[i]);
            if(rpZeroTempCurPos < rpZeroTempMaxCurPos){
                runningN++; //For zero
                rpNew[rpNewCount++] = runningN;
                if(rpNew[rpNewCount-1] != rpOrig[rpNewCount-1]){ printf("ERROR4\n"); }
                runningN++; //For one
                rpZeroTempCurPos++;
            } else { //rpZeroTempCurPos == rpZeroTempMaxCurPos //all dropped ones are exhausted, start taking in zeros now
                runningN++; //For zero
                rpNew[rpNewCount++] = runningN;
                if(rpNew[rpNewCount-1] != rpOrig[rpNewCount-1]){ printf("ERROR5\n"); }
                runningN++; //For zero
                rpNew[rpNewCount++] = runningN;
                if(rpNew[rpNewCount-1] != rpOrig[rpNewCount-1]){ printf("ERROR6\n"); }
                rpZeroCurPos++;
                while(rpZeroCurPos < rpZeroCount && rpZero[rpZeroCurPos] - rpZero[rpZeroCurPos-1] == 1){
                    runningN++; //For zero
                    rpNew[rpNewCount++] = runningN;
                    rpZeroCurPos++;
                }
                runningN++; //For one
                if(rpZeroCurPos == rpZeroCount){
                    rpZeroTempMaxCurPos = rpZeroN - rpZero[rpZeroCount-1] - 1;
                } else {
                    rpZeroTempMaxCurPos = rpZero[rpZeroCurPos] - rpZero[rpZeroCurPos-1] - 1; rpZeroTempCurPos = 0;
                }
                printf("rpZeroTempMaxCurPos: %d(%d - %d)\n", rpZeroTempMaxCurPos, rpZeroCurPos, rpZero[rpZeroCurPos]);
                rpZeroTempCurPos++;
            }
        }
    }
}

//For the last SplitId
if(splitId[splitIdCount-1] == 0){
    if(rpZeroCurPos == rpZeroCount){ //all zeros are over, only 1 left(preceded by 0s)
            runningN++;
            rpNew[rpNewCount++] = runningN;
            if(rpNew[rpNewCount-1] != rpOrig[rpNewCount-1]){ printf("ERROR7\n"); }
            runningN++;
    } else {
        for(i = rpZeroCurPos; i <= rpZeroCount; i++){
            runningN++;
            rpNew[rpNewCount++] = runningN;
            if(rpNew[rpNewCount-1] != rpOrig[rpNewCount-1]){ printf("ERROR7\n"); }
        }
        if(rpZero[rpZeroCount] != rpZeroN){
            runningN++;
        }
    }
} else {
    runningN++; //For dropped one
    if(rpOneCurPos == rpOneCount){ //no zero in the end of 1s
        runningN += rpOneN - rpOne[rpOneCurPos-1];
    } else {
        runningN += rpOne[rpOneCurPos] - rpOne[rpOneCurPos-1];
        rpNew[rpNewCount++] = runningN;
        if(rpNew[rpNewCount-1] != rpOrig[rpNewCount-1]){ printf("ERROR8\n"); }
    }
}

printf("rpNewCount: %d, runningN: %d\n", rpNewCount, runningN);
for(i = 0; i < rpNewCount; i++){
    printf("rpNew[%d]: %d, rpOrig[%d]: %d\n", i, rpNew[i], i, rpOrig[i]);
    if(rpNew[i] != rpOrig[i]){
        printf("verifyMagicSplit failure...\n");
    }
}
if(rpNew != NULL){ free(rpNew); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to cleanup all the allocations */
/* Input: */
/* Output: */
void magicSplitOldWorking(int curChar, int* rp, int rCount, int nCount){
if(LOG){ debug(HIGH, 0, "(magicSplit)Start curChar: %d, nCount: %d, rCount: %d\n", curChar, nCount, rCount); }
int i = 0, runOnForChar = 0, prevPos = 0, curPos = 0;
int* rpOne = (int*)malloc(sizeof(int)*nCount); memset(rpOne, 0, sizeof(int)*nCount); int rpOneCount = 0, rpOneN = 0;
int* rpZero = (int*)malloc(sizeof(int)*nCount); memset(rpZero, 0, sizeof(int)*nCount); int rpZeroCount = 0, rpZeroN = 0;
unsigned char* splitId = (unsigned char*)malloc(sizeof(unsigned char)*nCount); memset(splitId, 0, sizeof(unsigned char)*nCount); int splitIdCount = 0, splitIdZeroCount = 0;
if(rp[0] == 1){ 
    //rpZero[rpZeroCount++] = rp[0]; //For dropping the first zero
    //rpZeroN++; //For dropping the first zero
    runOnForChar = 0;
} else { 
    rpOneN = rp[0];
    rpOneN--; //For dropping the first one
    rpOne[rpOneCount++] = rpOneN;
    splitId[splitIdCount++] = 1;
    runOnForChar = -1;
}
prevPos = rp[0];
for(i = 1; i < rCount; i++){
    curPos = rp[i];
    if(curPos - prevPos > 1){ //there are 1s in between
        if(runOnForChar == 0){ 
            rpZeroN++; 
            splitId[splitIdCount++] = 0; splitIdZeroCount++;
            if(curPos - prevPos > 2){
                rpOneN += (curPos - prevPos) - 1;
                rpOneN--; //For dropping the first one
                rpOne[rpOneCount++] = rpOneN;
                splitId[splitIdCount++] = 1;
                runOnForChar = -1;
            } else {
                //rpZeroN++;  //For dropping the first zero
                //rpZero[rpZeroCount++] = rpZeroN; //For dropping the first zero
                runOnForChar = 0;
            }
        }  else {
            if(runOnForChar == -1){
                rpOneN += curPos - prevPos;
                rpOneN--; //For dropping the first one
                rpOne[rpOneCount++] = rpOneN;
                splitId[splitIdCount++] = 1;
                runOnForChar = -1;
            }
        }
    } else { //diff is one, i.e. continuous 0s
        if(runOnForChar == -1){
            //rpZeroN++; //For dropping the first zero
            //rpZero[rpZeroCount++] = rpZeroN; //For dropping the first zero
        } else {
            if(runOnForChar == 0){
                rpZeroN++;
                rpZero[rpZeroCount++] = rpZeroN;
            }
        }
        runOnForChar = 0;
    }
    prevPos = curPos;
}
if(nCount - rp[rCount-1] > 0){
    if(runOnForChar == 0){
        rpZeroN++;
        splitId[splitIdCount++] = 0; splitIdZeroCount++;
        if(nCount - rp[rCount-1] > 1){
            rpOneN += nCount - rp[rCount-1] - 1;
            //rpOneN--; //For dropping the first one //Not dropping for the last run of 1s
            splitId[splitIdCount++] = 1;
        }
    }
    if(runOnForChar == -1){
        rpOneN += nCount - rp[rCount-1];
        //rpOneN--; //For dropping the first one //Not dropping for the last run of 1s
        splitId[splitIdCount++] = 1;
    }
} else {
    if(nCount == rp[rCount-1]){
        if(rCount > 1 && (rp[rCount-1] == rp[rCount-2]+1)){
            splitId[splitIdCount++] = 0; splitIdZeroCount++;
        }
    }
}

printf("rpZero: (n: %d, r: %d) ", rpZeroN, rpZeroCount); //for(i = 0; i < rpZeroCount; i++){ printf("%d ", rpZero[i]); } printf("\n");
printf("rpOne : (n: %d, r: %d) ", rpOneN, rpOneCount); //for(i = 0; i < rpOneCount; i++){ printf("%d ", rpOne[i]); } printf("\n");
printf("splitId : (n: %d, r: %d) ", splitIdCount, splitIdZeroCount); //for(i = 0; i < splitIdCount; i++){ printf("%d ", splitId[i]); } printf("\n");

verifyMagicSplitNew(rpZero, rpZeroN, rpZeroCount, rpOne, rpOneN, rpOneCount, splitId, splitIdCount, rp);

if(splitId != NULL){ free(splitId); }
if(rpOne != NULL){ free(rpOne); }
if(rpZero != NULL){ free(rpZero); }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to cleanup all the allocations */
/* Input: */
/* Output: */
void magicSplitGood(int curChar, int* rp, int rCount, int nCount, SD** sD){
if(LOG){ debug(VERY_HIGH, 0, "(magicSplit)Start curChar: %d, nCount: %d, rCount: %d\n", curChar, nCount, rCount); }
int i = 0, runOnForChar = 0, prevPos = 0, curPos = 0;
(*sD) = (SD*)(malloc(sizeof(SD)));
(*sD)->rpOne = (unsigned int*)malloc(sizeof(unsigned int)*nCount); memset((*sD)->rpOne, 0, sizeof(unsigned int)*nCount);
(*sD)->rpOneCount = 0; (*sD)->rpOneN = 0;
(*sD)->rpZero = (int*)malloc(sizeof(unsigned int)*nCount); memset((*sD)->rpZero, 0, sizeof(unsigned int)*nCount); 
(*sD)->rpZeroCount = 0; (*sD)->rpZeroN = 0;
(*sD)->splitId = (unsigned char*)malloc(sizeof(unsigned char)*nCount); memset((*sD)->splitId, 0, sizeof(unsigned char)*nCount); 
(*sD)->splitIdCount = 0; (*sD)->splitIdZeroCount = 0;
(*sD)->rpSplitId = (unsigned int*)malloc(sizeof(unsigned int)*nCount); memset((*sD)->rpSplitId, 0, sizeof(unsigned int)*nCount); 
(*sD)->rpSplitIdCount = 0; (*sD)->rpSplitIdN = 0;
(*sD)->allRP = NULL;
int rpOneN = 0, rpOneCount = 0, rpZeroN = 0, rpZeroCount = 0, splitIdCount = 0, splitIdZeroCount = 0;
if(rp[0] == 1){ 
    //rpZero[rpZeroCount++] = rp[0]; //For dropping the first zero
    //rpZeroN++; //For dropping the first zero
    runOnForChar = 0;
} else { 
    rpOneN = rp[0];
    rpOneN--; //For dropping the first one
    (*sD)->rpOne[rpOneCount++] = rpOneN;
    (*sD)->splitId[splitIdCount++] = 1;
    runOnForChar = -1;
}
prevPos = rp[0];
for(i = 1; i < rCount; i++){
    curPos = rp[i];
    if(curPos - prevPos > 1){ //there are 1s in between
        if(runOnForChar == 0){ 
            rpZeroN++; 
            (*sD)->splitId[splitIdCount++] = 0; (*sD)->rpSplitId[splitIdZeroCount] = splitIdCount; splitIdZeroCount++;
            if(curPos - prevPos > 2){
                rpOneN += (curPos - prevPos) - 1;
                rpOneN--; //For dropping the first one
                (*sD)->rpOne[rpOneCount++] = rpOneN;
                (*sD)->splitId[splitIdCount++] = 1;
                runOnForChar = -1;
            } else {
                //rpZeroN++;  //For dropping the first zero
                //rpZero[rpZeroCount++] = rpZeroN; //For dropping the first zero
                runOnForChar = 0;
            }
        }  else {
            if(runOnForChar == -1){
                rpOneN += curPos - prevPos;
                rpOneN--; //For dropping the first one
                (*sD)->rpOne[rpOneCount++] = rpOneN;
                (*sD)->splitId[splitIdCount++] = 1;
                runOnForChar = -1;
            }
        }
    } else { //diff is one, i.e. continuous 0s
        if(runOnForChar == -1){
            //rpZeroN++; //For dropping the first zero
            //rpZero[rpZeroCount++] = rpZeroN; //For dropping the first zero
        } else {
            if(runOnForChar == 0){
                rpZeroN++;
                (*sD)->rpZero[rpZeroCount++] = rpZeroN;
            }
        }
        runOnForChar = 0;
    }
    prevPos = curPos;
}
if(nCount - rp[rCount-1] > 0){
    if(runOnForChar == 0){
        rpZeroN++;
        (*sD)->splitId[splitIdCount++] = 0; (*sD)->rpSplitId[splitIdZeroCount] = splitIdCount; splitIdZeroCount++;
        if(nCount - rp[rCount-1] > 1){
            rpOneN += nCount - rp[rCount-1] - 1;
            //rpOneN--; //For dropping the first one //Not dropping for the last run of 1s
            (*sD)->splitId[splitIdCount++] = 1;
        }
    }
    if(runOnForChar == -1){
        rpOneN += nCount - rp[rCount-1];
        //rpOneN--; //For dropping the first one //Not dropping for the last run of 1s
        (*sD)->splitId[splitIdCount++] = 1;
    }
} else {
    if(nCount == rp[rCount-1]){
        //if(rCount > 1 && (rp[rCount-1] == rp[rCount-2]+2)){
            (*sD)->splitId[splitIdCount++] = 0; (*sD)->rpSplitId[splitIdZeroCount] = splitIdCount; splitIdZeroCount++;
        //}
    }
}
(*sD)->rpOneCount = rpOneCount;
(*sD)->rpOneN = rpOneN;
(*sD)->rpZeroCount = rpZeroCount;
(*sD)->rpZeroN = rpZeroN;
(*sD)->splitIdCount = splitIdCount;
(*sD)->splitIdZeroCount = splitIdZeroCount;
(*sD)->rpSplitIdCount = splitIdZeroCount;
(*sD)->rpSplitIdN = splitIdCount;

if(LOG){ debug(VERY_HIGH, 6, "(magicSplit)rpZeroN: %d, rpZeroCount: %d, rpOneN: %d, rpOneCount: %d, splitIdCount: %d, splitIdZeroCount: %d\n", rpZeroN, rpZeroCount, rpOneN, rpOneCount, splitIdCount, splitIdZeroCount); }
/*if(curChar == 41){
printf("\nrpZero: (n: %d, r: %d) ", rpZeroN, rpZeroCount); 
printf("\nNKrpZero: "); for(i = 0; i < rpZeroCount; i++){ printf("%d ", (*sD)->rpZero[i]); } printf(" (%d)\n", rpZeroN);
printf("\nrpOne : (n: %d, r: %d) ", rpOneN, rpOneCount); 
printf("\nNKrpOne: "); for(i = 0; i < rpOneCount; i++){ printf("%d ", (*sD)->rpOne[i]); } printf(" (%d)\n", rpOneN);
printf("\nsplitId : (n: %d, r: %d) ", splitIdCount, splitIdZeroCount); 
printf("\nNKsplitId: "); for(i = 0; i < splitIdCount; i++){ printf("%d ", (*sD)->splitId[i]); } printf("\n");
}*/

//verifyMagicSplitNew((*sD)->rpZero, rpZeroN, rpZeroCount, (*sD)->rpOne, rpOneN, rpOneCount, (*sD)->splitId, splitIdCount, rp);

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to cleanup all the allocations */
/* Input: */
/* Output: */
void magicSplit(int inChar, int* rp, int rCount, int nCount, SD** sD){
if(LOG){ debug(VERY_HIGH, 0, "(magicSplit)Start inChar: %d, nCount: %d, rCount: %d\n", inChar, nCount, rCount); }
(*sD) = (SD*)(malloc(sizeof(SD)));
(*sD)->rpOne = (unsigned int*)malloc(sizeof(unsigned int)*nCount); memset((*sD)->rpOne, 0, sizeof(unsigned int)*nCount);
(*sD)->rpOneCount = 0; (*sD)->rpOneN = 0;
(*sD)->rpZero = (int*)malloc(sizeof(unsigned int)*nCount); memset((*sD)->rpZero, 0, sizeof(unsigned int)*nCount); 
(*sD)->rpZeroCount = 0; (*sD)->rpZeroN = 0;
(*sD)->splitId = (unsigned char*)malloc(sizeof(unsigned char)*nCount); memset((*sD)->splitId, 0, sizeof(unsigned char)*nCount); 
(*sD)->splitIdCount = 0; (*sD)->splitIdZeroCount = 0;
(*sD)->rpSplitId = (unsigned int*)malloc(sizeof(unsigned int)*nCount); memset((*sD)->rpSplitId, 0, sizeof(unsigned int)*nCount); 
(*sD)->rpSplitIdCount = 0; (*sD)->rpSplitIdN = 0;
(*sD)->allRP = NULL;
int rpOneN = 0, rpOneCount = 0, rpZeroN = 0, rpZeroCount = 0, splitIdCount = 0, splitIdZeroCount = 0, rpSplitIdCount = 0;

int i = 0, j = 0, curChar = -1, runningChar = -1, prevChar = -1, runStarted = TRUE;
//printf("RP: "); for(i = 0; i < rCount; i++){ printf("%d ", rp[i]); } printf("\n");
unsigned char* splitBuf[3]; unsigned int splitBufLen[3];
unsigned char* inBuf = (unsigned char*)malloc(sizeof(unsigned char)*nCount); memset(inBuf, 1, sizeof(unsigned char)*nCount);
unsigned int inBufLen = nCount;
for(i = 0; i < rCount; i++){ inBuf[rp[i]-1] = 0; }
for(i = 0; i < 3; i++){
    splitBuf[i] = (unsigned char*)malloc(sizeof(unsigned char)*nCount); memset(splitBuf[i], 0, sizeof(unsigned char)*nCount);
    splitBufLen[i] = 0;
}
runningChar = prevChar = inBuf[0];
splitBuf[prevChar][splitBufLen[prevChar]++] = prevChar;
for(i = 1; i < inBufLen; i++){
    curChar = inBuf[i];
    if(curChar == prevChar){
        splitBuf[curChar][splitBufLen[curChar]++] = curChar;
        if(i == inBufLen-1){
            splitBuf[2][splitBufLen[2]++] = curChar;
            splitBufLen[curChar]--;
        }
        runStarted = TRUE;
    } else {
        if(runStarted == TRUE){
            splitBuf[prevChar][splitBufLen[prevChar]-1] = curChar;
            splitBuf[2][splitBufLen[2]++] = prevChar;
            runStarted = FALSE;
        } else {
            splitBuf[curChar][splitBufLen[curChar]++] = curChar;
            runStarted = TRUE;
            if(i == inBufLen-1){
                splitBuf[2][splitBufLen[2]++] = curChar;
                splitBufLen[curChar]--;
            }
        }
        
    }
    prevChar = curChar;
}

if(inChar == 0){
    FILE* tf1 = (FILE*)openFile(HIGH, "id1.txt", "wb");
    fwrite(splitBuf[2], sizeof(unsigned char), splitBufLen[2], tf1);
    fclose(tf1);
}

//printf("000: ");
for(i = 0; i < splitBufLen[0]; i++){
    //printf("%d", splitBuf[0][i]);
    if(splitBuf[0][i] == 0){
        (*sD)->rpZero[rpZeroCount++] = i+1;
    }
}
//printf("\n");
for(i = 0; i < splitBufLen[1]; i++){
    if(splitBuf[1][i] == 0){
        (*sD)->rpOne[rpOneCount++] = i+1;
    }
}
for(i = 0; i < splitBufLen[2]; i++){
    (*sD)->splitId[i] = splitBuf[2][i];
    if(splitBuf[2][i] == 0){
        (*sD)->rpSplitId[rpSplitIdCount++] = i+1;
        splitIdZeroCount++;
    }
}

(*sD)->rpOneCount = rpOneCount;
(*sD)->rpOneN = splitBufLen[1];
(*sD)->rpZeroCount = rpZeroCount;
(*sD)->rpZeroN = splitBufLen[0];
(*sD)->splitIdCount = splitBufLen[2];
(*sD)->splitIdZeroCount = splitIdZeroCount;
(*sD)->rpSplitIdCount = rpSplitIdCount;
(*sD)->rpSplitIdN = splitBufLen[2];

if(LOG){ debug(VERY_HIGH, 6, "(magicSplit)rpZeroN: %d, rpZeroCount: %d, rpOneN: %d, rpOneCount: %d, splitIdCount: %d, splitIdZeroCount: %d\n", (*sD)->rpZeroN, (*sD)->rpZeroCount, (*sD)->rpOneN, (*sD)->rpOneCount, (*sD)->splitIdCount, (*sD)->splitIdZeroCount); }

for(i = 0; i < 3; i++){
    if(splitBuf[i] != NULL){ free(splitBuf[i]); }
}
if(inBuf != NULL){ free(inBuf); }
/*if(inChar == 41){
printf("\nrpZero: (n: %d, r: %d) ", rpZeroN, rpZeroCount); 
printf("\nNKrpZero: "); for(i = 0; i < rpZeroCount; i++){ printf("%d ", (*sD)->rpZero[i]); } printf(" (%d)\n", rpZeroN);
printf("\nrpOne : (n: %d, r: %d) ", rpOneN, rpOneCount); 
printf("\nNKrpOne: "); for(i = 0; i < rpOneCount; i++){ printf("%d ", (*sD)->rpOne[i]); } printf(" (%d)\n", rpOneN);
printf("\nsplitId : (n: %d, r: %d) ", splitIdCount, splitIdZeroCount); 
printf("\nNKsplitId: "); for(i = 0; i < splitIdCount; i++){ printf("%d ", (*sD)->splitId[i]); } printf("\n");
}*/

//verifyMagicSplitNew((*sD)->rpZero, rpZeroN, rpZeroCount, (*sD)->rpOne, rpOneN, rpOneCount, (*sD)->splitId, splitIdCount, rp);

}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to check the basic criteria for magicSplit and densityApplication */
/* Input: */
/* Output: */
int basicCheck(int curChar, int nCount, int rCount){
if(LOG){ debug(VERY_HIGH, 0, "(basicCheck)Start curChar: %d, nCount: %d, rCount: %d\n", curChar, nCount, rCount); }
int result = FALSE;
if(nCount > 100 && rCount > 25){
    result = TRUE;
}

if(result){ if(LOG){ debug(HIGH, 0, "(basicCheck)End basicCheckCleared = TRUE\n"); } } else { if(LOG){ debug(HIGH, 0, "(basicCheck)End basicCheckCleared = FALSE\n"); } }
//result = FALSE;
return result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to check split applicability */
/* Input: */
/* Output: */
int checkSplitApplicability(SD* sD, int curChar, int nCount, int rCount){
if(LOG){ debug(VERY_HIGH, 0, "(checkSplitApplicability)Start curChar: %d, nCount: %d, rCount: %d\n", curChar, nCount, rCount); }
int result = FALSE;
int overallBitLen = approxBitLenNCR(nCount, rCount);
if(LOG){ debug(HIGH, 3, "overallBitLen: %d(n: %d, r: %d)\n", overallBitLen, nCount, rCount); }
int rpZeroBitLen = 0, rpOneBitLen = 0, splitIdBitLen = 0;
rpZeroBitLen = approxBitLenNCR(sD->rpZeroN, sD->rpZeroCount);
if(LOG){ debug(HIGH, 3, "rpZeroBitLen: %d(n: %d, r: %d)\n", rpZeroBitLen, sD->rpZeroN, sD->rpZeroCount); }
rpOneBitLen = approxBitLenNCR(sD->rpOneN, sD->rpOneCount);
if(LOG){ debug(HIGH, 3, "rpOneBitLen: %d(n: %d, r: %d)\n", rpOneBitLen, sD->rpOneN, sD->rpOneCount); }
splitIdBitLen = approxBitLenNCR(sD->splitIdCount, sD->splitIdZeroCount);
if(LOG){ debug(HIGH, 3, "splitIdBitLen: %d(n: %d, r: %d)\n", splitIdBitLen, sD->splitIdCount, sD->splitIdZeroCount); }
int overhead = (4+2+2+2)*2*8;
if(LOG){ debug(HIGH, 3, "overhead: %d\n", overhead); }
int combinedBitLen = rpZeroBitLen + rpOneBitLen + splitIdBitLen;
int gain = overallBitLen - (combinedBitLen + overhead);
if(LOG){ debug(HIGH, 3, "overallBitLen: %d, combinedSplitBitLen: %d(%d + %d), gain: %d\n", overallBitLen, combinedBitLen+overhead, combinedBitLen, overhead, gain); }
if(gain > 100){ result = TRUE; if(LOG){ debug(VERY_HIGH, 1, "Eligible for split, gain: %d.\n", gain); } }
else { if(LOG){ debug(VERY_HIGH, 1, "Not Eligible for split. gain: %d\n", gain); } }
return result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to cleanup all the allocations */
/* Input: */
/* Output: */
void splitDataCleanup(SD* sD){
if(LOG){ debug(VERY_HIGH, 0, "(splitDataCleanup)Start\n"); }
if(sD->rpZero != NULL){ free(sD->rpZero); }
if(sD->rpOne != NULL){ free(sD->rpOne); }
if(sD->splitId != NULL){ free(sD->splitId); }
if(sD->rpSplitId != NULL){ free(sD->rpSplitId); }
if(sD != NULL){ free(sD); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to cleanup all the allocations */
/* Input: */
/* Output: */
void densityInfo(int curChar, unsigned int dNCount, unsigned int dRCount, BD* bD){
if(LOG){ debug(VERY_HIGH, 0, "(densityInfo)Start\n"); }
int bitLen = 0, k = 0, i = 0;
//bitLen = nCrBitLen(dNCount, dRCount);
if(LOG){ debug(VERY_HIGH, 3, "densityInfo: nCount: %d, rCount: %d, density: %d, bitLen: %d, rev density: %d\n", dNCount, dRCount, (dRCount*100)/dNCount, bitLen, (bitLen*100)/dNCount); }
for(k = 0; k < bD->blockTypesCount; k++){
    if(bD->blockRPNCount[k] != 0){
       //bitLen = nCrBitLen(bD->blockRPNCount[k], bD->blockRPCount[k]);
       bitLen = 0;
       if(LOG){ debug(VERY_HIGH, 3, "densityInfo BlockType%d: nCount: %d, rCount: %d, density: %d, bitLen: %d, rev density: %d\n", k, bD->blockRPNCount[k], bD->blockRPCount[k], (bD->blockRPCount[k]*100)/bD->blockRPNCount[k], bitLen, (bitLen*100)/bD->blockRPNCount[k]); }
       } else {
          if(LOG){ debug(VERY_HIGH, 3, "densityInfo BlockType%d: nCount: %d, rCount: %d\n", k, bD->blockRPNCount[k], bD->blockRPCount[k]); }
       }
      //if(curChar == 0){ printf("BLOCKTYPE%d: ", k); for(i = 0; i < bD->blockRPCount[k]; i++){ printf("%d ", bD->blockRP[k][i]); } printf(" (%d)\n", bD->blockRPNCount[k]); }
    }
//printf("NKBLOCKID: "); for(k = 0; k < bD->blockIdLen; k++){ printf("%d ", bD->blockId[k]); } printf("\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to write the nCount of split data to nCountRandom buf */
/* Input: */
/* Output: */
void writeDensityNDataToBuf(CD* od, int nCount){
if(LOG){ debug(VERY_HIGH, 1, "\n(writeDensityNDataToBuf)Start: nCount: %d\n", nCount); }
//if(nCount > UINT16_MAX){ printf("ERROR: nCount(%d) bigger than UINT16_MAX...Use a bigger datatype\n", nCount); }
od->nCountRandomBuf[od->nCountRandomBufLen++] = (unsigned int)nCount;
if(od->nCountRandomBufLen*sizeof(unsigned int) > od->nCountRandomBufAllocSize){ printf("FATAL ERROR: nCountRandomBufLen exceeds nCountRandomBufAllocSize\n"); }
int i = 0;
//for(i = 0; i < od->nCountRandomBufLen; i++){ printf("%d ", od->nCountRandomBuf[i]); } printf("\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to cleanup all the allocations */
/* Input: */
/* Output: */
int densityWrapper(int curChar, unsigned int* rp, unsigned int nCount, unsigned int rCount, BD* bD, CD* od, unsigned char** combinedBlockId, int* combinedBlockIdLen, int* combinedBlockIdAllocSize){
if(LOG){ debug(VERY_HIGH, 0, "-----------------------------------------------------------\n"); }
if(LOG){ debug(VERY_HIGH, 0, "(densityWrapper)Start for char %d, curChar, nCount: %d, rCount: %d\n", curChar, nCount, rCount); }
int densityApplicable = FALSE;
if(nCount != 0 && rCount != 0 && nCount != rCount){
    initializeBlockData(&bD, nCount);
    applyDensity(curChar, rp, rCount, nCount, od, bD);
    densityInfo(curChar, nCount, rCount, bD);
    int k = 0;
    //for(k = 0; k < rCount; k++){ printf("%d ", rp[k]); }
    densityApplicable = checkDensityApplicability(bD, curChar, nCount, rCount);
    if(densityApplicable){
        //printf("(densityWrapper)Density is applicable\n");
        if(LOG){ debug(VERY_HIGH, 1, "(densityWrapper)Density is applicable\n"); }
        while(*combinedBlockIdLen + sizeof(unsigned char)*bD->blockIdLen >= *combinedBlockIdAllocSize){ doubleUpChar(combinedBlockId, combinedBlockIdAllocSize, *combinedBlockIdLen); }
        memcpy(*combinedBlockId + *combinedBlockIdLen, bD->blockId, sizeof(unsigned char)*bD->blockIdLen); *combinedBlockIdLen += bD->blockIdLen;
        if(LOG){ debug(HIGH, 1, "(densityWrapper)Current combinedBlockIdLen: %d\n", *combinedBlockIdLen); }
        //compressBlockIds(bD, od);
        for(k = 0; k < bD->blockTypesCount; k++){
            if(bD->blockRPNCount[k] != 0 && bD->blockRPCount[k] != 0 && bD->blockRPNCount[k] != bD->blockRPCount[k]){
                if(LOG){ debug(MEDIUM, 3, "(densityWrapper)BlockType%d: nCount: %d, rCount: %d\n", k, bD->blockRPNCount[k], bD->blockRPCount[k]); }
                int* nRP = (int*)malloc(sizeof(int)*(bD->blockRPCount[k]+1)); memset(nRP, 0, sizeof(int)*(bD->blockRPCount[k]+1));
                int l = 0; nRP[0] = 1; for(l = 0; l < bD->blockRPCount[k]; l++){ nRP[l+1] = bD->blockRP[k][l]+1; }
                superEncodePos(od, curChar, nRP, bD->blockRPCount[k]+1, bD->blockRPNCount[k]+1);
                writeDensityNDataToBuf(od, bD->blockRPNCount[k]); //nCountRandomBuf
                writeDensityNDataToBuf(od, bD->blockRPCount[k]); //nCountRandomBuf
                if(nRP != NULL){ free(nRP); }
            } else {
                if(LOG){ debug(VERY_HIGH, 2, "CHOK Encoding not necessary for n: %d, r: %d\n", bD->blockRPNCount[k], bD->blockRPCount[k]); }
                if(LOG){ debug(MEDIUM, 3, "(densityWrapper)BlockType%d: nCount: %d, rCount: %d Encoding not required\n", k, bD->blockRPNCount[k], bD->blockRPCount[k]); }
                writeDensityNDataToBuf(od, bD->blockRPNCount[k]); //nCountRandomBuf
                writeDensityNDataToBuf(od, bD->blockRPCount[k]); //nCountRandomBuf
            }
        }
        od->densityEncodedBufCount++;
    } else { //else of densityApplicable
        if(LOG){ debug(VERY_HIGH, 1, "(densityWrapper)Density is not applicable\n"); }
        int* nRP = (int*)malloc(sizeof(int)*(rCount+1)); memset(nRP, 0, sizeof(int)*(rCount+1));
        int l = 0; nRP[0] = 1; for(l = 0; l < rCount; l++){ nRP[l+1] = rp[l]+1; }
        superEncodePos(od, curChar, nRP, rCount+1, nCount+1);
        if(nRP != NULL){ free(nRP); }
    }
    blockDataCleanup(bD);
} else { if(LOG){ debug(VERY_HIGH, 2, "CHOK Encoding not necessary for n: %d, r: %d\n", nCount, rCount); } }
return densityApplicable;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to cleanup all the allocations */
/* Input: */
/* Output: */
void encCleanup(CD* od){
if(LOG){ debug(VERY_HIGH, 0, "(encCleanup)Start\n"); }
//fclose(od->of);
if(od->charData != NULL){ free(od->charData); } if(LOG){ debug(VERY_HIGH, 0, "After freeing charData\n"); }
//if(od->fileBuf != NULL){ free(od->fileBuf); } if(LOG){ debug(VERY_HIGH, 0, "After freeing fileBuf\n"); }
if(od->outBuf != NULL){ free(od->outBuf); } if(LOG){ debug(VERY_HIGH, 0, "After freeing outBuf\n"); }
if(od->blockIdBuf != NULL){ free(od->blockIdBuf); } if(LOG){ debug(VERY_HIGH, 0, "After freeing blockIdBuf\n"); }
if(od->partialBytesBuf != NULL){ free(od->partialBytesBuf); } if(LOG){ debug(VERY_HIGH, 0, "After freeing partialBytesBuf\n"); }
if(od->partialBytes != NULL){ free(od->partialBytes); } if(LOG){ debug(VERY_HIGH, 0, "After freeing partialBytes\n"); }
if(od->nCountBuf != NULL){ free(od->nCountBuf); } if(LOG){ debug(VERY_HIGH, 0, "After freeing nCountBuf\n"); }
if(od->nCountBytes != NULL){ free(od->nCountBytes); } if(LOG){ debug(VERY_HIGH, 0, "After freeing nCountBytes\n"); }
if(od->rCountBuf != NULL){ free(od->rCountBuf); } if(LOG){ debug(VERY_HIGH, 0, "After freeing rCountBuf\n"); }
if(od->nCountRandomBuf != NULL){ free(od->nCountRandomBuf); } if(LOG){ debug(VERY_HIGH, 0, "After freeing nCountRandomBuf\n"); }
if(od->firstValDec != NULL){ free(od->firstValDec); } if(LOG){ debug(VERY_HIGH, 0, "After freeing firstValDec\n"); }
if(od->splitChars != NULL){ free(od->splitChars); } if(LOG){ debug(VERY_HIGH, 0, "After freeing splitChars\n"); }
if(od->splitDensityIdBits != NULL){  free(od->splitDensityIdBits); } if(LOG){ debug(VERY_HIGH, 0, "After freeing splitDensityIdBits\n"); }
if(od->splitDensityIdBytes != NULL){ free(od->splitDensityIdBytes); } if(LOG){ debug(VERY_HIGH, 0, "After freeing splitDensityIdBytes\n"); }
if(od->encValLen != NULL){ free(od->encValLen); } if(LOG){ debug(VERY_HIGH, 0, "After freeing encValLen\n"); }
//if(od != NULL){ free(od); } if(LOG){ debug(VERY_HIGH, 0, "After freeing od\n"); }
}

void preSplit(unsigned char curChar, int* rp, int rCount, int nCount){
int z = 0, cTemp = 0, prevCTemp = 0, ct = 0, count = 0, cache = 0;
//FILE* f1 = (FILE*)openFile(HIGH, "newRP.txt", "wb");
//printf("cTemp: ");
for(z = 0; z < rCount; z++){ 
    cTemp = rp[z]-(z+1); 
    if(cTemp == prevCTemp){ 
       if(cTemp != cache){ count++; }
       ct++; 
       cache = cTemp;
    } 
    printf("%d ", cTemp); 
    prevCTemp = cTemp; 
}
printf("\n"); printf("ct: %d\n", ct);
printf("count: %d\n", count);
//fclose(f1);

int i = 0;
unsigned int* newRP = (unsigned int*)malloc(sizeof(unsigned int)*rCount); memset(newRP, 0, sizeof(unsigned int)*rCount);
unsigned int* redRP = (unsigned int*)malloc(sizeof(unsigned int)*rCount); memset(redRP, 0, sizeof(unsigned int)*rCount);
unsigned int* repeatPos = (unsigned int*)malloc(sizeof(unsigned int)*rCount); memset(repeatPos, 0, sizeof(unsigned int)*rCount);
int redRPCount = 0, repeatPosCount = 0, curValue = 0, prevValue = 0, prevRepeatValue = 0, repeatGroupCount = 0;
printf("newRP: ");
for(i = 0; i < rCount; i++){
    curValue = newRP[i] = rp[i] - (i+1) + 1;
    printf("%d ", newRP[i]);
    if(i == 0){
        redRP[redRPCount++] = curValue;
    } else {
        if(curValue != prevValue){
            redRP[redRPCount++] = curValue;
        } else {
            if(curValue != prevRepeatValue){
                repeatGroupCount++;
            }
            //repeatPos[repeatPosCount++] = i+1 - repeatGroupCount;
            repeatPos[repeatPosCount++] = i+1;
            prevRepeatValue = curValue;
        }
    }
    prevValue = curValue;
}
printf("\n");
printf("repeatGroupCount: %d\n", repeatGroupCount);
printf("redRPCount: %d(%d) = %d\n", redRPCount, newRP[rCount-1], nCrBitLen(newRP[rCount-1], redRPCount));
printf("repeatPosCount: %d(%d --> %d) = %d --> %d\n", repeatPosCount, rCount, rCount - repeatGroupCount, nCrBitLen(rCount, repeatPosCount), nCrBitLen(rCount-repeatGroupCount, repeatPosCount));

unsigned char *tempRP = (unsigned char*)malloc(sizeof(unsigned char)*newRP[rCount-1]); memset(tempRP, 1, sizeof(unsigned char)*newRP[rCount-1]);
for(i = 0; i < redRPCount; i++){ tempRP[redRP[i]-1] = 0; }
FILE* f1 = (FILE*)openFile(HIGH, "rp1.txt", "wb"); fwrite(tempRP, sizeof(unsigned char), newRP[rCount-1], f1); fclose(f1);

unsigned char *repeatRP = (unsigned char*)malloc(sizeof(unsigned char)*rCount); memset(repeatRP, 1, sizeof(unsigned char)*rCount);
for(i = 0; i < repeatPosCount; i++){ repeatRP[repeatPos[i]-1] = 0; }
FILE* f2 = (FILE*)openFile(HIGH, "rp2.txt", "wb"); fwrite(repeatRP, sizeof(unsigned char), rCount, f2); fclose(f2);

if(repeatRP != NULL){ free(repeatRP); }
if(tempRP != NULL){ free(tempRP); }
if(repeatPos != NULL){ free(repeatPos); }
if(redRP != NULL){ free(redRP); }
if(newRP != NULL){ free(newRP); }
}


void writeInBuf(unsigned char** dest, unsigned int* destCurLen, unsigned int* destAllocSize, unsigned char* src, unsigned int unitSize, unsigned int unitsToWrite){
while(*destCurLen + unitSize*unitsToWrite >= *destAllocSize){ doubleUpChar(dest, destAllocSize, *destCurLen); }
memcpy(*dest + *destCurLen, src, unitSize*unitsToWrite); *destCurLen += unitSize*unitsToWrite;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to encode a character buffer by Vajra */
/* Input: */
/* Output: */
void encodeBuffer(CD* od){
#ifdef DEBUG
if(LOG){ debug(VERY_HIGH, 1, "(encodeBuffer)len: %d\n", od->inBufLen); }
#endif

int i = 0, j = 0, k = 0, l = 0, curChar = -1, distinctCharCount = 0;

//Find the order for encoding characters, based on their frequency
ORDER vj_order[MAX_CHARS];
struct timeval tStart, tEnd;
if(TL){ if(gettimeofday(&tStart, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
findEncodingOrder(od->logLevel, od->inBuf, od->inBufLen, vj_order);
if(TL){ if(gettimeofday(&tEnd, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
//if(TL){ printf("findEncodingOrder TIME : %lld\n", (uint64_t)timeval_diff(NULL, &tEnd, &tStart)); }


#ifdef DEBUG
if(LOG){
    if(LOG){ debug(LOW, 1, "(encodeBuffer)Order of characters as per their decreasing order of frequency: \n"); }
    for(i = 0; i < MAX_CHARS; i++){ if(vj_order[i].count != 0){ if(LOG){ debug(LOW, 2, "ch: %d, count: %d ", vj_order[i].ch, vj_order[i].count); } if(LOG){ debug(LOW, 1, "order: %d\n", vj_order[i].ch); } } }
    if(LOG){ debug(LOW, 1, "(encodeBuffer)Order of characters as per their decreasing order of frequency: \n"); }
    for(i = 0; i < MAX_CHARS; i++){ if(vj_order[i].count != 0){ if(LOG){ debug(LOW, 1, "%d ", vj_order[i].ch); } } }
    if(LOG){ debug(LOW, 1, "\n"); }
}
#endif

//Obtain the relative positions based on the above frequency based order
unsigned int* rp[MAX_CHARS];
unsigned int rpCount[MAX_CHARS];

//Allocate and Initialise the Relative Positions array
initializeRelativePos(od->logLevel, rp, rpCount, vj_order);
#ifdef DEBUG
if(LOG){
	for(i = 0; i < MAX_CHARS; i++){
		if(LOG){ debug(LOW, 1, "(encodeBuffer)rpCount[%d]: %d\n", i, rpCount[i]); }
	}
}
#endif

//Fill up the Relative Positions
//if(TL){ if(gettimeofday(&tStart, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
//fillRelativePos(od->logLevel, od->inBuf, od->inBufLen, vj_order, rp, rpCount);
//if(TL){ if(gettimeofday(&tEnd, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
//if(TL){ printf("fillRelativePos TIME : %lld\n", (uint64_t)timeval_diff(NULL, &tEnd, &tStart)); }

if(TL){ if(gettimeofday(&tStart, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
fillRelativePosFast(od->logLevel, od->inBuf, od->inBufLen, vj_order, rp, rpCount);
//fillRelativePosFast(od->logLevel, od->inBuf, od->inBufLen, vj_order);
if(TL){ if(gettimeofday(&tEnd, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
//if(TL){ printf("fillRelativePosFast TIME : %lld\n", (uint64_t)timeval_diff(NULL, &tEnd, &tStart)); }

#ifdef DEBUG
//Display the Relative Positions
if(LOG){ displayRelativePos(od->logLevel, rp, rpCount, vj_order, od->inBufLen); }
#endif

//Find the Rank for every character
int charsRank[MAX_CHARS];
memset(charsRank, 0, sizeof(charsRank));
for(i = 0; i < MAX_CHARS; i++){ charsRank[vj_order[i].ch] = i + 1; if(vj_order[i].count != 0){ distinctCharCount++; } }

//Find the N count for every character
int charsNCount[MAX_CHARS];
findNCount(od->logLevel, vj_order, od->inBufLen, rpCount, charsNCount);
#ifdef DEBUG
if(LOG){ displayNCount(od->logLevel, vj_order, charsNCount); }
#endif

//Update outfile and outbuffer pointers
//od = (CD*)malloc(sizeof(CD));
od->inCharCount = od->inBufLen;
//od->charData = (unsigned char*)malloc(sizeof(unsigned char)*od->inCharCount); memset(od->charData, 0, sizeof(unsigned char)*od->inCharCount); od->charDataCount = 0;
od->charData = (unsigned char*)malloc(sizeof(unsigned char)*MAX_CHARS); memset(od->charData, 0, sizeof(unsigned char)*MAX_CHARS); od->charDataCount = 0;
od->charDataAllocSize = MAX_CHARS;
//od->of = (FILE*)openFile(od->logLevel, "outEnc.txt", "wb");
od->ofDataLen = 0;
od->fileBuf = (unsigned char*)malloc(sizeof(unsigned char)*(od->inBufLen/4)); memset(od->fileBuf, 0, sizeof(unsigned char)*(od->inBufLen/4)); od->fileBufLen = 0;
od->fileBufAllocSize = od->inBufLen/4;
//printf("fileBufAllocSize: %d\n", od->fileBufAllocSize);
od->outBuf = (unsigned char*)malloc(sizeof(unsigned char)*(od->inBufLen/4)); memset(od->outBuf, 0, sizeof(unsigned char)*(od->inBufLen/4)); od->outBufLen = 0;
od->outBufAllocSize = od->inBufLen/4;

od->partialBytesBuf = NULL;
od->partialBytes = NULL;
//od->partialBytesBuf = (unsigned char*)malloc(sizeof(unsigned char)*od->inBufLen); //check len*2
//memset(od->partialBytesBuf, 0, sizeof(unsigned char)*od->inBufLen); od->partialBytesBufLen = 0;
od->emptyBytesCount = 0;

od->distinctCharCount = distinctCharCount;

od->nCountBufAllocSize = sizeof(unsigned int)*8*od->distinctCharCount;
od->nCountBuf = (unsigned char*)malloc(sizeof(unsigned char)*od->nCountBufAllocSize);
memset(od->nCountBuf, 0, sizeof(unsigned char)*od->nCountBufAllocSize);
od->nCountBufLen = 0;

od->rCountBufAllocSize = sizeof(unsigned int)*od->distinctCharCount;
od->rCountBuf = (unsigned int*)malloc(sizeof(unsigned int)*od->distinctCharCount);
memset(od->rCountBuf, 0, sizeof(unsigned int)*od->distinctCharCount);
od->rCountBufLen = 0;

od->firstValDecAllocSize = sizeof(unsigned char)*3*D_BLOCK_TYPES*od->distinctCharCount; //it cannot excees 3times(split) X 4times(density)
od->firstValDec = (unsigned char*)malloc(sizeof(unsigned char)*3*D_BLOCK_TYPES*od->distinctCharCount);
memset(od->firstValDec, 0, sizeof(unsigned char)*3*D_BLOCK_TYPES*od->distinctCharCount);
od->firstValDecLen = 0;

od->encValLenAllocSize = sizeof(unsigned int)*3*D_BLOCK_TYPES*od->distinctCharCount;
od->encValLen = (unsigned int*)malloc(sizeof(unsigned int)*3*D_BLOCK_TYPES*od->distinctCharCount);
memset(od->encValLen, 0, sizeof(unsigned int)*3*D_BLOCK_TYPES*od->distinctCharCount);
od->encValCount = 0;

od->nCountRandomBufAllocSize = sizeof(unsigned int)*(3+1)*2*(D_BLOCK_TYPES+1)*od->distinctCharCount;
od->nCountRandomBuf = (unsigned int*)malloc(sizeof(unsigned int)*(3+1)*2*(D_BLOCK_TYPES+1)*od->distinctCharCount);
memset(od->nCountRandomBuf, 0, sizeof(unsigned int)*(3+1)*2*(D_BLOCK_TYPES+1)*od->distinctCharCount);
od->nCountRandomBufLen = 0;
od->nCountBytes = NULL;

od->splitCharsAllocSize = MAX_CHARS;
od->splitChars = (unsigned char*)malloc(sizeof(unsigned char)*MAX_CHARS); memset(od->splitChars, 0, sizeof(unsigned char)*MAX_CHARS);
od->splitCharsCount = 0;
od->splitDensityIdBitsAllocSize = MAX_CHARS*3;
od->splitDensityIdBits = (unsigned char*)malloc(sizeof(unsigned char)*MAX_CHARS*3); memset(od->splitDensityIdBits, 0, sizeof(unsigned char)*MAX_CHARS*3);
od->splitDensityIdBitsCount = 0;

od->blockIdBuf = NULL;
od->blockIdBufLen = 0;
od->densityEncodedBufCount = 0;
od->blockIdCompressMode = 0;

BD* bD = NULL; //For Block Data
SD* sD = NULL; //For Split Data

unsigned int combinedBlockIdAllocSize = 2*(od->inCharCount/D_BLOCK_SIZE);
unsigned char* combinedBlockId = (unsigned char*)malloc(sizeof(unsigned char)*combinedBlockIdAllocSize);
memset(combinedBlockId, 0, sizeof(unsigned char)*(combinedBlockIdAllocSize));
unsigned int combinedBlockIdLen = 0;

struct timeval t1, t2, tDiff;
uint64_t tt = 0LL, ttSum = 0LL;
int bitLen = 0, densityApplicable = FALSE, basicCheckCleared = FALSE, splitApplicable = FALSE;

int nTotal = 0, rTotal = 0;
FILE* tf = (FILE*)openFile(od->logLevel, "onlyZero.txt", "wb");

//Encode the relative positions by vajra without using density
for(i = 0; i < MAX_CHARS; i++){
    if(vj_order[i].count != 0){
        if(LOG){ debug(VERY_HIGH, 0, "*************************************************************************\n"); }
        curChar = vj_order[i].ch;
        if(LOG){ debug(HIGH, 4, "CHAR: %d, N: %d, R: %d, density: %d\n", curChar, charsNCount[curChar], rpCount[curChar], (rpCount[curChar]*100)/charsNCount[curChar]); }
        //printf("CHAR: %d, N: %d, R: %d, density: %d\n", curChar, charsNCount[curChar], rpCount[curChar], (rpCount[curChar]*100)/charsNCount[curChar]);

        nTotal += charsNCount[curChar]; rTotal += rpCount[curChar];
        if(curChar == 3){
            unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char)*charsNCount[curChar]);
            if(rpCount[curChar]*2 > charsNCount[curChar]){ memset(temp, 1, sizeof(unsigned char)*charsNCount[curChar]); }
            else { memset(temp, 0, sizeof(unsigned char)*charsNCount[curChar]); }
            int z = 0;
            //preSplit(curChar, rp[curChar], rpCount[curChar], charsNCount[curChar]);
            if(rpCount[curChar]*2 > charsNCount[curChar]){ for(z = 0; z < rpCount[curChar]; z++){ temp[rp[curChar][z]-1] = 0; } }
            else { for(z = 0; z < rpCount[curChar]; z++){ temp[rp[curChar][z]-1] = 1; } }
            fwrite(temp, sizeof(unsigned char), charsNCount[curChar], tf);
            if(temp != NULL){ free(temp); }
        }
        /*
        int z = 0;
        printf("RRPP(%d)(n:%d)(r:%d) - ", curChar, charsNCount[curChar], rpCount[curChar]);
        for(z = 0; z < rpCount[curChar]; z++){
            printf("%d ", rp[curChar][z]);
        }
        printf("\n");
        */ 

        if(FULL_GMP){
            if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
            //printf("Current time is %ld seconds and %ld microseconds\n", t1.tv_sec, t1.tv_usec);
            //if(curChar == 14){
                //applyDensity(curChar, rp[curChar], rpCount[curChar], charsNCount[curChar], od, bD);
            //}
            encodePos(curChar, rp[curChar], rpCount[curChar], charsNCount[curChar], od->inBufLen, od);
            if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
            //printf("Current time is %ld seconds and %ld microseconds\n", t2.tv_sec, t2.tv_usec);
            if(TL){ ttSum += tt = timeval_diff(NULL, &t2, &t1); }
            if(TL){ printf("For char %d TIME: %lld ", curChar, tt); printf("sum: %lld\n", ttSum); }
        }

            //struct timeval t1, t2;
            //if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
            //if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday() call...\n"); } } //Time logging
            //if(TL){ printf("MAGIC SPLIT TIME : %lld\n", (uint64_t)timeval_diff(NULL, &t2, &t1)); }

        if(!FULL_GMP){
            //initializeBlockData(&bD, charsNCount[curChar]);
            basicCheckCleared = basicCheck(curChar, charsNCount[curChar], rpCount[curChar]);
            if(basicCheckCleared == TRUE){
                magicSplit(curChar, rp[curChar], rpCount[curChar], charsNCount[curChar], &sD);
                splitApplicable = checkSplitApplicability(sD, curChar, charsNCount[curChar], rpCount[curChar]);
                if(splitApplicable){
                    //printf("Split Applicable\n");
                    if(LOG){ debug(HIGH, 0, "Invoke densityWrapper for rpZero of char %d\n", curChar); }
                    //printf("Invoke densityWrapper for rpZero of char %d\n", curChar);
                    writeDensityNDataToBuf(od, sD->rpZeroN); writeDensityNDataToBuf(od, sD->rpZeroCount); //nCountRandomBuf
                    //printf("NKK: rpZeroN: %d, rpZeroCount: %d\n", sD->rpZeroN, sD->rpZeroCount);
                    if(densityWrapper(curChar, sD->rpZero, sD->rpZeroN, sD->rpZeroCount, bD, od, &combinedBlockId, &combinedBlockIdLen, &combinedBlockIdAllocSize)){
                        if(sD->rpZeroN != 0 && sD->rpZeroCount != 0 && sD->rpZeroN != sD->rpZeroCount){ 
                            od->splitDensityIdBits[od->splitDensityIdBitsCount++] = 1;
                            if(od->splitDensityIdBitsCount > od->splitDensityIdBitsAllocSize){ printf("FATAL ERROR: splitDensityIdBitsCount exceeds splitDensityIdBitsAllocSize\n"); }
                            if(LOG){ debug(HIGH, 0, "Id bit set to 1\n"); }
                        }
                    } else { 
                        if(sD->rpZeroN != 0 && sD->rpZeroCount != 0 && sD->rpZeroN != sD->rpZeroCount){ 
                            od->splitDensityIdBits[od->splitDensityIdBitsCount++] = 0;
                            if(od->splitDensityIdBitsCount > od->splitDensityIdBitsAllocSize){ printf("FATAL ERROR: splitDensityIdBitsCount exceeds splitDensityIdBitsAllocSize\n"); }
                            if(LOG){ debug(HIGH, 0, "Id bit set to 0\n"); }
                        }
                    }
                    if(LOG){ debug(HIGH, 0, "Invoke densityWrapper for rpOne of char %d\n", curChar); }
                    //printf("Invoke densityWrapper for rpOne of char %d\n", curChar);
                    writeDensityNDataToBuf(od, sD->rpOneN); writeDensityNDataToBuf(od, sD->rpOneCount); //nCountRandomBuf
                    //printf("NKK: rpOneN: %d, rpOneCount: %d\n", sD->rpZeroN, sD->rpZeroCount);
                    if(densityWrapper(curChar, sD->rpOne, sD->rpOneN, sD->rpOneCount, bD, od, &combinedBlockId, &combinedBlockIdLen, &combinedBlockIdAllocSize)){
                        if(sD->rpOneN != 0 && sD->rpOneCount != 0 && sD->rpOneN != sD->rpOneCount){ 
                            od->splitDensityIdBits[od->splitDensityIdBitsCount++] = 1;
                            if(od->splitDensityIdBitsCount > od->splitDensityIdBitsAllocSize){ printf("FATAL ERROR: splitDensityIdBitsCount exceeds splitDensityIdBitsAllocSize\n"); }
                            if(LOG){ debug(HIGH, 0, "Id bit set to 1\n"); }
                        }
                    } else { 
                        if(sD->rpOneN != 0 && sD->rpOneCount != 0 && sD->rpOneN != sD->rpOneCount){ 
                            od->splitDensityIdBits[od->splitDensityIdBitsCount++] = 0;
                            if(od->splitDensityIdBitsCount > od->splitDensityIdBitsAllocSize){ printf("FATAL ERROR: splitDensityIdBitsCount exceeds splitDensityIdBitsAllocSize\n"); }
                            if(LOG){ debug(HIGH, 0, "Id bit set to 0\n"); } 
                        }
                    }
                    if(LOG){ debug(HIGH, 0, "Invoke densityWrapper for splitId of char %d\n", curChar); }
                    //printf("Invoke densityWrapper for splitId of char %d\n", curChar);
                    writeDensityNDataToBuf(od, sD->rpSplitIdN); writeDensityNDataToBuf(od, sD->rpSplitIdCount); //nCountRandomBuf
                    //printf("NKK: rpSplitIdN: %d, rpSplitIdCount: %d\n", sD->rpZeroN, sD->rpZeroCount);
                    if(densityWrapper(curChar, sD->rpSplitId, sD->rpSplitIdN, sD->rpSplitIdCount, bD, od, &combinedBlockId, &combinedBlockIdLen, &combinedBlockIdAllocSize)){
                        if(sD->rpSplitIdN != 0 && sD->rpSplitIdCount != 0 && sD->rpSplitIdN != sD->rpSplitIdCount){ 
                            od->splitDensityIdBits[od->splitDensityIdBitsCount++] = 1;
                            if(od->splitDensityIdBitsCount > od->splitDensityIdBitsAllocSize){ printf("FATAL ERROR: splitDensityIdBitsCount exceeds splitDensityIdBitsAllocSize\n"); }
                            if(LOG){ debug(HIGH, 0, "Id bit set to 1\n"); }
                        }
                    } else { 
                        if(sD->rpSplitIdN != 0 && sD->rpSplitIdCount != 0 && sD->rpSplitIdN != sD->rpSplitIdCount){ 
                            od->splitDensityIdBits[od->splitDensityIdBitsCount++] = 0;
                            if(od->splitDensityIdBitsCount > od->splitDensityIdBitsAllocSize){ printf("FATAL ERROR: splitDensityIdBitsCount exceeds splitDensityIdBitsAllocSize\n"); }
                            if(LOG){ debug(HIGH, 0, "Id bit set to 0\n"); } 
                        }
                    }

                    od->splitChars[od->splitCharsCount++] = curChar;
                    //printf("splitCharsCount: %d, curChar: %d, splitDensityIdBitsCount: %d, n: %d, r: %d\n", od->splitCharsCount, curChar, od->splitDensityIdBitsCount, charsNCount[curChar], rpCount[curChar]);
                    if(od->splitCharsCount > od->splitCharsAllocSize){ printf("FATAL ERROR: splitCharsCount(%d) exceeds splitCharsAllocSize(%d)\n", od->splitCharsCount, od->splitCharsAllocSize); }
                } else { //else for splitApplicable
                    //if(LOG){ debug(HIGH, 0, "Invoke densityWrapper for char %d\n", curChar); }
                    //densityWrapper(curChar, rp[curChar], charsNCount[curChar], rpCount[curChar], bD, od, &combinedBlockId, &combinedBlockIdLen, &combinedBlockIdAllocSize);
                    if(LOG){ debug(HIGH, 0, "Invoke vajra encoding for char %d\n", curChar); }
                    if(charsNCount[curChar] != 0 && rpCount[curChar] != 0 && charsNCount[curChar] != rpCount[curChar]){
                        int* nRP = (int*)malloc(sizeof(int)*(rpCount[curChar]+1)); memset(nRP, 0, sizeof(int)*(rpCount[curChar]+1));
                        int l = 0; nRP[0] = 1; for(l = 0; l < rpCount[curChar]; l++){ nRP[l+1] = rp[curChar][l]+1; }
                        superEncodePos(od, curChar, nRP, rpCount[curChar]+1, charsNCount[curChar]+1);
                        if(nRP != NULL){ free(nRP); }
                    } else { if(LOG){ debug(VERY_HIGH, 2, "CHOK Encoding not necessary for n: %d and r:%d\n", charsNCount[curChar], rpCount[curChar]); } }
                } //end of splitApplicable
                splitDataCleanup(sD);
            } else {
                if(LOG){ debug(HIGH, 0, "Invoke vajra encoding for char %d\n", curChar); }
                if(charsNCount[curChar] != 0 && rpCount[curChar] != 0 && charsNCount[curChar] != rpCount[curChar]){
                    int* nRP = (int*)malloc(sizeof(int)*(rpCount[curChar]+1)); memset(nRP, 0, sizeof(int)*(rpCount[curChar]+1));
                    int l = 0; nRP[0] = 1; for(l = 0; l < rpCount[curChar]; l++){ nRP[l+1] = rp[curChar][l]+1; }
                    superEncodePos(od, curChar, nRP, rpCount[curChar]+1, charsNCount[curChar]+1);
                    if(nRP != NULL){ free(nRP); }
                } else { if(LOG){ debug(VERY_HIGH, 2, "CHOK Encoding not necessary for n: %d and r:%d\n", charsNCount[curChar], rpCount[curChar]); } }
            }
        }
        writeNCountToBuf(charsNCount[curChar], od);
        writeRCountToBuf(rpCount[curChar], od);
        writeCharDataToBuf((unsigned char)curChar, od);
    }
}

fclose(tf);

if(combinedBlockIdLen != 0){
    //printf("compressCombined hk\n");
    compressCombinedBlockId(combinedBlockId, combinedBlockIdLen, od);
}
if(combinedBlockId != NULL){ free(combinedBlockId); }
finalizeNCountInBuf(od);
//finalizePartialBytes(od);
finalizeSplitDensityIdBits(od);
//extractNCountFromBuf(od->nCountBytes, od->nCountBufLen, len);

//Encode the relative positions by vajra using density

if(LOG){ debug(od->logLevel, 1, "\n"); }

//Write vajra compression identifier
unsigned char vajraId[2] = { 'V', 'J'};
while(od->fileBufLen + sizeof(unsigned char)*2 >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &vajraId[0], sizeof(unsigned char)); od->fileBufLen += sizeof(unsigned char);
memcpy(od->fileBuf + od->fileBufLen, &vajraId[1], sizeof(unsigned char)); od->fileBufLen += sizeof(unsigned char);
if(LOG){ debug(od->logLevel, 1, "WROTE vajra identifier %c%c using %d bytes\n", vajraId[0], vajraId[1], sizeof(unsigned char)*2); }

//Write total file size of outBuf
while(od->fileBufLen + sizeof(unsigned int) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &od->inCharCount, sizeof(unsigned int)); od->fileBufLen += sizeof(unsigned int);
if(LOG){ debug(od->logLevel, 1, "WROTE fileSize %d using %d bytes\n", od->inCharCount, sizeof(unsigned int)); }

//Write total number of distinct characters
while(od->fileBufLen + sizeof(unsigned short) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &od->charDataCount, sizeof(unsigned short)); od->fileBufLen += sizeof(unsigned short);
if(LOG){ debug(od->logLevel, 1, "WROTE number of distinct characters %d using %d bytes\n", od->charDataCount, sizeof(unsigned short)); }


//Write the distinct characters
while(od->fileBufLen + sizeof(unsigned char)*od->charDataCount >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, od->charData, sizeof(unsigned char)*od->charDataCount); od->fileBufLen += sizeof(unsigned char)*od->charDataCount;
if(LOG){ debug(od->logLevel, 1, "WROTE charData using %d bytes\n", sizeof(unsigned char)*od->charDataCount); }
if(LOG){ displayEncCharData(od); }

//Write the length of nCount(nCountBufLen) as the number of bits
while(od->fileBufLen + sizeof(unsigned short) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &od->nCountBufLen, sizeof(unsigned short)); od->fileBufLen += sizeof(unsigned short);
if(LOG){ debug(od->logLevel, 1, "WROTE nCountBufLen %d using %d bytes\n", od->nCountBufLen, sizeof(unsigned short)); }

//Write the nCountBytes 
while(od->fileBufLen + sizeof(unsigned char)*(od->nCountBufLen/8) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, od->nCountBytes, sizeof(unsigned char)*(int)ceil((float)od->nCountBufLen/(float)8)); 
od->fileBufLen += sizeof(unsigned char)*(int)ceil((float)od->nCountBufLen/(float)8);
if(LOG){ debug(od->logLevel, 1, "WROTE nCountBytes %d bytes\n", sizeof(unsigned char)*(int)ceil((float)od->nCountBufLen/(float)8)); }

//Write the rCountBuf
while(od->fileBufLen + sizeof(unsigned int)*od->rCountBufLen >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, od->rCountBuf, sizeof(unsigned int)*od->rCountBufLen); 
od->fileBufLen += sizeof(unsigned int)*od->rCountBufLen;
if(LOG){ debug(od->logLevel, 1, "WROTE rCountBuf %d bytes\n", sizeof(unsigned int)*od->rCountBufLen); }

//Write the number of chars applicable for split
while(od->fileBufLen + sizeof(unsigned char) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &od->splitCharsCount, sizeof(unsigned char)); 
od->fileBufLen += sizeof(unsigned char);
if(LOG){ debug(od->logLevel, 1, "WROTE number of split chars as %d using %d bytes\n", od->splitCharsCount, sizeof(unsigned char)); }

//Write the splitChars
//if(od->splitCharsCount != 0){
while(od->fileBufLen + sizeof(unsigned char)*od->splitCharsCount >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, od->splitChars, sizeof(unsigned char)*od->splitCharsCount); 
od->fileBufLen += sizeof(unsigned char)*od->splitCharsCount; 
if(LOG){ debug(od->logLevel, 1, "WROTE splitChars using %d bytes\n", sizeof(unsigned char)*od->splitCharsCount); }
//}

//Write the splitDensityIdBytesLen
//if(od->splitCharsCount != 0){
while(od->fileBufLen + sizeof(unsigned char) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &od->splitDensityIdBytesLen, sizeof(unsigned char)); 
od->fileBufLen += sizeof(unsigned char);
if(LOG){ debug(od->logLevel, 1, "WROTE splitDensityIdBytesLen as %d using %d bytes\n", od->splitDensityIdBytesLen, sizeof(unsigned char)); }
//}

//Write the splitDensityIdBitsCount
//if(od->splitCharsCount != 0){
while(od->fileBufLen + sizeof(unsigned short) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &od->splitDensityIdBitsCount, sizeof(unsigned short)); 
od->fileBufLen += sizeof(unsigned short);
if(LOG){ debug(od->logLevel, 1, "WROTE splitDensityIdBitsCount as %d using %d bytes\n", od->splitDensityIdBitsCount, sizeof(unsigned short)); }
//}

//Write the splitDensityIdBytes
//if(od->splitCharsCount != 0){
//while(od->fileBufLen + sizeof(unsigned char)*(od->splitCharsCount*3/8) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
while(od->fileBufLen + sizeof(unsigned char)*(od->splitDensityIdBytesLen) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, od->splitDensityIdBytes, sizeof(unsigned char)*od->splitDensityIdBytesLen); 
od->fileBufLen += sizeof(unsigned char)*od->splitDensityIdBytesLen;
if(LOG){ debug(od->logLevel, 1, "WROTE splitDensityIdBytes using %d bytes\n", sizeof(unsigned char)*od->splitDensityIdBytesLen); }
//}

//Write the number of buffers encoded by density
while(od->fileBufLen + sizeof(unsigned char) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &od->densityEncodedBufCount, sizeof(unsigned char)); 
od->fileBufLen += sizeof(unsigned char);
if(LOG){ debug(od->logLevel, 1, "WROTE number of density encoded buf as %d using %d bytes\n", od->densityEncodedBufCount, sizeof(unsigned char)); }
//printf("densityEncodedBufCount: %d\n", od->densityEncodedBufCount);

//Write the nCountRandomBufLen
while(od->fileBufLen + sizeof(unsigned short) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &od->nCountRandomBufLen, sizeof(unsigned short)); 
od->fileBufLen += sizeof(unsigned short);
if(LOG){ debug(od->logLevel, 1, "WROTE nCountRandomBufLen as %d using %d bytes\n", od->nCountRandomBufLen, sizeof(unsigned short)); }

//Write the nCounts(4/8 each) of all the buffers encoded by density
//if(od->densityEncodedBufCount != 0){
while(od->fileBufLen + sizeof(unsigned int)*od->nCountRandomBufLen >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, od->nCountRandomBuf, sizeof(unsigned int)*od->nCountRandomBufLen); 
od->fileBufLen += sizeof(unsigned int)*od->nCountRandomBufLen; 
if(LOG){ debug(od->logLevel, 1, "WROTE nCountRandomBuf using %d bytes\n", sizeof(unsigned int)*od->nCountRandomBufLen); }
//printf("WROTE nCountRandomBuf using %d bytes\n", sizeof(unsigned int)*od->nCountRandomBufLen);
//for(i = 0; i < od->nCountRandomBufLen; i++){ printf("%d ", od->nCountRandomBuf[i]); }
//}

//Write the mode for blockIds compress
//if(od->densityEncodedBufCount != 0){
while(od->fileBufLen + sizeof(unsigned char) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &od->blockIdCompressMode, sizeof(unsigned char)); od->fileBufLen += sizeof(unsigned char);
if(LOG){ debug(od->logLevel, 1, "WROTE blockIdCompressMode %d using %d bytes\n", od->blockIdCompressMode, sizeof(unsigned char)); }
//}

//Write the length of blockIds metadata
//if(od->densityEncodedBufCount != 0){
while(od->fileBufLen + sizeof(unsigned int) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &od->blockIdBufLen, sizeof(unsigned int)); od->fileBufLen += sizeof(unsigned int);
if(LOG){ debug(od->logLevel, 1, "WROTE blockIdBufLen %d using %d bytes\n", od->blockIdBufLen, sizeof(unsigned int)); }
//}

//Write the blockIds metadata
//if(od->densityEncodedBufCount != 0){
while(od->fileBufLen + sizeof(unsigned char)*od->blockIdBufLen >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, od->blockIdBuf, sizeof(unsigned char)*od->blockIdBufLen); 
od->fileBufLen += sizeof(unsigned char)*od->blockIdBufLen;
if(LOG){ debug(od->logLevel, 1, "WROTE blockIdBuf %d bytes\n", sizeof(unsigned char)*od->blockIdBufLen); }
//}


/*if(FULL_GMP){
    //Write the length of partialBytesBuf as the number of bits
    while(od->fileBufLen + sizeof(unsigned short) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
    memcpy(od->fileBuf + od->fileBufLen, &od->partialBytesBufLen, sizeof(unsigned short)); od->fileBufLen += sizeof(unsigned short);
    if(LOG){ debug(od->logLevel, 1, "WROTE partialBytesBufLen %d using %d bytes\n", od->partialBytesBufLen, sizeof(unsigned short)); }

    //Write the partialBytes 
    while(od->fileBufLen + sizeof(unsigned char)*(od->partialBytesBufLen/8) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
    memcpy(od->fileBuf + od->fileBufLen, od->partialBytes, sizeof(unsigned char)*(int)ceil((float)od->partialBytesBufLen/(float)8)); 
    od->fileBufLen += sizeof(unsigned char)*(int)ceil((float)od->partialBytesBufLen/(float)8);
    if(LOG){ debug(od->logLevel, 1, "WROTE partialBytes %d bytes\n", sizeof(unsigned char)*(int)ceil((float)od->partialBytesBufLen/(float)8)); }
}
*/

//if(!FULL_GMP){
    //Write the number of gmp encoded values
    while(od->fileBufLen + sizeof(unsigned short) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
    memcpy(od->fileBuf + od->fileBufLen, &od->firstValDecLen, sizeof(unsigned short)); od->fileBufLen += sizeof(unsigned short);
    if(LOG){ debug(od->logLevel, 1, "WROTE the number of encoded gmp values %d using %d bytes\n", od->firstValDecLen, sizeof(unsigned short)); }

    //Write the firstValDec values
    while(od->fileBufLen + sizeof(unsigned char)*od->firstValDecLen >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
    memcpy(od->fileBuf + od->fileBufLen, od->firstValDec, sizeof(unsigned char)*(od->firstValDecLen)); od->fileBufLen += sizeof(unsigned char)*(od->firstValDecLen);
    if(LOG){ debug(od->logLevel, 1, "WROTE gmp firstValDec data using %d bytes\n", sizeof(unsigned char)*(od->firstValDecLen)); }

    //Write the lengths of encoded values
    while(od->fileBufLen + sizeof(unsigned int)*od->encValCount >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
    memcpy(od->fileBuf + od->fileBufLen, od->encValLen, sizeof(unsigned int)*(od->encValCount)); od->fileBufLen += sizeof(unsigned int)*(od->encValCount);
    if(LOG){ debug(od->logLevel, 1, "WROTE the lengths of encoded values using %d bytes\n", sizeof(unsigned int)*(od->encValCount)); }
//}

//Write the length of gmp encoded outBuf
while(od->fileBufLen + sizeof(unsigned int) >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
memcpy(od->fileBuf + od->fileBufLen, &od->outBufLen, sizeof(unsigned int)); od->fileBufLen += sizeof(unsigned int);
if(LOG){ debug(od->logLevel, 1, "WROTE length of gmp encoded outBuf %d using %d bytes\n", od->outBufLen, sizeof(unsigned int)); }

//printf("first Byte: %u\n", od->outBuf[0]);

//Write the gmp encoded outbuf
writeInBuf(&od->fileBuf, &od->fileBufLen, &od->fileBufAllocSize, od->outBuf, sizeof(unsigned char), od->outBufLen);
//while(od->fileBufLen + sizeof(unsigned char)*od->outBufLen >= od->fileBufAllocSize){ doubleUpChar(&(od->fileBuf), &(od->fileBufAllocSize), od->fileBufLen); }
//memcpy(od->fileBuf + od->fileBufLen, od->outBuf, sizeof(unsigned char)*(od->outBufLen)); od->fileBufLen += sizeof(unsigned char)*(od->outBufLen);
if(LOG){ debug(od->logLevel, 1, "WROTE gmp encoded outBuf using %d bytes\n", sizeof(unsigned char)*(od->outBufLen)); }


//Finally write the fileBuf into the file
//fwrite(od->fileBuf, sizeof(unsigned char), od->fileBufLen, od->of);
//if(LOG){ debug(HIGH, 1, "\n%d bytes of encoded data written to file\n", od->fileBufLen); }
//printf("%s file with %d bytes successfully compressed to %d bytes\n", od->filename, od->inBufLen, od->fileBufLen);

//if(TL){ printf("tG: %lld\n", od->tG); }
//displayGInstr();

//if(LOG){ debug(od->logLevel, 1, "\nWROTE inCharCount: %d, distinctCharCount: %d, ofDataLen: %d, fileBufLen: %d, outBufLen: %d, partialBytesBufLen in bits: %d, emptyBytesCount: %d, rCountBufLen: %d\n", od->inCharCount, distinctCharCount, od->ofDataLen, od->fileBufLen, od->outBufLen, od->partialBytesBufLen, od->emptyBytesCount, od->rCountBufLen*2); }
if(LOG){ debug(od->logLevel, 1, "\nWROTE inCharCount: %d, distinctCharCount: %d, ofDataLen: %d, fileBufLen: %d, outBufLen: %d, emptyBytesCount: %d, rCountBufLen: %d\n", od->inCharCount, distinctCharCount, od->ofDataLen, od->fileBufLen, od->outBufLen, od->emptyBytesCount, od->rCountBufLen*2); }

//Free the relative positions
if(LOG){ debug(od->logLevel, 0, "\n"); }
for(i = 0; i < MAX_CHARS; i++){
    if(rpCount[i] != 0 && rp[i] != NULL){ if(LOG){ debug(LOW, 1, "(encodeBuffer)Before Freeing rp[%d] ", i); } free(rp[i]); if(LOG){ debug(LOW, 1, "(encodeBuffer)After Freed rp[%d]\n", i); } }
}

//Cleanup all the allocations
encCleanup(od);
#ifdef DEBUG
if(LOG){ debug(HIGH, 0, "Everything cleaned up\n"); }
#endif
//printf("nTotal: %d, rTotal: %d\n", nTotal, rTotal);
}
