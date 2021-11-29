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

unsigned char* gBuf = NULL;
unsigned int gBufLen = 0;

void floatingDensity(unsigned char* inBuf, int filelen){
int i = 0;
int blkSize = 64, blkTypes = 10;
unsigned char* blocks[blkTypes]; unsigned int blockN[blkTypes], blockR[blkTypes], blockCount = 0;
for(i = 0; i < blkTypes; i++){
    blocks[i] = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(blocks[i], 0, sizeof(unsigned char)*filelen);
    blockN[i] = 0; blockR[i] = 0;
}
unsigned char* metaData = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(metaData, 0, sizeof(unsigned char)*filelen);
unsigned int metaDataLen = 0, misCount = 0;
unsigned char tempBlock[blkSize]; memset(tempBlock, 0, sizeof(tempBlock));
unsigned int tempBlockN = 0, tempBlockR = 0, tempBlockType = 0, prevTempBlockType = 0, firstBlockType = 0, prevFloatingBlockType = 0, curFloatingBlockType = 0;
float tempBlockDensity = 0, prevBlockDensity = 0;
for(i = 0; i < filelen; i++){
    tempBlock[tempBlockN++] = inBuf[i];
    if(inBuf[i] == 0){ tempBlockR++; }
    if((i+1) % blkSize == 0){ //Block ended
        blockCount++;
        tempBlockDensity = (float)(tempBlockR*100)/(float)tempBlockN;
        tempBlockType = tempBlockDensity/10; tempBlockType++; if(tempBlockType == 11){ tempBlockType--; }
        printf("Block(%d) ended with N: %d, R: %d, density: %.2f, tempBlockType: %d ", blockCount, tempBlockN, tempBlockR, tempBlockDensity, tempBlockType);
        if(blockCount > 1){
            if(tempBlockType > prevTempBlockType){
                metaData[metaDataLen++] = 1; //Rising
                curFloatingBlockType = prevTempBlockType+1;
                printf(" Rising, new block type: %d\n", curFloatingBlockType);
            } else {
                if(tempBlockType < prevTempBlockType){
                    metaData[metaDataLen++] = 2; //Falling
                    curFloatingBlockType = prevTempBlockType-1;
                    printf(" Falling, new block type: %d\n", curFloatingBlockType);
                } else { // tempBlockType == prevTempBlockType
                    metaData[metaDataLen++] = 0; //Stays the same
                    curFloatingBlockType = prevTempBlockType;
                    printf(" Same, new block type: %d\n", curFloatingBlockType);
                }
            }
            if(tempBlockType != curFloatingBlockType){ misCount++; }
        } else {
            firstBlockType = tempBlockType;
            memcpy(blocks[firstBlockType] + blockN[firstBlockType], tempBlock, sizeof(unsigned char)*tempBlockN);
            prevFloatingBlockType = firstBlockType;
        }


        tempBlockN = 0; tempBlockR = 0;
        memset(tempBlock, 0, sizeof(tempBlock));
        prevBlockDensity = tempBlockDensity;
        prevTempBlockType = tempBlockType;
    } else {
        if(i == filelen-1){ //Last partial block
            blockCount++;
            tempBlockDensity = (float)(tempBlockR*100)/(float)tempBlockN;
            printf("Last partial Block ended with N: %d, R: %d\n", blockCount, tempBlockN, tempBlockR, tempBlockDensity);
        }
    }
}
printf("misCount: %d\n", misCount);
if(metaData != NULL){ free(metaData); }
for(i = 0; i < blkTypes; i++){ if(blocks[i] != NULL){ free(blocks[i]); } }
}


void magicSplitZeroOne(unsigned char* inBuf, int filelen){
int i = 0, j = 0, curChar = -1, runningChar = -1, prevChar = -1, runStarted = TRUE;
unsigned char* splitBuf[3]; unsigned int splitBufLen[3];
for(i = 0; i < 3; i++){
    splitBuf[i] = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(splitBuf[i], 0, sizeof(unsigned char)*filelen);
    splitBufLen[i] = 0;
}
runningChar = prevChar = inBuf[0];
splitBuf[prevChar][splitBufLen[prevChar]++] = prevChar;
for(i = 1; i < filelen; i++){
    curChar = inBuf[i];
    if(curChar == prevChar){
        splitBuf[curChar][splitBufLen[curChar]++] = curChar;
        if(i == filelen-1){
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
            if(i == filelen-1){
                splitBuf[2][splitBufLen[2]++] = curChar;
                splitBufLen[curChar]--;
            }
        }
        
    }
    prevChar = curChar;
}

for(i = 0; i < 3; i++){
    printf("%d%d%d: ", i, i, i);
    int zeroCount = 0;
    for(j = 0; j < splitBufLen[i]; j++){ 
        printf("%d", splitBuf[i][j]); 
        if(splitBuf[i][j] == 0){ zeroCount++; }
    }
    printf("\n");
    printf("n: %d, zeroCount: %d\n", splitBufLen[i], zeroCount);
}


//Verify
unsigned char* verify = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(verify, 0, sizeof(unsigned char)*filelen);
unsigned int verifyLen = 0;
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
printf("verifyLen: %d, filelen: %d\n", verifyLen, filelen);
for(i = 0 ; i < verifyLen; i++){
    if(inBuf[i] != verify[i]){ printf("ERROR\n"); }
    //printf("orig: %d, verify: %d\n", inBuf[i], verify[i]);
}
if(verify != NULL){ free(verify); }

for(i = 0; i < 3; i++){ if(splitBuf[i] != NULL){ free(splitBuf[i]); } }

}


void rearrange(unsigned char* inBuf, int filelen){
int i = 0, j = 0, curChar = -1, prevChar = inBuf[0], runLength = 0;
int chkZero = 0, blkSize = 2048;
unsigned char* buf[blkSize]; int bufCount[blkSize], bufZeroCount[blkSize];
for(i = 0; i < blkSize; i++){
    buf[i] = NULL;
    bufCount[i] = 0;
    bufZeroCount[i] = 0;
}
if(prevChar == 1){ runLength++; }
for(i = 1; i < filelen; i++){
    curChar = inBuf[i];
    if(curChar == 0){
        if(curChar == prevChar){
            if(bufCount[0] == 0){
                buf[0] = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(buf[0], 0, sizeof(unsigned char)*filelen);
            }
            buf[0][bufCount[0]++] = 0; bufZeroCount[0]++;
            chkZero++;
        } else {
            printf("run of 1 ended with length %d\n", runLength);
            if(bufCount[runLength] == 0){
                buf[runLength] = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(buf[runLength], 0, sizeof(unsigned char)*filelen);
            }
            buf[runLength][bufCount[runLength]++] = 0; bufZeroCount[runLength]++;
            chkZero++;
            for(j = 0; j < runLength; j++){
                if(bufCount[j] == 0){
                    buf[j] = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(buf[j], 0, sizeof(unsigned char)*filelen);
                }
                buf[j][bufCount[j]++] = 1;
            }
            runLength = 0;
        }
    } else {
        runLength++;
    }
    prevChar = curChar;
}
printf("chkZero: %d\n", chkZero);
FILE* f1 = (FILE*)openFile(HIGH, "rearranged.txt", "wb");
int bitLen = 0, bitLenSum = 0;
for(i = 0; i < blkSize; i++){
    if(bufCount[i] != 0){
        mpz_t bV; mpz_init(bV); mpz_bin_uiui(bV, bufCount[i], bufZeroCount[i]); 
        bitLen = mpz_sizeinbase(bV, 2);
        mpz_clear(bV);
        bitLenSum += bitLen;
        printf("runLength: %d, bufCount[%d]: %d, zeroCount: %d, percent: %d, bitLen: %d, bitLenSum: %d\n", i, i, bufCount[i], bufZeroCount[i], bufZeroCount[i]*100/bufCount[i], bitLen, bitLenSum);
        for(j = 0; j < bufCount[i]; j++){
            fputc(buf[i][j], f1);
        }
    }
}
fclose(f1);
for(i = 0; i < blkSize; i++){ if(buf[i] != NULL){ free(buf[i]); } }
}


void superCodes(unsigned char* inBuf, int filelen){
int i = 0, j = 0, temp = 0, curChar = -1, found = FALSE, length = 0;
FILE* f1 = (FILE*)openFile(HIGH, "packedBits.txt", "wb");
FILE* f2 = (FILE*)openFile(HIGH, "metaBits.txt", "wb");
for(i = 0; i < filelen; i++){
    curChar = inBuf[i];
    temp = 128;
    printf("curChar: %d - ", curChar);
    found = FALSE;
    length = 0;
    for(j = 0; j < 8; j++){
        if(temp & curChar){
            printf("1");
            fputc(1, f1);
            found = TRUE;
            length++;
        } else {
            if(found == TRUE){
                printf("0");
                fputc(0, f1);
                length++;
            } 
        }
        temp >>= 1;
    }
    if(found == FALSE){
        printf("0");
        fputc(0, f1);
        length++;
    }
    printf(" - %d\n", length);
    fputc(length, f2);
}
fclose(f1);
fclose(f2);
}

void stepByStep(unsigned char* inBuf, int filelen){
int i = 0, curChar = -1, prevChar = -1;
unsigned char* fullBuf = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(fullBuf, 0,sizeof(unsigned char)*filelen);
int fullBufCount = 0;
FILE* f2 = (FILE*)openFile(HIGH, "step2.txt", "wb");
prevChar = inBuf[0];
for(i = 1; i < filelen; i++){
    curChar = inBuf[i];
    if(curChar == 0){
        if(prevChar != 0){
            fullBuf[fullBufCount++] = 1;
            fputc(1, f2);
        } else {
            fullBuf[fullBufCount++] = 0;
        }
    } else {
        if(prevChar != 0){
            fputc(0, f2);
        }
    }
    prevChar = curChar;
}
FILE* f1 = (FILE*)openFile(HIGH, "step1.txt", "wb"); fwrite(fullBuf, sizeof(unsigned char), fullBufCount, f1); fclose(f1);
fclose(f2);
if(fullBuf != NULL){ free(fullBuf); }
}


void removeNoise(unsigned char* inBuf, int filelen){
int i = 0, j = 0, curChar = -1, uniqueCount = 0, ct = 0;
int blkSize = 64;
FILE* f1 = (FILE*)openFile(HIGH, "withoutNoise.txt", "wb");
FILE* f2 = (FILE*)openFile(HIGH, "noise.txt", "wb");
unsigned char inChars[256], inData[blkSize]; memset(inChars, 0, sizeof(inChars)); memset(inData, 0, sizeof(inData));
for(i = 0; i < filelen; i++){
    curChar = inBuf[i];
    if((i+1) % blkSize == 0){ //Block ended
        inChars[curChar] = 1;
        inData[ct++] = curChar;
        uniqueCount = 0;
        //for(j = 0; j < 256; j++){ if(inChars[j] == 1){ uniqueCount++; } }
        for(j = 0; j < blkSize; j++){ if(inData[j] == 0){ uniqueCount++; } }
        if(uniqueCount > 56){
            printf("uniqueCount: %d\n", uniqueCount);
            for(j = 0; j < ct; j++){ fputc(inData[j], f2); }
        } else {
            for(j = 0; j < ct; j++){ fputc(inData[j], f1); }
        }
        memset(inChars, 0, sizeof(inChars));
        memset(inData, 0, sizeof(inData)); ct = 0;
    } else {
        inChars[curChar] = 1;
        inData[ct++] = curChar;
    }
}
fclose(f1);
fclose(f2);
}


void pluckZeros(unsigned char* inBuf, int filelen){
int i = 0, j = 0, inChar = 0, runThreshold = 1000, startPos = -1, endPos = -1, curPos = -1, prevPos = -1, prevChar = -1, curChar = -1, runLength = 0;
FILE* f1 = (FILE*)openFile(HIGH, "withoutZero.txt", "wb");
for(i = 0; i < filelen; i++){
    curChar = inBuf[i];
    if(curChar == inChar){
        if(prevChar == curChar){
        } else {
            startPos = i;
        }
    } else {
        if(prevChar == inChar){
            endPos = i-1;
            runLength = endPos - startPos + 1;
            if(runLength > runThreshold){
                printf("startPos: %d, endPos: %d, runLength: %d\n", startPos, endPos, runLength);
            } else {
                //printf("startPos: %d, endPos: %d, runLength: %d\n", startPos, endPos, runLength);
                for(j = 0; j < runLength; j++){ fputc(0, f1); }
            }
            fputc(curChar, f1);
        } else {
            fputc(curChar, f1);
        }
    }
    prevChar = curChar;
}
fclose(f1);
}


void trialDensity(unsigned char* inBuf, int filelen){
int i = 0, curChar = -1, nextChar = -1;
int c00 = 0, c11 = 0, c01 = 0, c10 = 0;
FILE* f1 = (FILE*)openFile(HIGH, "meta.txt", "wb");
for(i = 0; i < filelen; i++){
    curChar = inBuf[i++];
    nextChar = inBuf[i];
    if(curChar == 0 && nextChar == 0){ c00++; fputc(0, f1); }
    if(curChar == 1 && nextChar == 1){ c11++; fputc(1, f1); }
    if(curChar == 0 && nextChar == 1){ c01++; fputc(2, f1); }
    if(curChar == 1 && nextChar == 0){ c10++; fputc(2, f1); }
}
printf("c00: %d, c11: %d, c01: %d, c10: %d\n", c00, c11, c01, c10);
fclose(f1);
}


void switchBits(unsigned char* inBuf, int filelen){
int i = 0, zeroCount = 0, oneCount = 0, overallZeroCount = 0, expected = 0, newCount = 0, blockZeroCount = 0;
int exZero = 0, exOne = 0;
float curPercent = 0, overallPercent = 0, blockPercent = 0;
for(i = 0; i < filelen; i++){ if(inBuf[i] == 0){ overallZeroCount++; } }
overallPercent = (float)(overallZeroCount*100) / (float)filelen;
printf("overallPercent: %.3f\n", overallPercent);
for(i = 0; i < filelen; i++){
    if(inBuf[i] == expected){ newCount++; }
    if(inBuf[i] == 0){ zeroCount++; } else { oneCount++; }
    curPercent = (float)(zeroCount*100)/(float)(i+1);
    if(curPercent > overallPercent){ expected = 1; } else { expected = 0; }
    if(expected == 0 && inBuf[i] == 0){ exZero++; }
    if(expected == 1 && inBuf[i] == 0){ exOne++; }
    printf("(%d)curPercent: %.3f, dev: %.3f, expected %d, newCount: %d\n", inBuf[i], curPercent, overallPercent-curPercent, expected, newCount);

    if((i+1)%64 == 0){
        if(inBuf[i] == 0){ blockZeroCount++; }
        blockPercent = (float)(blockZeroCount*100)/(float)64;
        printf("Block Percent: %.3f\n", blockPercent);
        blockZeroCount = 0;
    } else {
        if(inBuf[i] == 0){ blockZeroCount++; }
    }

}
printf("zeroCount: %d, oneCount: %d, filelen: %d\n", zeroCount, oneCount, filelen);
printf("exZero: %d, exOne: %d\n", exZero, exOne);

}


void bitToByte(unsigned char* inBuf, int filelen){
int i = 0, j = 0, zeroByte = 0, curByte = 0;
FILE* f1 = (FILE*)openFile(HIGH, "bitToByte.txt", "wb");
for(i = 0; i < filelen; ){
    curByte = 0;
    for(j = 0; j < 8; j++){
        if(i < filelen){ curByte = curByte | inBuf[i++] << (7-j); }
    }
    printf("curByte: %d\n", curByte);
    fputc((unsigned char)curByte, f1);
    //if(inBuf[i++] == 0 && inBuf[i++] == 0 && inBuf[i++] == 0 && inBuf[i++] == 0 && inBuf[i++] == 0 && inBuf[i++] == 0 && inBuf[i++] == 0 && inBuf[i] == 0){
    //    zeroByte++;
    //}
}
printf("zeroByte: %d(%d)\n", zeroByte, filelen/8);
fclose(f1);
}


void checkMultiBits(unsigned char* inBuf, int filelen){
int i = 0, firstBit = 0, secondBit = 0, c00 = 0, c01 = 0, c10 = 0, c11 = 0;
int newlen = filelen/2, p0 = 0, p1 = 0, zeroCount = 0;
for(i = 0; i < filelen; i++){
    firstBit = inBuf[i++]; secondBit = inBuf[i];
    if(firstBit == 0 && secondBit == 0){ c00++; zeroCount += 2; }
    if(firstBit == 0 && secondBit == 1){ c01++; zeroCount++; }
    if(firstBit == 1 && secondBit == 0){ c10++; zeroCount++; }
    if(firstBit == 1 && secondBit == 1){ c11++; }
}
printf("zeroCount: %d\n", zeroCount);
p0 = zeroCount*100/filelen;
p1 = (filelen-zeroCount)*100/filelen;
printf("zeroPercent: %d, onePercent: %d\n", p0, p1);
printf("c00: %d(%d)(actual: %d)(expected: %d)(bitLen: %d)\n", c00, newlen, c00*100/newlen, p0*p0, nCrBitLen(newlen, c00));
printf("c01: %d(%d)(actual: %d)(expected: %d)(bitLen: %d)\n", c01, newlen, c01*100/newlen, p0*p1, nCrBitLen(newlen-c00, c01));
printf("c10: %d(%d)(actual: %d)(expected: %d)(bitLen: %d)\n", c10, newlen, c10*100/newlen, p1*p0, nCrBitLen(newlen-c00-c01, c10));
printf("c11: %d(%d)(actual: %d)(expected: %d)(bitLen: %d)\n", c11, newlen, c11*100/newlen, p1*p1, nCrBitLen(newlen-c00-c01-c10, c11));

}


void checkChunks(unsigned char* inBuf, int filelen){
int i = 0, j = 0, check = 0, zeroCount = 0, zeroCountPos = 0, chunkLen = 2, zeroCountPosSum = 0, emptyChunksCount = 0;
for(i = 0; i < filelen; i++){
    if((i+1) % chunkLen == 0){
        if(inBuf[i] == 0){ zeroCount++; }
        for(j = 0; j < chunkLen; j++){
            if(inBuf[i-j] == 0){ zeroCountPos = chunkLen - j; break; }
        }
        //if(zeroCountPos == 0){ zeroCountPos = 1; emptyChunksCount++; }
        if(zeroCountPos == 0){ zeroCountPos = chunkLen; emptyChunksCount++; }
        //else { zeroCountPos++; }
        printf("%d zeroCount: %d, zeroCountPos: %d\n", inBuf[i], zeroCount, zeroCountPos);
        zeroCountPosSum += zeroCountPos;
        zeroCount = 0; zeroCountPos = 0;
    } else {
        printf("%d", inBuf[i]);
        if(inBuf[i] == 0){ zeroCount++; }
    }
}
printf("zeroCountPosSum: %d\n", zeroCountPosSum);
printf("emptyChunksCount: %d\n", emptyChunksCount);
}


int checkChunksSkewdness(unsigned char* inBuf, int filelen, unsigned char inChar, int* lastViableChunkLength){
int i = 0, j = 0, runLength = 0, prevChar = inBuf[0], curChar = -1;
int chunksChar = inChar, chunksCharCount = 0;
int oneCount = 0;
int chunksCount[256]; for(i = 0; i < 256; i++){ chunksCount[i] = 0; }
if(prevChar == chunksChar){ runLength++; chunksCharCount++; oneCount++; }
for(i = 1; i < filelen; i++){
    curChar = inBuf[i];
    if(curChar == chunksChar){
        chunksCharCount++;
        oneCount++; 
        if(curChar == prevChar){
            runLength++;
            if(i == filelen-1){ 
                //printf("last run of %d ended with length %d(i: %d)\n ", chunksChar, runLength, i);
                if(runLength > 256){ printf("chunk length exceeds 256, Not Applicable. Exiting...\n"); return 0; }
                chunksCount[runLength]++; 
            }
        } else {
            runLength = 1;
        } 
    } else {
        if(prevChar == chunksChar){
            //printf("run of %d ended with length %d(i: %d)\n ", chunksChar, runLength, i);
            if(runLength > 256){ printf("chunk length exceeds 256, Not Applicable. Exiting...\n"); return 0; }
            chunksCount[runLength]++;
        }
    }
    prevChar = curChar;
}
float chunkCharPercent = (float)(chunksCharCount*100)/(float)filelen;
float chunkOtherCharPercent = (float)((filelen-chunksCharCount)*100)/(float)filelen;
int allChunksCount = 0, chunksSeen = 0, bitLenChunks = 0, bitLenRemaining = 0, tempZeroOne = 0, chunkCharCountInChunks = 0, bitLenChunksSum = 0, meta = 0;
float expectedPercent = 0;
int finalBitLen = 0, finalBitLenWithMeta = 0, finalBitLenWithMetaPrev = 0;
int finalViableChunkLen = 1, bestBitLenWithMeta = 0;
printf("%ds count: %d(%d): percentage: %.2f\n", 1-chunksChar, filelen - chunksCharCount, filelen, chunkOtherCharPercent);
printf("%ds count: %d(%d): percentage: %.2f\n", chunksChar, chunksCharCount, filelen, chunkCharPercent);
int overallBitLen =  approxBitLenNCR(filelen, chunksCharCount);
//printf("bits: %d\n", overallBitLen);
meta = (1+2+2)*8;
for(i = 0; i < 256; i++){ if(chunksCount[i] != 0){ allChunksCount += chunksCount[i]; } }
for(i = 0; i < 256; i++){
    if(chunksCount[i] != 0){ 
        expectedPercent = chunkOtherCharPercent;
        for(j = 0; j < i; j++){ expectedPercent = (expectedPercent * chunkCharPercent) / 100; }
        bitLenChunks = approxBitLenNCR(allChunksCount-chunksSeen, chunksCount[i]);
        tempZeroOne += allChunksCount - chunksSeen;
        chunkCharCountInChunks += allChunksCount-chunksSeen-chunksCount[i];
        bitLenRemaining = approxBitLenNCR(filelen-tempZeroOne, chunksCharCount - chunkCharCountInChunks);
        bitLenChunksSum += bitLenChunks;
        finalBitLen = bitLenRemaining + bitLenChunksSum;
        finalBitLenWithMeta = bitLenRemaining + bitLenChunksSum + meta;
        printf("chunksCount[%d]: %5d, count as per percentage: %5d, zero-one: %5d(%ds: %5d, %ds: %5d, per: %.2f)(bits: %5d) - (n: %5d, r:%5d)(bits: %5d) = (%5d), meta: %d = %d\n", 
              i, 
              chunksCount[i], 
              (int)(filelen*expectedPercent)/100, //count as per percentage
              allChunksCount - chunksSeen, //zero-one
              1-chunksChar, //zeros or ones
              chunksCount[i], //zeros
              chunksChar, //zeros or ones
              allChunksCount-chunksSeen-chunksCount[i], //ones
              (float)(chunksCount[i]*100)/(float)(allChunksCount-chunksSeen), //zero-one percentage
              bitLenChunks, //bit len
              filelen-tempZeroOne, //remaining N after removing chunks
              chunksCharCount - chunkCharCountInChunks, //remaining R after removing seperated ones
              bitLenRemaining, //bit len
              finalBitLen, 
              meta, 
              finalBitLenWithMeta);  
        chunksSeen += chunksCount[i];
        meta += (1+2)*8;
        if(finalBitLenWithMetaPrev != 0 && finalBitLenWithMeta > finalBitLenWithMetaPrev){
            printf("No longer viable...\n");
            break;
        } else { 
            bestBitLenWithMeta = finalBitLenWithMeta;
            finalViableChunkLen = i; 
        }
        finalBitLenWithMetaPrev = finalBitLenWithMeta;
    }
}
printf("total new chars: %d\n", allChunksCount);
//printf("finalViableChunkLen: %d\n", finalViableChunkLen);
//printf("bestBitLenWithMeta: %d\n", bestBitLenWithMeta);
//printf("overallBitLen: %d\n", overallBitLen);
*lastViableChunkLength = finalViableChunkLen;
int margin = 100;
if(overallBitLen > bestBitLenWithMeta + margin){
    printf("Gain of %d\n", overallBitLen - bestBitLenWithMeta);
    return (overallBitLen - bestBitLenWithMeta); //Gain
} else {
    return 0; //No Gain
}
}


void wrapperCheckOneChunks(int* rp, int n, int r){
unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char)*n); memset(temp, 1, sizeof(unsigned char)*n);
int i = 0;
for(i = 0; i < r; i++){
    temp[rp[i]-1] = 0;
}
int lastViableChunkLength = 0;
printf("ZERO CHUNKS\n");
int zeroChunkGain = checkChunksSkewdness(temp, n, 0, &lastViableChunkLength);
printf("ONE CHUNKS\n");
int oneChunkGain = checkChunksSkewdness(temp, n, 1, &lastViableChunkLength);
if(oneChunkGain > 0 && zeroChunkGain > 0){
if(oneChunkGain >= zeroChunkGain){
    printf("GAIN from one chunks: %d\n", oneChunkGain);
} else {
    printf("GAIN from zero chunks: %d\n", zeroChunkGain);
}
} else 
if(oneChunkGain > 0 && zeroChunkGain == 0){
    printf("GAIN from one chunks: %d\n", oneChunkGain);
} else
if(oneChunkGain == 0 && zeroChunkGain > 0){
    printf("GAIN from zero chunks: %d\n", zeroChunkGain);
}
if(temp != NULL){ free(temp); }
}

void checkOnes(unsigned char* inBuf, int filelen){
int i = 0, j = 0, runLength = 0, prevChar = inBuf[0], curChar = -1, singleZeroCount = 0, tempCount = 0;
int oneCount = 0;
int oneChunks[256]; for(i = 0; i < 256; i++){ oneChunks[i] = 0; }
if(prevChar == 1){ runLength++; oneCount++; }
for(i = 1; i < filelen; i++){
    curChar = inBuf[i];
    if(curChar == 1){
        oneCount++; 
        if(curChar == prevChar){
            runLength++;
            if(i == filelen-1){ 
                printf("last run of 1 ended with length %d(i: %d)\n ", runLength, i);
                oneChunks[runLength]++; if(runLength > 256){ printf("TAKE CARE\n"); }
            }
        } else {
            //printf("run of 0 ended with length %d(i: %d) ", runLength, i);
            runLength = 1;
        } 
    } else {
        if(prevChar == 1){
            printf("run of 1 ended with length %d(i: %d)\n ", runLength, i);
            oneChunks[runLength]++; if(runLength > 256){ printf("TAKE CARE\n"); }
            if(runLength == 1){ singleZeroCount++; tempCount--; }
        } else {
        }
    }
    prevChar = curChar;
}
int p1 = oneCount*100/filelen, tempSum = 0;
int p0 = (filelen-oneCount)*100/filelen;
int tempSeen = 0, bitLen1 = 0, bitLen2 = 0, tempZeroOne = 0, tempZeros = 0, tempOnes = 0, tempBitLen = 0, meta = 0;
float tempPer = 0;
printf("zero count: %d(%d): percentage: %d\n", filelen - oneCount, filelen, p0);
printf("one count: %d(%d): percentage: %d\n", oneCount, filelen, p1);
printf("bits: %d\n", nCrBitLen(filelen, oneCount));
meta = (1+2+2)*8;
for(i = 0; i < 256; i++){ if(oneChunks[i] != 0){ tempSum += oneChunks[i]; } }
for(i = 0; i < 256; i++){
    if(oneChunks[i] != 0){ 
        tempPer = p0;
        for(j = 0; j < i; j++){ tempPer = (tempPer * p1) / 100; }
        //printf("oneChunks[%d]: %d, percentage: %.2f, count as per percentage: %d, actual percentage: %.2f\n", i, oneChunks[i], tempPer, (int)(filelen*tempPer)/100, (float)((float)oneChunks[i]*100/filelen));  
        bitLen1 = nCrBitLen(tempSum-tempSeen, oneChunks[i]);
        tempZeroOne += tempSum - tempSeen;
        tempOnes += oneChunks[i];
        tempZeros += tempSum-tempSeen-oneChunks[i];
        bitLen2 = nCrBitLen(filelen-tempZeroOne, oneCount - tempZeros);
        tempBitLen += bitLen1;
        printf("oneChunks[%d]: %5d, count as per percentage: %5d, zero-one: %5d(zeros: %5d, ones: %5d, per: %2d)(bits: %5d) - (n: %5d, r:%5d)(bits: %5d) = (%5d), meta: %d = %d\n", i, oneChunks[i], (int)(filelen*tempPer)/100, tempSum - tempSeen, oneChunks[i], tempSum-tempSeen-oneChunks[i], oneChunks[i]*100/(tempSum-tempSeen), bitLen1, filelen-tempZeroOne, oneCount - tempZeros, bitLen2, bitLen2+tempBitLen, meta, bitLen2+tempBitLen+meta);  
        tempSeen += oneChunks[i];
        //tempSum += oneChunks[i];
        meta += (1+2)*8;
    }
}
printf("total new chars: %d\n", tempSum);
}

void checkZeros(unsigned char* inBuf, int filelen){
int i = 0, j = 0, runLength = 0, prevChar = inBuf[0], curChar = -1, singleZeroCount = 0, tempCount = 0;
int zeroCount = 0;
int zeroChunks[256]; for(i = 0; i < 256; i++){ zeroChunks[i] = 0; }
//unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(temp, 0, sizeof(unsigned char)*filelen);
if(prevChar == 0){ runLength++; zeroCount++; }
for(i = 1; i < filelen; i++){
    curChar = inBuf[i];
    //temp[i] = inBuf[i];
    if(curChar == 0){
        zeroCount++; 
        if(curChar == prevChar){
            runLength++;
            if(i == filelen-1){ 
                printf("last run of 0 ended with length %d(i: %d)\n ", runLength, i);
                if(runLength > 256){ printf("TAKE CARE\n"); } zeroChunks[runLength]++;
            }
            //temp[tempCount++] = 0;
        } else {
            //printf("run of 0 ended with length %d(i: %d) ", runLength, i);
            runLength = 1;
            //temp[tempCount++] = 0;
        } 
    } else {
        if(prevChar == 0){
            printf("run of 0 ended with length %d(i: %d)\n ", runLength, i);
            if(runLength > 256){ printf("TAKE CARE\n"); } zeroChunks[runLength]++;
            if(runLength == 1){ singleZeroCount++; tempCount--; }
            //temp[tempCount++] = 1;
        } else {
            //temp[tempCount++] = 1;
        }
    }
    prevChar = curChar;
}
printf("\nsingleZeroCount: %d(%d)\n", singleZeroCount, filelen);
printf("\ntempCount: %d\n", tempCount);
int p0 = zeroCount*100/filelen, tempSum = 0, tempSeen = 0, bitLen1 = 0, bitLen2 = 0, tempZeroOne = 0, tempZeros = 0, tempOnes = 0, tempBitLen = 0;
int p1 = (filelen-zeroCount)*100/filelen;
float tempPer = 0;
printf("zero count: %d(%d): percentage: %d\n", zeroCount, filelen, p0);
printf("one count: %d(%d): percentage: %d\n", filelen - zeroCount, filelen, p1);
printf("bits: %d\n", nCrBitLen(filelen, zeroCount));
for(i = 0; i < 256; i++){ if(zeroChunks[i] != 0){ tempSum += zeroChunks[i]; } }
for(i = 0; i < 256; i++){
    if(zeroChunks[i] != 0){ 
        tempPer = p1;
        for(j = 0; j < i; j++){ tempPer = (tempPer * p0) / 100; }
        //printf("zeroChunks[%d]: %d, percentage: %.2f, count as per percentage: %d, actual percentage: %.2f\n", i, zeroChunks[i], tempPer, (int)(filelen*tempPer)/100, (float)((float)zeroChunks[i]*100/filelen));  
        bitLen1 = nCrBitLen(tempSum-tempSeen, zeroChunks[i]);
        tempZeroOne += tempSum - tempSeen;
        tempZeros += zeroChunks[i];
        tempOnes += tempSum-tempSeen-zeroChunks[i];
        bitLen2 = nCrBitLen(filelen-tempZeroOne, zeroCount - tempOnes);
        tempBitLen += bitLen1;
        printf("zeroChunks[%d]: %5d, count as per percentage: %5d, zero-one: %5d(zeros: %5d, ones: %5d, per: %2d)(bits: %5d) - (n: %5d, r:%5d)(bits: %5d) = (%5d)\n", i, zeroChunks[i], (int)(filelen*tempPer)/100, tempSum - tempSeen, zeroChunks[i], tempSum-tempSeen-zeroChunks[i], zeroChunks[i]*100/(tempSum-tempSeen), bitLen1, filelen-tempZeroOne, zeroCount - tempOnes, bitLen2, bitLen2+tempBitLen);  
        //tempSum += zeroChunks[i];
        tempSeen += zeroChunks[i];
    }
}
printf("total new chars: %d\n", tempSum);
//FILE* f1 = (FILE*)openFile(HIGH, "newOne.txt", "wb"); fwrite(temp, sizeof(unsigned char), tempCount, f1); fclose(f1);
//if(temp != NULL){ free(temp); }
}


void checkGaps(unsigned char* inBuf, int filelen){
int i = 0, j = 0, runLength = 0, prevChar = inBuf[0], curChar = -1, oneCount = 0, newOneCount = 0;
unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(temp, 0, sizeof(unsigned char)*filelen);
if(prevChar = 1){ runLength++; }
for(i = 1; i < filelen; i++){
    curChar = inBuf[i];
    if(curChar == 1){
        if(curChar == prevChar){
            runLength++;
            if(i == filelen-1){ oneCount += runLength; printf("last run of 1 ended with length %d(%d)(i: %d) ", runLength, oneCount, i); }
        } else {
            //printf("run of 1 ended with length %d(%d)\n", runLength, oneCount);
            //oneCount += runLength;
            runLength = 1;
        }
    } else {
        if(prevChar == 1){
            oneCount += runLength;
            printf("run of 1 ended with length %d(%d)(i: %d) ", runLength, oneCount, i);
            if(runLength == 1){ newOneCount += 2; printf(" newOneCount: %d\n", newOneCount); }
            if(runLength == 2){ newOneCount += 2; printf(" newOneCount: %d\n", newOneCount); }
            if(runLength > 2){ newOneCount += 2; printf(" newOneCount: %d\n", newOneCount); }
            temp[i] = 1;
            temp[i - runLength] = 1;
        }
            //runLength = 1;
    }
    prevChar = curChar;
}
printf("\noneCount: %d, newOneCount: %d\n", oneCount, newOneCount);
FILE* f1 = (FILE*)openFile(HIGH, "newOne.txt", "wb"); fwrite(temp, sizeof(unsigned char), filelen, f1); fclose(f1);
if(temp != NULL){ free(temp); }
}


void analyseGaps(unsigned char* inBuf, int filelen){
int i = 0, j = 0, k = 0, prevPos = -1, curPos = -1, noOne = 0;
FILE* f1 = (FILE*)openFile(HIGH, "gap.txt", "wb");
FILE* f2 = (FILE*)openFile(HIGH, "new.txt", "wb");
for(i = 0; i < filelen; i++){
    if(inBuf[i] == 0){ 
        curPos = i; 
        if(prevPos != -1){
            printf("curPos: %d, prevPos: %d ", curPos, prevPos);
            if(curPos != prevPos + 1){
                printf("GAP(%d) ", curPos - prevPos - 1);
                for(j = prevPos + 1; j < curPos; j++){ printf(" %d", inBuf[j]); } 
                for(j = prevPos + 1; j < curPos; j++){ if(inBuf[j] == 1){ printf(" ONE "); break; } }
                if(j == curPos){ noOne += curPos-prevPos-1; printf(" noOne: %d ", noOne); fputc(1, f1); }
                else { fputc(0, f1); for(k = prevPos + 1; k < curPos; k++){ fputc(inBuf[k], f2);  } }
            } else {
            }
            printf("\n");
        }
        prevPos = curPos;
    }
    
}
printf("noOne: %d\n", noOne);
fclose(f1);
fclose(f2);
}


void predict(unsigned char* inBuf, int filelen){
int i = 0;
unsigned char* outBuf1 = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(outBuf1, 0, sizeof(unsigned char)*filelen);
unsigned char* outBuf2 = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(outBuf2, 0, sizeof(unsigned char)*filelen);
int outBuf1Len = 0, outBuf2Len = 0, prevChar = -1;
//prevChar = inBuf[0]+inBuf[1];
prevChar = inBuf[0];
for(i = 2; i < filelen; i++){
    //printf("%d - %d - %d - %d = %d = curChar = %d ", inBuf[i-1], inBuf[i-2], inBuf[i-3], inBuf[i-4], prevChar/4, inBuf[i]);
    //if(prevChar/2 > 0){
    if(prevChar == 1){
        //if(inBuf[i-1] == 0 || inBuf[i-2] == 0){
        //    outBuf1[outBuf1Len++] = inBuf[i]; //printf(" Moved\n");
        //} else {
            outBuf2[outBuf2Len++] = inBuf[i]; //printf(" Moved\n");
        //}
    } else {
        outBuf1[outBuf1Len++] = inBuf[i]; //printf(" Not \n");
    }
    //prevChar = inBuf[i]+inBuf[i-1];
    prevChar = inBuf[i];
}

FILE* of1 = (FILE*)openFile(HIGH, "of1.txt", "wb"); fwrite(outBuf1, sizeof(unsigned char), outBuf1Len, of1); fclose(of1);
FILE* of2 = (FILE*)openFile(HIGH, "of2.txt", "wb"); fwrite(outBuf2, sizeof(unsigned char), outBuf2Len, of2); fclose(of2);

if(outBuf1 != NULL){ free(outBuf1); }
if(outBuf2 != NULL){ free(outBuf2); }
}


/* Function to compare two rotated strings based on the index in the original buffer */

int compareBwtIndices(const void* left, const void* right){
//printf("(compareBwtIndices)gBufLen: %d", gBufLen);
int leftIndex = *((int*)(left));
int rightIndex = *((int*)(right));
//printf("leftIndex: %d, rightIndex: %d\n", leftIndex, rightIndex);
int i = 0, j = 0, cmpCount = 0;
for(cmpCount = 0; cmpCount < gBufLen; cmpCount++){
	if(gBuf[leftIndex] == gBuf[rightIndex]){
		if(leftIndex != gBufLen - 1){leftIndex++;}
		else {leftIndex = 0;}
		if(rightIndex != gBufLen - 1){rightIndex++;}
		else {rightIndex = 0;}
	} else
	{
		if(gBuf[leftIndex] < gBuf[rightIndex]){
			return -1;
		} else 
		{
			return 1;
		}
	}
}
return 0;
}

/* Function to find bwt */

void bwTransform(unsigned char* inBuf, int len){
printf("(bwTransform)len: %d\n", len);
gBuf = inBuf; gBufLen = len;
//int i = 0, k = 0, indices[len], indicesCount = 0;
int i = 0, k = 0, indicesCount = 0;
int* indices = (int*)malloc(sizeof(int)*len);
memset(indices, -1, sizeof(int)*len);
/* Dumping the inBuf as readable values */
/*printf("Readable Text\n");
for(i = 0; i < len; i++){
	printf("%d", inBuf[i]);
}
printf("\n");
*/
/* For normal bwt */
for(i = 0; i < len; i++){
	indices[i] = i;
}
indicesCount = len;
/*
printf("Indices Before Sorting\n");
for(i = 0; i < indicesCount; i++){
	printf("indices[%d]: %d\n", i, indices[i]);
}
*/
qsort(indices, indicesCount, sizeof(int), compareBwtIndices);
FILE* fp = (FILE*)openFile(HIGH, "b1.txt", "wb");
/*
printf("Indices After Sorting\n");
for(i = 0; i < indicesCount; i++){
	printf("indices[%d]: %d\n", i, indices[i]);
}
*/
// Last column values
unsigned char chValue = 0, prevChValue = 0;
//printf("Last column values(indicesCount: %d)\n", indicesCount);
for(i = 0; i < indicesCount; i++){
	if(indices[i] - 1 >=0){
		//printf("%d ", inBuf[indices[i] - 1]);
		chValue = inBuf[indices[i] - 1];
		//printf(" chValue: %d\n", chValue);
		fputc(chValue, fp);
	} else 
	{
		//printf("%d\n", inBuf[len - 1]);
		fputc(inBuf[len - 1], fp);
	}
}
fclose(fp);
/*for(i = 0; i < len; i++){
	displayByIndex(inBuf, len, indices[i]);
}*/

}



int main(int argc, char** argv){

char filename[1024];
sprintf(filename, "%s", argv[2]);
FILE* inFile = (FILE*)openFile(HIGH, filename, "rb");
int filelen = fileLen(HIGH, inFile); 
unsigned char* inBuf = (unsigned char*)malloc(sizeof(unsigned char)*filelen); memset(inBuf, 0, sizeof(unsigned char)*filelen);
int charsRead = fread(inBuf, sizeof(unsigned char), filelen, inFile);

if(charsRead != filelen){ printf("File cannot be read properly. Exiting...\n"); } 
else { printf("%d chars read successfully.\n", charsRead); }

//bitToByte(inBuf, filelen);
//checkMultiBits(inBuf, filelen);
//predict(inBuf, filelen);
//analyseGaps(inBuf, filelen);
//checkGaps(inBuf, filelen);
//checkZeros(inBuf, filelen);
//checkOnes(inBuf, filelen);
int lastViableChunkLength = 0;
//checkChunksSkewdness(inBuf, filelen, 1, &lastViableChunkLength);
//checkChunks(inBuf, filelen);
//switchBits(inBuf, filelen);
//trialDensity(inBuf, filelen);
//pluckZeros(inBuf, filelen);
//removeNoise(inBuf, filelen);
//stepByStep(inBuf, filelen);
//superCodes(inBuf, filelen);
//rearrange(inBuf, filelen);
//magicSplitZeroOne(inBuf, filelen);
//bwTransform(inBuf, filelen);
floatingDensity(inBuf, filelen);

if(inBuf != NULL){ free(inBuf); }
printf("All Freed successfully.\n");
return 1;
}

