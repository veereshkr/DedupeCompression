#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include "gmp.h"
#include "misc.h"
#include "encodeOps.h"
#include "decodeOps.h"
//#include "fastDiv.h"

#define UNSETTLE_ZEROS 6
int lastSettled = FALSE;
uint64_t gInstr = 0;
int overWrite = FALSE;
int prevO = FALSE;

void displayGInstr(){ if(TL){ printf("gInstr: %d\n", gInstr); } }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to display gmp bits */
/* Input: */
/* Output: */
void displayGmpBits(mpz_t bV){
int l = 0, bVSize = mpz_sizeinbase(bV, 2); 
if(LOG){ debug(LOW, 0, "BITS: "); }
for(l = bVSize-1; l >= 0; l--){ 
    if(l == MAX_BIT-1){ if(LOG){ debug(LOW, 0, "# "); } }
    if(mpz_tstbit(bV, l)){ 
        if(LOG){ debug(LOW, 0, "1 "); }
    } else { 
        if(LOG){ debug(LOW, 0, "0 "); }
    } 
} 
if(LOG){ debug(LOW, 0, "\n"); }
}


void verify(mpz_t bigSum, mpz_t settled, mpz_t check, int carry){
mpz_t temp; mpz_init(temp); mpz_set(temp, settled);
mpz_mul_2exp(temp, temp, mpz_sizeinbase(check, 2));
mpz_add(temp, temp, check);
if(LOG){ debug(LOW, 2, "lastSettled: %d, carry: %d\n", lastSettled, carry); }
if(mpz_cmp_ui(settled, 0) != 0){ 
    if(carry == FALSE){
        mpz_clrbit(temp, mpz_sizeinbase(check, 2)-1); 
    }
}
//if(mpz_cmp_ui(settled, 0) != 0 && lastSettled == TRUE && splFlag == TRUE){ mpz_clrbit(temp, mpz_sizeinbase(check, 2)-1); }
//if(mpz_cmp_ui(settled, 0) != 0 && splFlag == FALSE){ mpz_clrbit(temp, mpz_sizeinbase(check, 2)-1); }
if(LOG){ debug(LOW, 0, "check(%d):   ", mpz_sizeinbase(check, 2)); }
//if(mpz_sizeinbase(check, 2) == 37){ displayGmpBits(check); }
if(LOG){ debug(LOW, 0, "settled(%d): ", mpz_sizeinbase(settled, 2)); }
//if(mpz_sizeinbase(settled, 2) == 150049){ displayGmpBits(settled); }
if(LOG){ debug(LOW, 0, "temp(%d):    ", mpz_sizeinbase(temp, 2)); }
//if(mpz_sizeinbase(temp, 2) == 150086){ displayGmpBits(temp); }
if(LOG){ debug(LOW, 0, "bigSum(%d):  ", mpz_sizeinbase(bigSum,2)); }
//if(mpz_sizeinbase(bigSum, 2) == 150086){ displayGmpBits(bigSum); }
//if(mpz_cmp(bigSum, temp) != 0){ if(LOG){ debug(LOW, 0, "verify failure...\n"); } }
if(mpz_cmp(bigSum, temp) != 0){ printf("verify failure...\n"); }
mpz_clear(temp);

}


void settleBits(mpz_t original, mpz_t settled, int carry, int* carrySettled){
int origLen = mpz_sizeinbase(original, 2);
mpz_t new; mpz_init(new); mpz_set(new, original);
if(LOG){ debug(LOW, 1, "carry is %d, carrySettled is %d\n", carry, *carrySettled); }
if(LOG){ debug(LOW, 1, "original len: %d\n", origLen); }
int i = 0, zeroCountAfter32 = 0, settleStartPos = 0;
int curBit = 0;
displayGmpBits(original);
if(carry == TRUE){ if(LOG){ debug(LOW, 0, "Carry propogated\n"); } }
for(i = 32; i < origLen-1; i++){
    if(!mpz_tstbit(original, i)){ zeroCountAfter32++; }
    if(settleStartPos == 0 && zeroCountAfter32 == UNSETTLE_ZEROS){ settleStartPos = i+1; }
}
if(mpz_cmp_ui(settled, 0) != 0 && carry == FALSE){
    zeroCountAfter32++;
    if(settleStartPos == 0 && zeroCountAfter32 == UNSETTLE_ZEROS){ settleStartPos = i+1; }
}
if(LOG){ debug(LOW, 1, "zeroCountAfter32: %d, settleStartPos: %d\n", zeroCountAfter32, settleStartPos); }
if(zeroCountAfter32 > UNSETTLE_ZEROS){
    if(LOG){ debug(LOW, 1, "Bits eligible for settlement (%d): ", origLen - settleStartPos); }
    curBit = mpz_tstbit(original, origLen-1);
    if(mpz_cmp_ui(settled, 0) != 0 && curBit == 1 && carry == FALSE){
        if(LOG){ debug(LOW, 1, "0 "); }
        mpz_mul_2exp(settled, settled, 1);
        mpz_clrbit(new, origLen-1);
    }
    if(mpz_cmp_ui(settled, 0) != 0 && curBit == 1 && carry == TRUE){
        if(LOG){ debug(LOW, 1, "1 "); }
        mpz_mul_2exp(settled, settled, 1);
        mpz_setbit(settled, 0);
        mpz_clrbit(new, origLen-1);
    }
    if(mpz_cmp_ui(settled, 0) == 0 && curBit == 1){
        if(LOG){ debug(LOW, 1, "1 "); }
        mpz_mul_2exp(settled, settled, 1);
        mpz_setbit(settled, 0);
        mpz_clrbit(new, origLen-1);
    }
    for(i = origLen-2; i >= settleStartPos; i--){
        curBit = mpz_tstbit(original, i);
        if(LOG){ debug(LOW, 1, "%d ", curBit); }
        mpz_mul_2exp(settled, settled, 1);
        if(curBit == 1){
            mpz_setbit(settled, 0);
        }
        mpz_clrbit(new, i);
    }
    mpz_setbit(new, settleStartPos-1);
    if(LOG){ debug(LOW, 1, "\n"); }
    if(LOG){ debug(LOW, 0, "Yes settlement...\n"); }
    lastSettled = TRUE;
    *carrySettled = TRUE;
} else { 
    if(LOG){ debug(LOW, 0, "No settlement...\n"); }
    lastSettled = FALSE;
    if(carry == TRUE){ *carrySettled = FALSE; }

}
if(LOG){ debug(LOW, 1, "settled len is %d\n", mpz_sizeinbase(settled, 2)); }
displayGmpBits(new);
//displayGmpBits(settled);

mpz_set(original, new);
mpz_clear(new);

}


/*
void settleBitsOld(mpz_t original, mpz_t settled){
int origLen = mpz_sizeinbase(original, 2);
mpz_t new; mpz_init(new); mpz_set(new, original);
if(LOG){ debug(LOW, 1, "original len: %d\n", origLen); }
int i = 0, zeroCountAfter32 = 0, pos3rdZero = 0, settleStartPos = 0;
int oneBit = 0;
for(i = 32; i < origLen-1; i++){
    oneBit = mpz_tstbit(original, i);
    if(!oneBit){ zeroCountAfter32++; }
    if(pos3rdZero == 0 && zeroCountAfter32 == UNSETTLE_ZEROS){ pos3rdZero = i; }
    if(pos3rdZero != 0 && oneBit == 1 && settleStartPos == 0){ settleStartPos = i; }
}
if(mpz_tstbit(original, origLen-1) == 1 && splFlag == TRUE){
    //1 is actually a 0
    zeroCountAfter32++;
    if(pos3rdZero == 0 && zeroCountAfter32 == UNSETTLE_ZEROS){ pos3rdZero = origLen-1; }
}
if(LOG){ debug(LOW, 1, "zeroCountAfter32: %d\n", zeroCountAfter32); }
if(zeroCountAfter32 > UNSETTLE_ZEROS){
    if(LOG){ debug(LOW, 1, "%d left most zero bits can be settled, pos3rdZero: %d, settleStartPos: %d\n", zeroCountAfter32-3, pos3rdZero, settleStartPos); }
}
settleStartPos = pos3rdZero;
if(zeroCountAfter32 > UNSETTLE_ZEROS && pos3rdZero > 31){
    if(LOG){ debug(LOW, 1, "Settled bits (%d): ", (origLen-1)-(pos3rdZero)); }
    if(mpz_tstbit(original, origLen-1) == 1 && splFlag == TRUE){
            if(LOG){ debug(LOW, 0, "0 "); } 
            mpz_mul_2exp(settled, settled, 1);
    }
    if(mpz_tstbit(original, origLen-1) == 1 && splFlag == FALSE){
            if(LOG){ debug(LOW, 0, "1 "); } 
            mpz_mul_2exp(settled, settled, 1);
            mpz_setbit(settled, 0);
    }
    mpz_clrbit(new, origLen-1);
    for(i = origLen-2; i > pos3rdZero; i--){
        if(mpz_tstbit(original, i)){ 
            if(LOG){ debug(LOW, 0, "1 "); } 
            mpz_mul_2exp(settled, settled, 1);
            mpz_setbit(settled, 0);
        } else { 
            if(LOG){ debug(LOW, 0, "0 "); } 
            mpz_mul_2exp(settled, settled, 1);
        }
        mpz_clrbit(new, i);
    }
    mpz_setbit(new, pos3rdZero);
    if(LOG){ debug(LOW, 0, "\n"); }
    displayGmpBits(original);
    displayGmpBits(new);
    if(LOG){ debug(LOW, 0, "new bit len: %d\n", mpz_sizeinbase(new, 2)); }
    splFlag = TRUE;
    if(LOG){ debug(LOW, 0, "Yes settlement\n"); }
} else {
    if(LOG){ debug(LOW, 0, "No settlement\n"); }
    splFlag = FALSE;
}
//gmp_printf("original(%Zd) trimmed to new(%Zd), settled(%Zd)\n", original, new, settled);
gmp_printf("original(%Zd) trimmed to new(%Zd), settled len is %d, trimmed len is %d\n", original, new, mpz_sizeinbase(settled, 2), mpz_sizeinbase(new, 2));
if(mpz_sizeinbase(original, 2) > 60){ printf("WARNING: original more than 60 bits long\n"); }
if(mpz_sizeinbase(new, 2) > 60){ printf("WARNING: new more than 60 bits long\n"); }
mpz_set(original, new);
mpz_clear(new);

}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to get bit len */
/* Input: */
/* Output: */
int gtBitLen(uint32_t n){
    int i = 0, result = 0; 
    //mpz_t bV; mpz_init(bV); mpz_set_ui(bV, n); result = mpz_sizeinbase(bV, 2); mpz_clear(bV); return result;
    if(n != 0){
        while(((n>>(32-(result+1))) & 1) == 0){
            result++;
        }
        result = 32-result;
    } else { result = 1; }
    return result;
     
}

int gtBitLen64(uint64_t n){
//printf("uint64_t: %lld\n", n);
int i = 0, result= 0;
uint64_t max = 9223372036854775808LL;
for(i = 0; i < 64; i++){
    if(n & max){
        //printf("ONE ");
        result = 64-i; break;
    } else {
        //printf("ZERO ");
    }
    n <<= 1;
}
//printf("result: %d\n", result);
return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to calculate binomial value by multiplying and dividing a previous value */
/* Input: */
/* Output: */
uint32_t superNCR_noninline(uint32_t prevNCR, int prevN, int prevR, int curN, int curR){
if(LOG){ debug(HIGH, 4, "(prevN, prevR)(%d, %d) (curN, curR)(%d, %d)\n", prevN, prevR, curN, curR); }
uint64_t result;
if(curN == prevN - 1 && curR == prevR){
    result = ((uint64_t)prevNCR * (uint64_t)(prevN - prevR)) / (uint64_t)prevN;
    if(LOG){ debug(MEDIUM, 2, "(%d/%d)\n", prevN-prevR, prevN); }
}
if(curN == prevN - 1 && curR == prevR - 1){
    result = ((uint64_t)prevNCR * (uint64_t)prevR) / (uint64_t)prevN;
    if(LOG){ debug(MEDIUM, 2, "(%d/%d)\n", prevR, prevN); }
}
gInstr++;
return (uint32_t)result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to calculate binomial value by multiplying and dividing a previous value */
/* Input: */
/* Output: */
uint32_t superNCR_sumTrial(uint32_t prevNCR, int prevN, int prevR, int curN, int curR){
gInstr++;
uint64_t result = 0, temp = 0, newResult = 0;
int i = 0;
if(curN == prevN - 1 && curR == prevR){
    temp = result = (uint64_t)prevNCR * (uint64_t)(prevN - prevR);
    
    //printf("\nresult: %llu : ", result);
    uint64_t max = 1; max <<= 63;
    //printf("max: %llu\n", max);
    for(i = 0; i <64; i++){
        if(temp & max){
            //printf("1(%d) ", 64-i);
            //newResult += (fastDivMap[prevN-1] >> i);
        } else {
            //printf("0(%d) ", 64-i);
        }
        temp <<= 1;
    }
    result = result / (uint64_t)prevN;
    //printf("oldResult: %llu ", result);
    //printf("newResult: %llu\n", newResult);
} else 
if(curN == prevN - 1 && curR == prevR - 1){
    temp = result = (uint64_t)prevNCR * (uint64_t)prevR;
    
    //printf("\nresult: %llu : ", result);
    uint64_t max = 1; max <<= 63;
    //printf("max: %llu\n", max);
    for(i = 0; i <64; i++){
        if(temp & max){
            //printf("1(%d) ", 64-i);
            //newResult += (fastDivMap[prevN-1] >> i);
        } else {
            //printf("0(%d) ", 64-i);
        }
        temp <<= 1;
    }
    result = result / (uint64_t)prevN;
    //printf("oldResult: %llu ", result);
    //printf("newResult: %llu\n", newResult);
}
return (uint32_t)result;
//return (uint32_t)newResult;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to calculate binomial value by multiplying and dividing a previous value */
/* Input: */
/* Output: */
static inline uint32_t superNCR(uint32_t prevNCR, int prevN, int prevR, int curN, int curR){
gInstr++;
if(curN == prevN - 1 && curR == prevR){
    return(((uint64_t)prevNCR * (uint64_t)(prevN - prevR)) / (uint64_t)prevN);
} else 
if(curN == prevN - 1 && curR == prevR - 1){
    return(((uint64_t)prevNCR * (uint64_t)prevR) / (uint64_t)prevN);
}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to calculate binomial value by multiplying and dividing a previous value */
/* Input: */
/* Output: */
uint32_t superNCR_trial(uint32_t prevNCR, int prevN, int prevR, int curN, int curR){
gInstr++;
uint64_t result = 0, resultFloat = 0, n = 0, m = 0, q = 0, t = 0;
unsigned int d = prevN, p = 0;
p = (int)ceil(log2((double)d));
m = (uint64_t)ceil((uint64_t)pow(2,32+p) / (uint64_t)d);
m = m & UINT32_MAX;
if(curN == prevN - 1 && curR == prevR){
    result = (((uint64_t)prevNCR * (uint64_t)(prevN - prevR)) / (uint64_t)prevN);
    //n = (uint64_t)prevNCR * (uint64_t)(prevN - prevR); 
    n = (uint64_t)prevNCR;
    q = (m * n) >> 32;
    t = ((n - q) >> 1) + q;
    t = t >> (p-1);
    t = t * (uint64_t)(prevN-prevR);
    resultFloat = t;
} else 
if(curN == prevN - 1 && curR == prevR - 1){
    result = (((uint64_t)prevNCR * (uint64_t)prevR) / (uint64_t)prevN);
    //n = (uint64_t)prevNCR * (uint64_t)prevR; 
    n = (uint64_t)prevNCR;
    q = (m * n) >> 32;
    t = ((n - q) >> 1) + q;
    t = t >> (p-1);
    t = t * (uint64_t)prevR;
    resultFloat = t;
}
printf("result: %lld, resultFloat: %lld\n", result, resultFloat);
return result;
//return resultFloat;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to convert gmp value into unsigned char array */
/* Input: */
/* Output: */
void exportUCharToBitArray(unsigned char* uChar, unsigned int len, unsigned char** bitArray){
if(LOG){ debug(HIGH, 0, "(exportUCharToBitArray)Start len: %d\n", len); }
int i = 0, j = 0, k = 0;
unsigned char ch = 0;
for(i = 0; i < len; i++){
    ch = uChar[i];
    if(LOG){ debug(VERY_LOW, 2, "uChar[%d]: %d\n", i, ch); }
    
    for(j = 0; j < 8; j++){
        if(ch & 128){ 
            if(LOG){ debug(VERY_LOW, 0, "ONE "); }
            (*bitArray)[k++] = 1;
        } else {
            if(LOG){ debug(VERY_LOW, 0, "ZERO "); }
            //if(k != 0){ (*bitArray)[k++] = 0; }
            (*bitArray)[k++] = 0;
        }
        ch <<= 1;
    }
}
//printf("first byte export: %u\n", uChar[0]);
if(LOG){ debug(HIGH, 0, "(exportUCharToBitArray)End\n"); }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to convert gmp value into unsigned char array */
/* Input: */
/* Output: */
void exportGmpToUCharArray(mpz_t bV, unsigned char** byteArray, int* byteArrayLen){
if(LOG){ debug(HIGH, 0, "(exportGmpToUCharArray)Start\n"); }
int i = 0, j = 0, k = 0;
size_t size = 1;
size_t count_p = 0;
int order = 1;
int endian = 1;
size_t nails = 0;
void *rop = NULL;
void* mpzWords = mpz_export(rop, &count_p, order, size, endian, nails, bV);
if(LOG){ debug(MEDIUM, 1, "count_p: %d\n", count_p); }

*byteArrayLen = count_p;
*byteArray = (unsigned char*)malloc(sizeof(unsigned char)*(*byteArrayLen)); memset(*byteArray, 0, sizeof(unsigned char)*(*byteArrayLen));

memcpy(*byteArray, (unsigned char*)mpzWords, sizeof(unsigned char)*(*byteArrayLen));

if(mpzWords != NULL){ free(mpzWords); }
if(LOG){ debug(HIGH, 0, "(exportGmpToUCharArray)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to convert gmp value into bit array */
/* Input: */
/* Output: */
void exportGmpToBitArray(mpz_t bV, unsigned char** bitArray, int* bitArrayLen){
if(LOG){ debug(HIGH, 0, "(exportGmpToBitArray)Start\n"); }
int i = 0, j = 0, k = 0;
size_t size = 1;
size_t count_p = 0;
int order = 1;
int endian = 1;
size_t nails = 0;
void *rop = NULL;
void* mpzWords = mpz_export(rop, &count_p, order, size, endian, nails, bV);
if(LOG){ debug(MEDIUM, 1, "count_p: %d\n", count_p); }

*bitArrayLen = mpz_sizeinbase(bV, 2);
*bitArray = (unsigned char*)malloc(sizeof(unsigned char)*(*bitArrayLen)); memset(*bitArray, 0, sizeof(unsigned char)*(*bitArrayLen));

unsigned char ch = 0;
for(i = 0; i < count_p; i++){
    ch = ((unsigned char*)mpzWords)[i];
    if(LOG){ debug(VERY_LOW, 2, "mpzWords[%d]: %d\n", i, ch); }
    
    for(j = 0; j < 8; j++){
        if(ch & 128){ 
            if(LOG){ debug(VERY_LOW, 0, "ONE "); }
            (*bitArray)[k++] = 1;
        } else {
            if(LOG){ debug(VERY_LOW, 0, "ZERO "); }
            if(k != 0){ (*bitArray)[k++] = 0; }
        }
        ch <<= 1;
    }
}
if(k != *bitArrayLen){ if(LOG){ debug(HIGH, 0, "Fatal Error in exportGmpToBitArray...\n"); } }
for(i = 0; i < *bitArrayLen; i++){ if(LOG){ debug(LOW, 1, "%d ", (*bitArray)[i]); } } if(LOG){ debug(LOW, 0, "\n"); }
if(mpzWords != NULL){ free(mpzWords); }
if(LOG){ debug(HIGH, 0, "(exportGmpToBitArray)End\n"); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to decode encoded value using gmp operations */
/* Input: */
/* Output: */
int superGmpDecodePos(mpz_t encValue, int nCount, int rCount, int* inRP, int encShifts){
if(LOG){ debug(HIGH, 2, "(superGmpDecodePos)nCount: %d, rCount: %d\n", nCount, rCount); }
uint32_t MAX = UINT32_MAX;
int i = 0, j = 0, n = 0, r = 0;
int* rp = (int*)malloc(sizeof(int)*rCount);
int rpCount = 0;

//Export encValue into bit array
unsigned char* bitArray = NULL; int bitArrayLen = 0;
exportGmpToBitArray(encValue, &bitArray, &bitArrayLen);
if(LOG){ debug(LOW, 1, "bitArrayLen: %d\n", bitArrayLen); }
for(i = 0; i < bitArrayLen; i++){ 
    if(LOG){ debug(LOW, 1, "%d ", bitArray[i]); }
} if(LOG){ debug(LOW, 0, "\n"); }

//Read the first 32bits
uint32_t curVal = 0L;
for(i = 0; i < 32; i++){
    curVal <<= 1;
    curVal = curVal | bitArray[i];
}
if(LOG){ debug(MEDIUM, 1, "curVal: %u\n", curVal); }

int encValueLen = mpz_sizeinbase(encValue, 2);
if(LOG){ debug(MEDIUM, 1, "encValueLen: %d\n", encValueLen); }

//n+1Cr is MAX
r = rCount;
int curValue = 0, prevValue = 0, btLenCurNCR = 0, btLen = 0, curPosBitArray = MAX_BIT-1, leftShifts = 0;
int prevN = nCount + 1, prevR = rCount, curN = 0, curR = 0;
int runningLeftShifts = 0, partialShift = FALSE, shiftTimes = 0;
uint32_t curNCR = 0, prevNCR = MAX;
if(LOG){ debug(LOW, 1, "prevNCR: %u\n", prevNCR); }
for(i = 0; i < nCount && r > 0; i++){
    if(LOG){ debug(LOW, 0, "-----------------------------------------------------\n"); }
    n = nCount - i;
    if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
    curN = n; curR = r;
    curNCR = superNCR(prevNCR, prevN, prevR, curN, curR);
    btLenCurNCR = gtBitLen(curNCR);
    if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLenCurNCR); }
    prevN = curN; prevR = curR;
    mpz_t bV; mpz_init(bV); mpz_set_ui(bV, curNCR);
    if(LOG){ debug(LOW, 1, "bV left shifted by %d positions\n", encShifts-runningLeftShifts); }
    mpz_mul_2exp(bV, bV, encShifts-runningLeftShifts);
    if(btLenCurNCR < 32){
        runningLeftShifts += 32-btLenCurNCR;
    }
    if(LOG){ debug(LOW, 2, "encValue size: %d, bV size: %d\n", mpz_sizeinbase(encValue, 2), mpz_sizeinbase(bV, 2)); }
    if(mpz_cmp(encValue, bV) >= 0){
        mpz_sub(encValue, encValue, bV);
        if(LOG){ debug(LOW, 3, "Position %d is valid, bV size: %d, encValueSize: %d\n", curN, mpz_sizeinbase(bV, 2), mpz_sizeinbase(encValue, 2)); }
        r--;
        rp[rpCount++] = nCount - curN + 1;
    }
    if(btLenCurNCR < 32){
        curNCR <<= 32-btLenCurNCR;
    }
    if(LOG){ debug(LOW, 3, "new curNCR after %d left shifts is %u, (%d)\n", 32-btLenCurNCR, curNCR, gtBitLen(curNCR)); }
    mpz_clear(bV);
    prevNCR = curNCR;
}
if(LOG){ debug(LOW, 0, "RP Decoded: "); }
for(j = 0; j < rpCount; j++){
    if(rp[j] != inRP[j]){ printf("Decode failure...\n"); }
    if(LOG){ debug(LOW, 1, "%d ", rp[j]); }
}
if(LOG){ debug(LOW, 0, "\n"); }
if(bitArray != NULL){ free(bitArray); }
if(rp != NULL){ free(rp); }
return;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to decode encoded value without using gmp operations */
/* Input: */
/* Output: */
int superNonGmpDecodePos(mpz_t encValue, int nCount, int rCount, int* inRP, int encShifts){
if(LOG){ debug(HIGH, 2, "(superNonGmpDecodePos)nCount: %d, rCount: %d, encShifts: %d\n", nCount, rCount, encShifts); }
uint32_t MAX = UINT32_MAX;
int i = 0, j = 0, k = 0, n = 0, r = 0;
int* rp = (int*)malloc(sizeof(int)*rCount);
int rpCount = 0;

//Export encValue into bit array
unsigned char* bitArray = NULL; int bitArrayLen = 0;
exportGmpToBitArray(encValue, &bitArray, &bitArrayLen);
if(LOG){ debug(LOW, 1, "bitArrayLen: %d\n", bitArrayLen); }
for(i = 0; i < bitArrayLen; i++){ 
    if(LOG){ debug(LOW, 1, "%d ", bitArray[i]); }
} if(LOG){ debug(LOW, 0, "\n"); }

//Read the first 32bits
uint32_t curVal = 0L;
for(i = 0; i < 32; i++){
    curVal <<= 1;
    curVal = curVal | bitArray[i];
}
if(LOG){ debug(MEDIUM, 1, "curVal: %u\n", curVal); }



//n+1Cr is MAX
r = rCount;
int curValue = 0, prevValue = 0, btLenCurNCR = 0, btLen = 0, curPosBitArray = MAX_BIT-1, leftShifts = 0;
int prevN = nCount + 1, prevR = rCount, curN = 0, curR = 0;
int runningLeftShifts = 0, partialShift = FALSE, shiftTimes = 0, curValSize = 0, lostBits = 0, runningLostBits = 0;
uint32_t curNCR = 0, prevNCR = MAX;
if(LOG){ debug(LOW, 1, "prevNCR: %u\n", prevNCR); }
for(i = 0; i < nCount && r > 0; i++){
    if(LOG){ debug(LOW, 0, "=============================================\n"); }
    n = nCount - i;
    if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
    curN = n; curR = r;
    curNCR = superNCR(prevNCR, prevN, prevR, curN, curR);
    btLenCurNCR = gtBitLen(curNCR);
    if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLenCurNCR); }

    if(i == 0){ //Only for the first time
        if(btLenCurNCR < 32){
            curNCR <<= 32-btLenCurNCR;
        }
    }
    if(LOG){ debug(LOW, 2, "curVal: %u, (%d)\n", curVal, gtBitLen(curVal)); }

    prevN = curN; prevR = curR;
    if(runningLostBits < lostBits){
        if(btLenCurNCR < 32){
            if(32-btLenCurNCR > lostBits-runningLostBits){
                curNCR <<= lostBits-runningLostBits;
                runningLostBits += lostBits-runningLostBits;
                if(LOG){ debug(LOW, 2, "new curNCR after %d left shifts is %u, (%d)\n", lostBits-runningLostBits, curNCR, gtBitLen(curNCR)); }
            } else {
                curNCR <<= 32-btLenCurNCR;
                runningLostBits += 32-btLenCurNCR;
                if(LOG){ debug(LOW, 2, "new curNCR after %d left shifts is %u, (%d)\n", 32-btLenCurNCR, curNCR, gtBitLen(curNCR)); }
            }
        }
    }
    if(LOG){ debug(LOW, 2, "lostBits: %d, runningLostBits: %d\n", lostBits, runningLostBits); }
    if(runningLostBits == lostBits){
    if(curVal >= curNCR){
        if(LOG){ debug(LOW, 1, "Position %d is valid\n", curN); }
        curVal -= curNCR; curValSize = gtBitLen(curVal);
        if(LOG){ debug(LOW, 2, "new curVal: %u, (%d)\n", curVal, curValSize); }
        curVal <<= 32 - curValSize;
        lostBits = 32-curValSize; runningLostBits = 0;
        if(LOG){ debug(LOW, 3, "new curVal after %d left shifts(lost bits: %d) : %u, (%d)\n", 32-curValSize, lostBits, curVal, gtBitLen(curVal)); }
        //pick 32-btLen bits from the bitArray and do binomial OR in the end
        for(j = 1; j < 32-curValSize; j++){
            curVal = curVal | (bitArray[curPosBitArray + j] << 32-curValSize-j);
        }
        curVal = curVal | bitArray[curPosBitArray + 32-curValSize];
        curPosBitArray += 32-curValSize;
        if(LOG){ debug(LOW, 3, "new curVal after OR with %d bits : %u, (%d)\n", 32-curValSize, curVal, gtBitLen(curVal)); }
        rp[rpCount++] = nCount - curN + 1;
        r--;
    }
    }
    btLen = gtBitLen(curNCR);
    if(btLen < 32){ 
        if(LOG){ debug(LOW, 2, "lostBits: %d, runningLostBits: %d\n", lostBits, runningLostBits); }
        if(32-btLen > lostBits-runningLostBits){
            curNCR <<= lostBits-runningLostBits; 
            runningLostBits += lostBits-runningLostBits;
            if(LOG){ debug(LOW, 3, "new curNCR after %d left shifts is %u, (%d)\n", lostBits-runningLostBits, curNCR, gtBitLen(curNCR)); }
        } else {
            curNCR <<= 32-btLen; 
            runningLostBits += 32-btLen;
            if(LOG){ debug(LOW, 3, "new curNCR after %d left shifts is %u, (%d)\n", 32-btLen, curNCR, gtBitLen(curNCR)); }
        }
        //curNCR <<= 32-btLen; 
        //runningLostBits += 32-btLen;
        //if(LOG){ debug(LOW, 3, "new curNCR after %d left shifts is %u, (%d)\n", 32-btLen, curNCR, gtBitLen(curNCR)); }
    }

    prevNCR = curNCR;
}
if(LOG){ debug(LOW, 0, "RP Decoded: "); }
for(j = 0; j < rpCount; j++){
    if(rp[j] != inRP[j]){ printf("Decode failure...\n"); }
    if(LOG){ debug(LOW, 1, "%d ", rp[j]); }
}
if(LOG){ debug(LOW, 0, "\n"); }
if(bitArray != NULL){ free(bitArray); }
if(rp != NULL){ free(rp); }
return;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to decode encoded value without using gmp operations */
/* Input: */
/* Output: */
int superDecodePos(mpz_t encValue, int nCount, int rCount, int* inRP, int encShifts){
if(LOG){ debug(HIGH, 2, "(superDecodePos)nCount: %d, rCount: %d, encShifts: %d\n", nCount, rCount, encShifts); }
uint32_t MAX = UINT32_MAX;
int i = 0, j = 0, k = 0, n = 0, r = 0;
int* rp = (int*)malloc(sizeof(int)*rCount);
int rpCount = 0;

//Export encValue into bit array
unsigned char* bitArray = NULL; int bitArrayLen = 0;
exportGmpToBitArray(encValue, &bitArray, &bitArrayLen);
if(LOG){ debug(LOW, 1, "bitArrayLen: %d\n", bitArrayLen); }
for(i = 0; i < bitArrayLen; i++){ 
    if(LOG){ debug(LOW, 1, "%d ", bitArray[i]); }
} if(LOG){ debug(LOW, 0, "\n"); }

//Read the first 32bits
uint32_t curVal = 0L;
//for(i = 0; i < 32; i++){
//    curVal <<= 1;
//    curVal = curVal | bitArray[i];
//}
//if(LOG){ debug(MEDIUM, 1, "curVal: %u\n", curVal); }



//n+1Cr is MAX
r = rCount;
int curValue = 0, prevValue = 0, btLenCurNCR = 0, btLen = 0, curPosBitArray = 0, leftShifts = 0;
int prevN = nCount + 1, prevR = rCount, curN = 0, curR = 0;
int runningLeftShifts = 0, partialShift = FALSE, shiftTimes = 0, curValSize = 0, lostBits = 0, runningLostBits = 0;
uint32_t curNCR = 0, prevNCR = MAX;
if(LOG){ debug(LOW, 1, "prevNCR: %u\n", prevNCR); }
for(i = 0; i < nCount && r > 0; i++){
    if(LOG){ debug(LOW, 0, "=============================================\n"); }
    n = nCount - i;
    if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
    curN = n; curR = r;
    curNCR = superNCR(prevNCR, prevN, prevR, curN, curR);
    btLenCurNCR = gtBitLen(curNCR);
    if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLenCurNCR); }

    if(i == 0){ //Only for the first time
        for(k = 0; k < btLenCurNCR; k++){
            curVal <<= 1;
            curVal = curVal | bitArray[k];
        }
        curPosBitArray = btLenCurNCR-1;
        if(LOG){ debug(LOW, 2, "curVal: %u, (%d)\n", curVal, gtBitLen(curVal)); }
    } else {
        if(gtBitLen(curNCR) >= gtBitLen(curVal)){
        if(LOG){ debug(LOW, 2, "curNCR is %d bit short. Left shift curNCR by %d bit and pick %d bits in curVal\n", 32-btLenCurNCR, 32-btLenCurNCR, 32-btLenCurNCR); }
        curNCR <<= 32-btLenCurNCR;
        if(LOG){ debug(LOW, 2, "new curNCR after left shift by %d bits: %u, (%d)\n", 32-btLenCurNCR, curNCR, gtBitLen(curNCR)); }
        curVal <<= 32-btLenCurNCR;
        if(LOG){ debug(LOW, 2, "new curVal after left shift by %d bits: %u, (%d)\n", 32-btLenCurNCR, curVal, gtBitLen(curVal)); }
        //pick 32-btLen bits from the bitArray and do binomial OR in the end
        for(j = 1; j < 32-btLenCurNCR; j++){
            curVal = curVal | (bitArray[curPosBitArray + j] << 32-btLenCurNCR-j);
        }
        curVal = curVal | bitArray[curPosBitArray + 32-btLenCurNCR];
        curPosBitArray += 32-btLenCurNCR;
        if(LOG){ debug(LOW, 3, "new curVal after OR with %d bits : %u, (%d)\n", 32-btLenCurNCR, curVal, gtBitLen(curVal)); }
        } else {
            if(LOG){ debug(LOW, 2, "curVal: %u, (%d)\n", curVal, gtBitLen(curVal)); }
        }
        
        
    }
    prevN = curN; prevR = curR;

    if(curVal >= curNCR){
        if(LOG){ debug(LOW, 1, "Position %d is valid\n", curN); }
        curVal -= curNCR;
        if(LOG){ debug(LOW, 2, "curVal after subtraction: %u, (%d)\n", curVal, gtBitLen(curVal)); }
        rp[rpCount++] = nCount - curN + 1;
        r--;
    }
      



    prevNCR = curNCR;
}
if(LOG){ debug(LOW, 0, "RP Decoded: "); }
for(j = 0; j < rpCount; j++){
    if(rp[j] != inRP[j]){ printf("Decode failure...\n"); }
    if(LOG){ debug(LOW, 1, "%d ", rp[j]); }
}
if(LOG){ debug(LOW, 0, "\n"); }
if(bitArray != NULL){ free(bitArray); }
if(rp != NULL){ free(rp); }
return;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to decode encoded value without using gmp operations */
/* Input: */
/* Output: */
//int superMagicDecodePos(mpz_t encValue, int nCount, int rCount, int* inRP, int firstValLen){
int superMagicDecodePos(unsigned char* encValue, int encValueLen, int curChar, int nCount, int rCount, int firstValLen, DD* id){

struct timeval t1, t2;
if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }

if(LOG){ debug(VERY_HIGH, 2, "(superMagicDecodePos)nCount: %d, rCount: %d, firstValLen: %d, encValueLen: %d\n", nCount, rCount, firstValLen, encValueLen); }
uint32_t MAX = UINT32_MAX;
uint32_t curNCR = 0, prevNCR = MAX;
int i = 0, j = 0, k = 0, n = 0, r = 0;
int* rp = (int*)malloc(sizeof(int)*rCount);
int rpCount = 0;

id->rp[curChar] = (unsigned int*)malloc(sizeof(unsigned int)*nCount); memset(id->rp[curChar], 0, sizeof(unsigned int)*nCount);

//Export encValue into bit array
int bitArrayLen = encValueLen*8;
unsigned char* bitArray = (unsigned char*)malloc(sizeof(unsigned char)*bitArrayLen*8); 
memset(bitArray, 0, sizeof(unsigned char)*bitArrayLen*8);
exportUCharToBitArray(encValue, encValueLen, &bitArray);

if(LOG){ debug(LOW, 1, "bitArrayLen: %d\n", bitArrayLen); }
for(i = 0; i < bitArrayLen; i++){ 
    if(LOG){ debug(LOW, 1, "%d ", bitArray[i]); }
} if(LOG){ debug(LOW, 0, "\n"); }

//Read the first 32bits
uint64_t curVal = 0L;
//uint32_t curVal = 0L;

//n+1Cr is MAX
r = rCount;
int curValue = 0, prevValue = 0, btLenCurNCR = 0, btLen = 0, curPosBitArray = 0, leftShifts = 0;
int prevN = nCount + 1, prevR = rCount, curN = 0, curR = 0;
int runningLeftShifts = 0, partialShift = FALSE, shiftTimes = 0, curValSize = 0, lostBits = 0, runningLostBits = 0;
int zeroBitsInEnc = 0, zeroBitsInNCR = 0, btNew = 0;

//Loose the initial 0 bits
k = 0;
while(bitArray[k++] == 0){
    curPosBitArray++;
}
//printf("curPosBitArray: %d\n", curPosBitArray);

if(LOG){ debug(LOW, 1, "prevNCR: %u\n", prevNCR); }
for(i = 0; i < nCount && r > 0; i++){
    //#if(LOG){ debug(LOW, 0, "=============================================\n"); }
    n = nCount - i;
    //#if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
    curN = n; curR = r;
    curNCR = superNCR(prevNCR, prevN, prevR, curN, curR);
    btLenCurNCR = gtBitLen(curNCR);
    //#if(LOG){ debug(LOW, 2, "prevNCR: %u, curNCR: %u, (%d)\n", prevNCR, curNCR, btLenCurNCR); }

    if(i == 0){ //Only for the first time
        ////if(LOG){ debug(LOW, 2, "encValueLen: %d, firstValLen: %d\n", encValueLen, firstValLen); }
        for(k = 0; k < firstValLen; k++){
            curVal <<= 1;
            curVal = curVal | bitArray[curPosBitArray + k];
        }
        //curPosBitArray = btLenCurNCR-1;
        curPosBitArray += firstValLen-1;
        //#if(LOG){ debug(LOW, 2, "curVal: %lld, (%d)\n", curVal, gtBitLen64(curVal)); }
    } else {
        //#if(LOG){ debug(LOW, 2, "curNCR is %d bit short. Left shift curNCR by %d bits and align the curVal boundary by picking %d bits\n", MAX_BIT-btLenCurNCR, MAX_BIT-btLenCurNCR, MAX_BIT-btLenCurNCR); }
        curNCR <<= MAX_BIT-btLenCurNCR;
        zeroBitsInNCR -= MAX_BIT-btLenCurNCR;
        //#if(LOG){ debug(LOW, 2, "curNCR after %d bits left shift is %u, (%d)\n", MAX_BIT-btLenCurNCR, curNCR, gtBitLen(curNCR)); }
        curVal <<= MAX_BIT-btLenCurNCR;
        zeroBitsInEnc -= MAX_BIT-btLenCurNCR;
        //#if(LOG){ debug(LOW, 2, "curVal after picking %d bits is %lld, (%d)\n", MAX_BIT-btLenCurNCR, curVal, gtBitLen64(curVal)); }
        //pick MAX_BIT-btLen bits from the bitArray and do binomial OR in the end
        if(MAX_BIT-btLenCurNCR > 0){
        for(j = 1; j < MAX_BIT-btLenCurNCR; j++){
            curVal = curVal | (bitArray[curPosBitArray + j] << MAX_BIT-btLenCurNCR-j);
        }
        curVal = curVal | bitArray[curPosBitArray + MAX_BIT-btLenCurNCR];
        curPosBitArray += MAX_BIT-btLenCurNCR;
        }
        //#if(LOG){ debug(LOW, 3, "curVal after OR with %d bits : %lld, (%d)\n", MAX_BIT-btLenCurNCR, curVal, gtBitLen64(curVal)); }
    }


    prevN = curN; prevR = curR;

    //#if(LOG){ debug(LOW, 3, "Compare curVal(%lld (%d)) and curNCR(%u (%d))\n", curVal, gtBitLen64(curVal), curNCR, gtBitLen(curNCR)); }


    if(curVal >= curNCR){
        //#if(LOG){ debug(LOW, 1, "Position %d is valid\n", curN); }
        curVal -= curNCR;
        //#if(LOG){ debug(LOW, 2, "curVal after subtraction: %lld, (%d)\n", curVal, gtBitLen64(curVal)); }
        rp[rpCount++] = nCount - curN + 1;
        r--;
    } else {
        //#if(LOG){ debug(LOW, 1, "Position %d is junk\n", curN); }
    }
      
    btNew = gtBitLen(curNCR);
    if(btNew < MAX_BIT){
        //#if(LOG){ debug(LOW, 1, "SPECIAL curNCR len is %d\n", btNew); }
        curNCR <<= MAX_BIT-btNew;
        //#if(LOG){ debug(LOW, 1, "Shifting curNCR left by %d bits %u\n", MAX_BIT-btNew, curNCR); }
        curVal <<= MAX_BIT-btNew;
        //#if(LOG){ debug(LOW, 1, "Picking %d bits for curVal %lld\n", MAX_BIT-btNew, curVal); }
        //pick MAX_BIT-btLen bits from the bitArray and do binomial OR in the end
        for(j = 1; j < MAX_BIT-btNew; j++){
            curVal = curVal | (bitArray[curPosBitArray + j] << MAX_BIT-btNew-j);
        }
        curVal = curVal | bitArray[curPosBitArray + MAX_BIT-btNew];
        curPosBitArray += MAX_BIT-btNew;
        //#if(LOG){ debug(LOW, 3, "curVal after OR with %d bits : %lld, (%d)\n", MAX_BIT-btNew, curVal, gtBitLen64(curVal)); }
    }

    prevNCR = curNCR;
}
if(LOG){ debug(LOW, 0, "RP Decoded: "); }
for(j = 0; j < rpCount; j++){
    //if(rp[j] != inRP[j]){ printf("Decode failure...\n"); }
    if(LOG){ debug(LOW, 1, "%d ", rp[j]); }
}

//Fill the relative positions, after removing the deliberately added 1st position
for(i = 0; i < rCount-1; i++){
    id->rp[curChar][i] = rp[i+1]-1;
}
id->rpCount[curChar] = rCount-1;

if(LOG){ debug(LOW, 0, "\n"); }
if(bitArray != NULL){ free(bitArray); }
if(rp != NULL){ free(rp); }

if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
//if(TL){ printf("superMagicDecodePos(n:%d, r:%d, curChar: %d) TIME : %lld\n", nCount, rCount, curChar, (uint64_t)timeval_diff(NULL, &t2, &t1)); }
id->tG += (uint64_t)timeval_diff(NULL, &t2, &t1);

return;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to decode encoded value without using gmp operations */
/* Input: */
/* Output: */
int superMagicDecodePosToPos(unsigned char* encValue, int encValueLen, int curChar, int nCount, int rCount, int firstValLen, int** rpReturn, DD* id){

struct timeval t1, t2;
//if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }

if(LOG){ debug(VERY_HIGH, 2, "(superMagicDecodePosToPos)nCount: %d, rCount: %d, firstValLen: %d, encValueLen: %d\n", nCount, rCount, firstValLen, encValueLen); }
uint32_t MAX = UINT32_MAX;
//uint32_t curNCR = 0, prevNCR = MAX;
register unsigned int curNCR = 0, prevNCR = MAX;
int i = 0, j = 0, k = 0, n = 0, r = 0;
int* rp = (int*)malloc(sizeof(int)*rCount);
*rpReturn = (int*)malloc(sizeof(int)*rCount);
int rpCount = 0;

//id->rp[curChar] = (unsigned int*)malloc(sizeof(unsigned int)*nCount); memset(id->rp[curChar], 0, sizeof(unsigned int)*nCount);

//Export encValue into bit array
int bitArrayLen = encValueLen*8;
unsigned char* bitArray = (unsigned char*)malloc(sizeof(unsigned char)*bitArrayLen*8); 
memset(bitArray, 0, sizeof(unsigned char)*bitArrayLen*8);
exportUCharToBitArray(encValue, encValueLen, &bitArray);

if(LOG){ debug(LOW, 1, "bitArrayLen: %d\n", bitArrayLen); }
for(i = 0; i < bitArrayLen; i++){ 
    if(LOG){ debug(LOW, 1, "%d ", bitArray[i]); }
} if(LOG){ debug(LOW, 0, "\n"); }

//Read the first 32bits
uint64_t curVal = 0L;
//uint32_t curVal = 0L;

//n+1Cr is MAX
r = rCount;
int curValue = 0, prevValue = 0, btLenCurNCR = 0, btLen = 0, curPosBitArray = 0, leftShifts = 0;
int prevN = nCount + 1, prevR = rCount, curN = 0, curR = 0;
int runningLeftShifts = 0, partialShift = FALSE, shiftTimes = 0, curValSize = 0, lostBits = 0, runningLostBits = 0;
int zeroBitsInEnc = 0, zeroBitsInNCR = 0, btNew = 0;

//Loose the initial 0 bits
k = 0;
while(bitArray[k++] == 0){
    curPosBitArray++;
}
if(LOG){ debug(VERY_HIGH, 1, "curPosBitArray: %d\n", curPosBitArray); }

if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
if(LOG){ debug(LOW, 1, "prevNCR: %u\n", prevNCR); }
for(i = 0; i < nCount && r > 0; i++){
    ////if(LOG){ debug(LOW, 0, "=============================================\n"); }
    n = nCount - i;
    ////if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
    curN = n; curR = r;
    curNCR = superNCR(prevNCR, prevN, prevR, curN, curR);
    btLenCurNCR = gtBitLen(curNCR);
    ////if(LOG){ debug(LOW, 2, "prevNCR: %u, curNCR: %u, (%d)\n", prevNCR, curNCR, btLenCurNCR); }

    if(i == 0){ //Only for the first time
        ////if(LOG){ debug(LOW, 2, "encValueLen: %d, firstValLen: %d\n", encValueLen, firstValLen); }
        for(k = 0; k < firstValLen; k++){
            curVal <<= 1;
            curVal = curVal | bitArray[curPosBitArray + k];
        }
        //curPosBitArray = btLenCurNCR-1;
        curPosBitArray += firstValLen-1;
        ////if(LOG){ debug(LOW, 2, "curVal: %lld, (%d)\n", curVal, gtBitLen64(curVal)); }
    } else {
        ////if(LOG){ debug(LOW, 2, "curNCR is %d bit short. Left shift curNCR by %d bits and align the curVal boundary by picking %d bits\n", MAX_BIT-btLenCurNCR, MAX_BIT-btLenCurNCR, MAX_BIT-btLenCurNCR); }
        curNCR <<= MAX_BIT-btLenCurNCR; zeroBitsInNCR -= MAX_BIT-btLenCurNCR;
        ////if(LOG){ debug(LOW, 2, "curNCR after %d bits left shift is %u, (%d)\n", MAX_BIT-btLenCurNCR, curNCR, gtBitLen(curNCR)); }
        curVal <<= MAX_BIT-btLenCurNCR;
        zeroBitsInEnc -= MAX_BIT-btLenCurNCR;
        ////if(LOG){ debug(LOW, 2, "curVal after picking %d bits is %lld, (%d)\n", MAX_BIT-btLenCurNCR, curVal, gtBitLen64(curVal)); }
        //pick MAX_BIT-btLen bits from the bitArray and do binomial OR in the end
        if(MAX_BIT-btLenCurNCR > 0){
        for(j = 1; j < MAX_BIT-btLenCurNCR; j++){
            curVal = curVal | (bitArray[curPosBitArray + j] << MAX_BIT-btLenCurNCR-j);
        }
        curVal = curVal | bitArray[curPosBitArray + MAX_BIT-btLenCurNCR];
        curPosBitArray += MAX_BIT-btLenCurNCR;
        }
        ////if(LOG){ debug(LOW, 3, "curVal after OR with %d bits : %lld, (%d)\n", MAX_BIT-btLenCurNCR, curVal, gtBitLen64(curVal)); }
    }


    prevN = curN; prevR = curR;

    ////if(LOG){ debug(LOW, 3, "Compare curVal(%lld (%d)) and curNCR(%u (%d))\n", curVal, gtBitLen64(curVal), curNCR, gtBitLen(curNCR)); }


    if(curVal >= curNCR){
        ////if(LOG){ debug(LOW, 1, "Position %d is valid\n", curN); }
        curVal -= curNCR;
        ////if(LOG){ debug(LOW, 2, "curVal after subtraction: %lld, (%d)\n", curVal, gtBitLen64(curVal)); }
        rp[rpCount++] = nCount - curN + 1;
        r--;
    } else {
        ////if(LOG){ debug(LOW, 1, "Position %d is junk\n", curN); }
    }
      
    btNew = gtBitLen(curNCR);
    if(btNew < MAX_BIT){
        ////if(LOG){ debug(LOW, 1, "SPECIAL curNCR len is %d\n", btNew); }
        curNCR <<= MAX_BIT-btNew;
        ////if(LOG){ debug(LOW, 1, "Shifting curNCR left by %d bits %u\n", MAX_BIT-btNew, curNCR); }
        curVal <<= MAX_BIT-btNew;
        ////if(LOG){ debug(LOW, 1, "Picking %d bits for curVal %lld\n", MAX_BIT-btNew, curVal); }
        //pick MAX_BIT-btLen bits from the bitArray and do binomial OR in the end
        for(j = 1; j < MAX_BIT-btNew; j++){
            curVal = curVal | (bitArray[curPosBitArray + j] << MAX_BIT-btNew-j);
        }
        curVal = curVal | bitArray[curPosBitArray + MAX_BIT-btNew];
        curPosBitArray += MAX_BIT-btNew;
        ////if(LOG){ debug(LOW, 3, "curVal after OR with %d bits : %lld, (%d)\n", MAX_BIT-btNew, curVal, gtBitLen64(curVal)); }
    }

    prevNCR = curNCR;
}
if(LOG){ debug(LOW, 0, "RP Decoded: "); }
for(j = 0; j < rpCount; j++){
    //if(rp[j] != inRP[j]){ printf("Decode failure...\n"); }
    if(LOG){ debug(LOW, 1, "%d ", rp[j]); }
}

//Fill the relative positions, after removing the deliberately added 1st position
for(i = 0; i < rCount-1; i++){
    (*rpReturn)[i] = rp[i+1]-1;
}

if(LOG){ debug(LOW, 0, "\n"); }
if(bitArray != NULL){ free(bitArray); }
if(rp != NULL){ free(rp); }

if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
//if(TL){ printf("superMagicDecodePos(n:%d, r:%d, curChar: %d) TIME : %lld\n", nCount, rCount, curChar, (uint64_t)timeval_diff(NULL, &t2, &t1)); }
id->tG += (uint64_t)timeval_diff(NULL, &t2, &t1);

return;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to encode positions using 32bit max for ncr  and return the encValue as buffer */
/* Input: */
/* Output: */
void superEncodePosToBuf(int logging, int* rp, int rCount, int nCount, unsigned char** encValue, unsigned int* encValueLen, unsigned char* firstValueLen){
if(LOG){ debug(VERY_HIGH, 2, "(superEncodePosToBuf)nCount: %d, rCount: %d\n", nCount, rCount); }
//wrapperCheckOneChunks(rp, nCount, rCount);

struct timeval t1, t2;
if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }

uint32_t MAX = UINT32_MAX;
uint32_t curNCR = 0, prevNCR = MAX;
int i = 0, j = 0, n = 0, r = 0;
int encShifts = 0;

if(LOG){ debug(LOW, 0, "RP Encoded: "); }
for(j = 0; j < rCount; j++){ if(LOG){ debug(LOW, 1, "%d ", rp[j]); } }
if(LOG){ debug(LOW, 0, "\n"); }
//n+1Cr is MAX
r = rCount;
int curValue = 0, prevValue = 0, btLen = 0;
int prevN = nCount + 1, prevR = rCount, curN = 0, curR = 0;
if(LOG){ debug(LOW, 1, "prevNCR: %u\n", prevNCR); }
int bc = 0; //For crosss checking
int l = 0, checkSize = 0;
mpz_t bigSum; mpz_init(bigSum); int used = FALSE;
mpz_t check; mpz_init(check);
mpz_t settled; mpz_init(settled);
unsigned int bigSumLen = 0;
for(i = 0; i < rCount; i++){
    curValue = rp[i];
    if(i == 0){
        for(j = 0; j < curValue - 1; j++){
            n = nCount - j;
            if(LOG){ debug(LOW, 0, "**********************************************\n"); }
            if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
            //Derive nCr from n+1Cr
            curN = n; curR = r; curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
            btLen = gtBitLen(curNCR); 
            if(btLen < MAX_BIT) { 
                if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen)); }
                curNCR <<= MAX_BIT-btLen;
                if(used){ mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } }
                //if(used){ mpz_mul_2exp(check, check, MAX_BIT-btLen); if(LOG){ debug(LOW, 1, "check shifted left by %d bits\n", MAX_BIT-btLen); } }
                //if(curN < 15485 && curN > 15460){ mpz_mul_2exp(check, check, MAX_BIT-btLen); gmp_printf("check: %Zd\n", check); }
                //if(curN > 61400){ mpz_mul_2exp(check, check, MAX_BIT-btLen); gmp_printf("check: %Zd\n", check); }
                //displayGmpBits(check);
                //if(curN > 0){ settleBits(check, settled, FALSE); }
                //verify(bigSum, settled, check);
                if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
                encShifts += MAX_BIT-btLen;
                bc += MAX_BIT-btLen;
            }
            else { if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); } }
            prevN = curN; prevR = curR; prevNCR = curNCR;
        }
        n = nCount - curValue + 1;
        if(LOG){ debug(LOW, 0, "**********************************************\n"); }
        if(LOG){ debug(LOW, 2, "n: %d, r: %d\n", n, r); }
        curN = n; curR = r; curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
        btLen = gtBitLen(curNCR);
        if(btLen < MAX_BIT) { 
            if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen)); }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            //mpz_add_ui(check, check, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to check\n", curNCR); }
            if(logging == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
            if(used){ mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } }
            if(used){ mpz_mul_2exp(check, check, MAX_BIT-btLen); if(LOG){ debug(LOW, 1, "check shifted left by %d bits\n", MAX_BIT-btLen); } }
            //if(curN < 15485 && curN > 15460){ mpz_add_ui(check, check, curNCR); mpz_mul_2exp(check, check, MAX_BIT-btLen); gmp_printf("check: %Zd\n", check); }
            //if(curN > 61400){ mpz_add_ui(check, check, curNCR); mpz_mul_2exp(check, check, MAX_BIT-btLen); gmp_printf("check: %Zd\n", check); }
            //displayGmpBits(check);
            //if(curN > 0){ settleBits(check, settled); }
            //verify(bigSum, settled, check);
            if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            encShifts += MAX_BIT-btLen;
            bc += MAX_BIT-btLen;
            curNCR <<= MAX_BIT-btLen; 
        }
        else { 
            if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; if(LOG){ debug("%u added to bigSum\n", curNCR); }
            //mpz_add_ui(check, check, curNCR); used = TRUE; if(LOG){ debug("%u added to check\n", curNCR); }
            //if(curN < 15485 && curN > 15460){ mpz_add_ui(check, check, curNCR); gmp_printf("check: %Zd\n", check); }
            //if(curN > 61400){ mpz_add_ui(check, check, curNCR); gmp_printf("check: %Zd\n", check); }
            //displayGmpBits(check);
            //if(curN > 0){ settleBits(check, settled); }
            //verify(bigSum, settled, check);
            if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            if(logging == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
        }
        prevN = curN; prevR = curR;
        r--;
    } else {
        for(j = 0; j < curValue - prevValue - 1; j++){
            n = nCount - prevValue - j;
            if(LOG){ debug(LOW, 0, "**********************************************\n"); }
            if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
            curN = n; curR = r; curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
            btLen = gtBitLen(curNCR);
            if(btLen < MAX_BIT) { 
                if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen)); }
                curNCR <<= MAX_BIT-btLen; 
                if(used){ mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } }
                //if(used){ mpz_mul_2exp(check, check, MAX_BIT-btLen); if(LOG){ debug(LOW, 1, "check shifted left by %d bits\n", MAX_BIT-btLen); } }
                //if(curN < 15485 && curN > 15460){ mpz_mul_2exp(check, check, MAX_BIT-btLen); gmp_printf("check: %Zd\n", check); }
                //if(curN > 61400){ mpz_mul_2exp(check, check, MAX_BIT-btLen); gmp_printf("check: %Zd\n", check); }
                //displayGmpBits(check);
                //if(curN > 0){ settleBits(check, settled); }
                //verify(bigSum, settled, check);
                if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
                encShifts += MAX_BIT-btLen;
                bc += MAX_BIT-btLen;
            }
            else { if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); } }
            prevN = curN; prevR = curR; prevNCR = curNCR;
        }
        n = nCount - curValue + 1;
        if(LOG){ debug(LOW, 0, "**********************************************\n"); }
        if(LOG){ debug(LOW, 2, "n: %d, r: %d\n", n, r); }
        curN = n; curR = r; curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
        btLen = gtBitLen(curNCR);
        if(btLen < MAX_BIT) { 
            if(LOG){ debug(LOW, 3, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen));  }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            //mpz_add_ui(check, check, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to check\n", curNCR); }
            if(logging == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
            if(used){ mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } }
            //if(used){ mpz_mul_2exp(check, check, MAX_BIT-btLen); if(LOG){ debug(LOW, 1, "check shifted left by %d bits\n", MAX_BIT-btLen); } }
            //if(curN < 15485 && curN > 15460){ mpz_add_ui(check, check, curNCR); mpz_mul_2exp(check, check, MAX_BIT-btLen); gmp_printf("check: %Zd\n", check); }
            //if(curN > 61400){ mpz_add_ui(check, check, curNCR); mpz_mul_2exp(check, check, MAX_BIT-btLen); gmp_printf("check: %Zd\n", check); }
            //displayGmpBits(check);
            //if(curN > 0){ settleBits(check, settled); }
            //verify(bigSum, settled, check);
            if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            encShifts += MAX_BIT-btLen;
            bc += MAX_BIT-btLen;
            curNCR <<= MAX_BIT-btLen; 
        }
        else { 
            if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            //mpz_add_ui(check, check, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to check\n", curNCR); }
            //if(curN < 15485 && curN > 15460){ mpz_add_ui(check, check, curNCR); gmp_printf("check: %Zd\n", check); }
            //if(curN > 61400){ mpz_add_ui(check, check, curNCR); gmp_printf("check: %Zd\n", check); }
            //displayGmpBits(check);
            //if(curN > 0){ settleBits(check, settled); }
            //verify(bigSum, settled, check);
            if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            if(logging == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
        }
        prevN = curN; prevR = curR;
        r--;
    }
    if(LOG){ debug(LOW, 1, "bigSum bit len: %d\n", mpz_sizeinbase(bigSum, 2)); }
    prevValue = curValue;
    prevNCR = curNCR;
}
bigSumLen = mpz_sizeinbase(bigSum, 2);
//if(LOG){ debug(LOW, 1, "Length of bigSum is %d\n", bigSumLen); }
if(LOG){ debug(VERY_HIGH, 1, "Length of bigSum is %u\n", bigSumLen); }
if(logging == VERY_LOW){ gmp_printf("Final encoded value bigSum: %Zd, %u\n", bigSum, bigSumLen); }

if(LOG){ debug(MEDIUM, 1, "firstValueLen: %d\n", bigSumLen - encShifts); }
*firstValueLen = (unsigned char)(bigSumLen - encShifts);
if(LOG){ debug(MEDIUM, 1, "encVauelLen: %u\n", (unsigned int)ceil((float)bigSumLen/(float)8)); }
*encValueLen = (unsigned int)ceil((float)bigSumLen/(float)8);

//convert gmp to unsigned char*
writeGmpToBuf(bigSum, bigSumLen, encValue);

mpz_clear(bigSum);
mpz_clear(check);
mpz_clear(settled);

if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
//if(TL){ printf("superEncodePos TIME : %lld\n", (uint64_t)timeval_diff(NULL, &t2, &t1)); }
//if(TL){ printf("superEncodePosToBuf(n:%d, r:%d  TIME : %lld\n", nCount, rCount, (uint64_t)timeval_diff(NULL, &t2, &t1)); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to encode positions using 32bit max for ncr */
/* Input: */
/* Output: */
void superEncodePosGood(CD* od, int curChar, int* rp, int rCount, int nCount){
if(LOG){ debug(VERY_HIGH, 2, "(superEncodePos)nCount: %d, rCount: %d\n", nCount, rCount); }

struct timeval t1, t2, t3, t4, t5, t6;
uint64_t t43 = 0, t65 = 0;
if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }

uint32_t MAX = UINT32_MAX;
//uint32_t curNCR = 0, prevNCR = MAX;
//register uint32_t curNCR = 0, prevNCR = MAX;
register unsigned int curNCR = 0, prevNCR = MAX;
int n = 0, r = 0;
register int i = 0, j = 0;
int encShifts = 0;

if(LOG){ debug(LOW, 0, "RP Encoded: "); }
//for(j = 0; j < rCount; j++){ if(LOG){ debug(LOW, 1, "%d ", rp[j]); } }
if(LOG){ debug(LOW, 0, "\n"); }
//n+1Cr is MAX
r = rCount;
int curValue = 0, prevValue = 0, btLen = 0;
register int prevN = nCount + 1, prevR = rCount, curN = 0, curR = 0, used = FALSE;
if(LOG){ debug(LOW, 1, "prevNCR: %u\n", prevNCR); }
int bc = 0; //For crosss checking
int l = 0;
mpz_t bigSum; 
//mpz_init(bigSum); 
mpz_init2(bigSum, approxBitLenNCR(nCount, rCount)); 
unsigned int bigSumLen = 0;
for(i = 0; i < rCount; i++){
    curValue = rp[i];
    if(i == 0){
        for(j = 0; j < curValue - 1; j++){
            n = nCount - j;
            //if(LOG){ debug(LOW, 0, "**********************************************\n"); }
            //if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
            //Derive nCr from n+1Cr
            curN = n; curR = r; 
            ////if(TL){ if(gettimeofday(&t3, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
            curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
            ////if(TL){ if(gettimeofday(&t4, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
            ////t43 += (uint64_t)timeval_diff(NULL, &t4, &t3);
            btLen = gtBitLen(curNCR); 
            if(btLen < MAX_BIT) { 
                //if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen)); }
                curNCR <<= MAX_BIT-btLen;
                if(used){ 
                    mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); 
                    //if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } 
                }
                //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
                encShifts += MAX_BIT-btLen;
                bc += MAX_BIT-btLen;
            }
            else { 
                //if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); } 
            }
            prevN = curN; prevR = curR; prevNCR = curNCR;
        }
        n = nCount - curValue + 1;
        //if(LOG){ debug(LOW, 0, "**********************************************\n"); }
        //if(LOG){ debug(LOW, 2, "n: %d, r: %d\n", n, r); }
        curN = n; curR = r; 
        ////if(TL){ if(gettimeofday(&t3, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
        curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
        ////if(TL){ if(gettimeofday(&t4, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
        ////t43 += (uint64_t)timeval_diff(NULL, &t4, &t3);
        btLen = gtBitLen(curNCR);
        if(btLen < MAX_BIT) { 
            //if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen)); }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; 
            //if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            //if(od->logLevel == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
            if(used){ 
                mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); 
                //if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } 
            }
            //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            encShifts += MAX_BIT-btLen;
            bc += MAX_BIT-btLen;
            curNCR <<= MAX_BIT-btLen; 
        }
        else { 
            //if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; 
            //if(LOG){ debug("%u added to bigSum\n", curNCR); }
            //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            //if(od->logLevel == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
        }
        prevN = curN; prevR = curR;
        r--;
    } else {
        for(j = 0; j < curValue - prevValue - 1; j++){
            n = nCount - prevValue - j;
            //if(LOG){ debug(LOW, 0, "**********************************************\n"); }
            //if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
            curN = n; curR = r; 
            ////if(TL){ if(gettimeofday(&t3, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
            curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
            ////if(TL){ if(gettimeofday(&t4, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
            ////t43 += (uint64_t)timeval_diff(NULL, &t4, &t3);
            btLen = gtBitLen(curNCR);
            if(btLen < MAX_BIT) { 
                //if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen)); }
                curNCR <<= MAX_BIT-btLen; 
                if(used){ 
                    mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); 
                    //if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } 
                }
                //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
                encShifts += MAX_BIT-btLen;
                bc += MAX_BIT-btLen;
            }
            else { 
                //if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); } 
            }
            prevN = curN; prevR = curR; prevNCR = curNCR;
        }
        n = nCount - curValue + 1;
        //if(LOG){ debug(LOW, 0, "**********************************************\n"); }
        //if(LOG){ debug(LOW, 2, "n: %d, r: %d\n", n, r); }
        curN = n; curR = r; 
        ////if(TL){ if(gettimeofday(&t3, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
        curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
        ////if(TL){ if(gettimeofday(&t4, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
        ////t43 += (uint64_t)timeval_diff(NULL, &t4, &t3);
        btLen = gtBitLen(curNCR);
        if(btLen < MAX_BIT) { 
            //if(LOG){ debug(LOW, 3, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen));  }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; 
            //if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            //if(od->logLevel == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
            if(used){ 
                mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); 
                //if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } 
            }
            //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            encShifts += MAX_BIT-btLen;
            bc += MAX_BIT-btLen;
            curNCR <<= MAX_BIT-btLen; 
        }
        else { 
            //if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; 
            //if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            //if(od->logLevel == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
        }
        prevN = curN; prevR = curR;
        r--;
    }
    //if(LOG){ debug(LOW, 1, "bigSum bit len: %d\n", mpz_sizeinbase(bigSum, 2)); }
    prevValue = curValue;
    prevNCR = curNCR;
}
bigSumLen = mpz_sizeinbase(bigSum, 2);

//if(LOG){ debug(LOW, 1, "Length of bigSum is %d\n", bigSumLen); }
if(LOG){ debug(MEDIUM, 1, "Length of bigSum is %u\n", bigSumLen); }
if(od->logLevel == VERY_LOW){ gmp_printf("Final encoded value bigSum: %Zd, %u\n", bigSum, bigSumLen); }
//if(LOG){ debug(LOW, 1, "%d added as the firstValDec[%d]\n", bigSumLen - encShifts, od->firstValDecLen); }




if(LOG){ debug(MEDIUM, 2, "%d added as the firstValDec[%d]\n", bigSumLen - encShifts, od->firstValDecLen); }
od->firstValDec[(od->firstValDecLen)++] = (unsigned char)(bigSumLen - encShifts);
if(od->firstValDecLen > od->firstValDecAllocSize){ printf("FATAL ERROR: firstValDecLen exceeds firstValDecAllocSize\n"); }
if(LOG){ debug(MEDIUM, 2, "%u added as the encValLen[%u]\n", (unsigned int)ceil((float)bigSumLen/(float)8), od->encValCount); }
od->encValLen[(od->encValCount)++] = (unsigned int)ceil((float)bigSumLen/(float)8);
if(od->encValCount*sizeof(unsigned int) > od->encValLenAllocSize){ printf("FATAL ERROR: encValCount exceedss encValLenAllocSize\n"); }



//Decode
//superMagicDecodePos(bigSum, nCount, rCount, rp, bigSumLen - encShifts);

writeGmpToFileFull(bigSum, bigSumLen, od);

mpz_clear(bigSum);
if(LOG){ debug(VERY_HIGH, 0, "(superEncodePos)End Length of encoded value: %d bits\n", bigSumLen); }

//if(TL){ printf("superEncodePos(n:%d, r:%d) superNCR TIME : %lld\n", nCount, rCount, t43); }
if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
if(TL){ printf("superEncodePos(n:%d, r:%d) TIME : %lld\n", nCount, rCount, (uint64_t)timeval_diff(NULL, &t2, &t1)); }
od->tG += (uint64_t)timeval_diff(NULL, &t2, &t1);
}


void settleCarryCore(uint64_t* inSum, unsigned char *st, int* stLen, uint64_t curNCR){ 
int i = 0;
uint64_t temp = 1; temp <<= 63;
printf("Sum will exceed 64 bits. Taking care of the carry...\n");
printf("inSum: %llu, UINT64_MAX: %llu, curNCR: %u\n", *inSum, UINT64_MAX, curNCR);
//*inSum = curNCR - (UINT64_MAX - *inSum);
if(overWrite == FALSE){
    *inSum = curNCR - (UINT64_MAX - *inSum);
    for(i = *stLen-1; i--; i >= 0){
        if(st[i] == 1){ st[i] = 0; printf(" 1-->0 "); }
        else {
            if(st[i] == 0){ st[i] = 1; printf(" 0-->1 "); break; }
        }
    }
    *inSum = *inSum | temp;
    overWrite = TRUE;
} else { //overWrite == TRUE
    printf("BRAVO overWrite TRUE\n");
    *inSum = curNCR - (UINT64_MAX - *inSum);
    *inSum = *inSum | temp;
    overWrite = FALSE;
}
}

void settleCarry(uint64_t* inSum, unsigned char *st, int* stLen, uint64_t curNCR){ 
//printf("sum: %llu overWrite: %d ", *inSum, overWrite);
int i = 0;
uint64_t temp = 1; temp <<= 63;
if(UINT64_MAX - *inSum < curNCR){ // sum will exceed the 64 bit boundary
    settleCarryCore(inSum, st, stLen, curNCR);
} else {
    *inSum += curNCR;
}
//printf("  sum: %llu\n", *inSum);
}


void settleShiftCore(uint64_t* inSum, unsigned char *st, int* stLen, unsigned int shiftBy){ 
int i = 0, inSumBitLen = gtBitLen64(*inSum);
uint64_t temp = 1;
for(i = 1; i <= shiftBy - (64 - inSumBitLen); i++){
    temp = 1;
    temp <<= (inSumBitLen-1 - (i-1));
    if((*inSum & temp) == 0){
        //printf("0");
        st[(*stLen)++] = 0;
    } else {
        *inSum = *inSum & (UINT64_MAX - temp);
        if(overWrite == TRUE){
            //printf("1(0)");
            st[(*stLen)++] = 0;
            overWrite = FALSE;
        } else {
            //printf("1");
            st[(*stLen)++] = 1;
        }
    }
}
//*inSum <<= shiftBy - (64-inSumBitLen);
*inSum <<= shiftBy;
temp = 1; temp <<= 63;
if((*inSum & temp) == 0){
    *inSum = *inSum | temp;
    overWrite = TRUE;
}
}


void settleShift(uint64_t* inSum, unsigned char *st, int* stLen, unsigned int shiftBy){ 
//printf("sum: %llu shiftBy: %d overWrite: %d ", *inSum, shiftBy, overWrite);
int i = 0;
uint64_t temp = 1; temp <<= 63;
if(gtBitLen64(*inSum) + shiftBy > 64){ // sum will exceed the 64 bit boundary
    //printf(" core ");
    settleShiftCore(inSum, st, stLen, shiftBy);
} else {
    //printf(" no core ");
    *inSum <<= shiftBy;
}
//printf("  sum: %llu\n", *inSum);
}


void settleCarryOld(uint64_t* inSum, unsigned char *st, int* stLen){ 
printf("sumCarry: %lld  ", *inSum);
int i = 0;
/*
uint64_t temp = 1, sum = *inSum; temp <<= SETTLE_BITS-1;
for(i = *stLen-1; i--; i >= 0){
    if(st[i] == 1){ st[i] = 0; printf(" 1-->0 "); }
    else {
        if(st[i] == 0){ st[i] = 1; printf(" 0-->1 "); break; }
    }
}
while((sum & temp) == 0){
    st[(*stLen)++] = 0;
    temp >>= 1;
}
sum <<= 64-SETTLE_BITS; sum>>= 64-SETTLE_BITS;
*inSum = sum;
*/


if(overWrite == FALSE){
    for(i = *stLen-1; i--; i >= 0){
        if(st[i] == 1){ st[i] = 0; printf(" 1-->0 "); }
        else {
            if(st[i] == 0){ st[i] = 1; printf(" 0-->1 "); break; }
        }
    }
    uint64_t temp = 1, sum = *inSum; temp <<= SETTLE_BITS-1;
    sum = sum | temp;
    overWrite = TRUE;
    printf(" over-write most significant bit as 1. ");
    sum <<= 64-SETTLE_BITS; sum>>= 64-SETTLE_BITS;
    *inSum = sum;
} else {
    uint64_t temp = 1, sum = *inSum; temp <<= SETTLE_BITS-1;
    sum = sum | temp;
    overWrite = FALSE;
    printf(" restoring the most significant bit as 1. ");
    sum <<= 64-SETTLE_BITS; sum>>= 64-SETTLE_BITS;
    *inSum = sum;
}
printf("sumCarry: %lld\n", *inSum);
}

void settle(uint64_t* inSum, unsigned char *st, int* stLen){ 
printf("sum: %lld  ", *inSum);
unsigned int s = 0, t = 0; 
uint64_t temp = 1, sum = *inSum, leftMark = 1; temp <<= 63; leftMark <<= SETTLE_BITS-1;
while((temp & sum) == 0){ 
    temp >>= 1; s++; 
} 
for(t = s+1; t <= 64-SETTLE_BITS; t++){ 
    if((temp & sum) == 0){ 
        printf("0"); 
        st[(*stLen)++] = 0;
    } else { 
        if(overWrite == TRUE){
            printf("1(0)"); 
            st[(*stLen)++] = 0;
            overWrite = FALSE;
        } else {
            printf("1"); 
            st[(*stLen)++] = 1;
        }
    } 
    temp >>= 1; 
} 
printf(" s: %d  ", s);
/*while((sum & temp) == 0){
    printf("*%lld*", temp);
    st[(*stLen)++] = 0;
    temp >>= 1;
}*/
if((sum & leftMark) == 0){
    printf(" most significant bit is zero, over-write it as 1... ");
    overWrite = TRUE;
    sum = sum | leftMark;
} else {
    overWrite = FALSE;
}
sum <<= 64-SETTLE_BITS; sum>>= 64-SETTLE_BITS;
*inSum = sum;
printf("sum: %lld\n", *inSum);
}


void finalizeSettle(unsigned char* st, unsigned int* stLen, uint64_t sum){
//printf("sum: %lld  ", sum);
unsigned int s = 0, t = 0;
uint64_t temp = 1; temp <<= 63;
while((temp & sum) == 0){
    temp >>= 1; s++;
}
for(t = s+1; t <= 64; t++){ 
    if((temp & sum) == 0){ 
        //printf("0"); 
        st[(*stLen)++] = 0;
    } else { 
        if(overWrite == TRUE){
            //printf("finalizeSettle: overWrite\n");
            //printf("1(0)"); 
            st[(*stLen)++] = 0;
            overWrite = FALSE;
        } else {
            //printf("1"); 
            st[(*stLen)++] = 1;
        }
    } 
    temp >>= 1; 
} 
//printf(" s: %d  ", s);
}

void compareGmpSumBits(mpz_t bigSum, unsigned char* st, unsigned int stLen){
size_t size = 1;
size_t count_p = 0;
int order = 1;
int endian = 1;
size_t nails = 0;
void* rop = NULL;
void* mpzWords = mpz_export(rop, &count_p, order, size, endian, nails, bigSum);
int i = 0, s = 0, stLenTemp = 0;
unsigned char temp = 1, firstByte = 0; temp <<= 7;
firstByte = ((unsigned char*)mpzWords)[0];
//printf("Number of gmp words: %d, firstByte: %d\n", count_p, firstByte);
for(i = 1; i <= 8; i++){
    if((temp & firstByte) != 0){ break;}
    s++; temp >>= 1;
}
temp = 1; temp <<= 7-s;
for(i = s; i < 8; i++){
    if((temp & firstByte) == 0){
        //printf("%d-%d # ", 0, st[stLenTemp]);
        if(st[stLenTemp++] != 0){ printf("mismatch...\n"); }
    } else {
        //printf("%d-%d # ", 1, st[stLenTemp]);
        if(st[stLenTemp++] != 1){ printf("mismatch...\n"); }
    }
    temp >>= 1;
}
for(i = 0; i < count_p - 1; i++){
    temp = 1; temp <<= 7;
    firstByte = ((unsigned char*)mpzWords)[i+1];
    for(s = 0; s < 8; s++){
        if((temp & firstByte) == 0){
            //printf("%d-%d # ", 0, st[stLenTemp]);
            if(st[stLenTemp++] != 0){ printf("mismatch...\n"); }
        } else {
            //printf("%d-%d # ", 1, st[stLenTemp]);
            if(st[stLenTemp++] != 1){ printf("mismatch...\n"); }
        }
        temp >>= 1;
    }
}


if(mpzWords != NULL){ free(mpzWords); }
}

void writeEncValueToFileFull(unsigned char* st, unsigned int stLen, CD* od){
while(od->outBufLen + stLen/8 + DOUBLEUP_MARGIN >= od->outBufAllocSize){ doubleUpChar(&(od->outBuf), &(od->outBufAllocSize), od->outBufLen); }
unsigned char tempChar = 0;
int i = 0, j = 0;
if(stLen % 8 != 0){
    for(i = 0; i < stLen % 8; i++){
        if(st[i] == 1){
            tempChar = tempChar | (1 << ((stLen % 8) - (i+1)));
        }
    }
    memcpy(od->outBuf + od->outBufLen, &tempChar, sizeof(unsigned char)); od->outBufLen += sizeof(unsigned char);
    if(od->outBufLen > od->outBufAllocSize){ printf("FATAL ERROR: outBufLen(%d) exceeds outBufAllocSize(%d)\n", od->outBufLen, od->outBufAllocSize); }
}
for(i = stLen % 8; i < stLen; ){
    tempChar = 0;
    for(j = 0; j < 8; j++){
        if(st[i] == 1){
            tempChar = tempChar | (1 << (8 - (j+1)));
        }
        i++;
    }
    memcpy(od->outBuf + od->outBufLen, &tempChar, sizeof(unsigned char)); od->outBufLen += sizeof(unsigned char);
    if(od->outBufLen > od->outBufAllocSize){ printf("FATAL ERROR: outBufLen(%d) exceeds outBufAllocSize(%d)\n", od->outBufLen, od->outBufAllocSize); }
}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to encode positions using 32bit max for ncr */
/* Input: */
/* Output: */
void superEncodePos(CD* od, int curChar, int* rp, int rCount, int nCount){
if(LOG){ debug(VERY_HIGH, 2, "(superEncodePosSpl)nCount: %d, rCount: %d\n", nCount, rCount); }
//wrapperCheckOneChunks(rp, nCount, rCount);

struct timeval t1, t2, t3, t4, t5, t6;
uint64_t t43 = 0, t65 = 0;
if(TL){ if(gettimeofday(&t1, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }

uint32_t MAX = UINT32_MAX;
//uint32_t curNCR = 0, prevNCR = MAX;
//register uint32_t curNCR = 0, prevNCR = MAX;
register unsigned int curNCR = 0, prevNCR = MAX;
int n = 0, r = 0;
register int i = 0, j = 0, s = 0, t = 0;
int encShifts = 0;

if(LOG){ debug(LOW, 0, "RP Encoded: "); }
//for(j = 0; j < rCount; j++){ if(LOG){ debug(LOW, 1, "%d ", rp[j]); } }
if(LOG){ debug(LOW, 0, "\n"); }
//n+1Cr is MAX
r = rCount;
int curValue = 0, prevValue = 0, btLen = 0;
register int prevN = nCount + 1, prevR = rCount, curN = 0, curR = 0, used = FALSE;
if(LOG){ debug(LOW, 1, "prevNCR: %u\n", prevNCR); }
int bc = 0; //For crosss checking
int l = 0;
mpz_t bigSum;  //GMP
mpz_init(bigSum);  //GMP
//mpz_init2(bigSum, approxBitLenNCR(nCount, rCount)); 
uint64_t sum = 0, leftMark = 1, rightMark = 1, temp = 0;
leftMark <<= SETTLE_BITS; leftMark -= 1;
//printf("leftMark: %lld, rightMark: %lld\n", leftMark, rightMark);
overWrite = FALSE; prevO = FALSE;
//unsigned char* st = (unsigned char*)malloc(sizeof(unsigned char)*1000000); memset(st, 0, sizeof(unsigned char)*1000000);
unsigned char* st = (unsigned char*)malloc(sizeof(unsigned char)*nCount + 64); memset(st, 0, sizeof(unsigned char)*nCount + 64);
unsigned int stLen = 0;
unsigned int bigSumLen = 0;
for(i = 0; i < rCount; i++){
    curValue = rp[i];
    if(i == 0){
        for(j = 0; j < curValue - 1; j++){
            n = nCount - j;
            //#if(LOG){ debug(LOW, 0, "**********************************************\n"); }
            //#if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
            //Derive nCr from n+1Cr
            curN = n; curR = r; 
            ////if(TL){ if(gettimeofday(&t3, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
            curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
            ////if(TL){ if(gettimeofday(&t4, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
            ////t43 += (uint64_t)timeval_diff(NULL, &t4, &t3);
            btLen = gtBitLen(curNCR); 
            if(btLen < MAX_BIT) { 
                //#if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen)); }
                curNCR <<= MAX_BIT-btLen;
                if(used){ 
                    mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); //GMP
                    settleShift(&sum, st, &stLen, MAX_BIT-btLen);
                    //sum <<= MAX_BIT-btLen;
                    //printf("shift by: %d\n", MAX_BIT-btLen);
                    //if(sum > leftMark){ settle(&sum, st, &stLen); }
                    //#if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } 
                }
                //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
                encShifts += MAX_BIT-btLen;
                bc += MAX_BIT-btLen;
            }
            else { 
                //#if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); } 
            }
            prevN = curN; prevR = curR; prevNCR = curNCR;
        }
        n = nCount - curValue + 1;
        if(LOG){ debug(LOW, 0, "**********************************************\n"); }
        if(LOG){ debug(LOW, 2, "n: %d, r: %d\n", n, r); }
        curN = n; curR = r; 
        ////if(TL){ if(gettimeofday(&t3, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
        curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
        ////if(TL){ if(gettimeofday(&t4, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
        ////t43 += (uint64_t)timeval_diff(NULL, &t4, &t3);
        btLen = gtBitLen(curNCR);
        if(btLen < MAX_BIT) { 
            //#if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen)); }
            mpz_add_ui(bigSum, bigSum, curNCR); //GMP
            used = TRUE; 
            settleCarry(&sum, st, &stLen, curNCR);
            //sum += curNCR;
            //if(sum > leftMark){ settleCarry(&sum, st, &stLen); }
            //#if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            //if(od->logLevel == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
            if(used){ 
                mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); //GMP
                settleShift(&sum, st, &stLen, MAX_BIT-btLen);
                //sum <<= MAX_BIT-btLen;
                //printf("shift by: %d\n", MAX_BIT-btLen);
                //if(sum > leftMark){ settle(&sum, st, &stLen); }
                //#if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } 
            }
            //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            encShifts += MAX_BIT-btLen;
            bc += MAX_BIT-btLen;
            curNCR <<= MAX_BIT-btLen; 
        }
        else { 
            //#if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); }
            mpz_add_ui(bigSum, bigSum, curNCR); //GMP
            used = TRUE; 
            settleCarry(&sum, st, &stLen, curNCR);
            //sum += curNCR;
            //if(sum > leftMark){ settleCarry(&sum, st, &stLen); }
            //#if(LOG){ debug("%u added to bigSum\n", curNCR); }
            //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            //if(od->logLevel == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
        }
        prevN = curN; prevR = curR;
        r--;
    } else {
        for(j = 0; j < curValue - prevValue - 1; j++){
            n = nCount - prevValue - j;
            //#if(LOG){ debug(LOW, 0, "**********************************************\n"); }
            //#if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
            curN = n; curR = r; 
            ////if(TL){ if(gettimeofday(&t3, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
            curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
            ////if(TL){ if(gettimeofday(&t4, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
            ////t43 += (uint64_t)timeval_diff(NULL, &t4, &t3);
            btLen = gtBitLen(curNCR);
            if(btLen < MAX_BIT) { 
                //#if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen)); }
                curNCR <<= MAX_BIT-btLen; 
                if(used){ 
                    mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); //GMP
                    settleShift(&sum, st, &stLen, MAX_BIT-btLen);
                    //sum <<= MAX_BIT-btLen;
                    //printf("shift by: %d\n", MAX_BIT-btLen);
                    //if(sum > leftMark){ settle(&sum, st, &stLen); }
                    //#if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } 
                }
                //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
                encShifts += MAX_BIT-btLen;
                bc += MAX_BIT-btLen;
            }
            else { 
                //#if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); } 
            }
            prevN = curN; prevR = curR; prevNCR = curNCR;
        }
        n = nCount - curValue + 1;
        //#if(LOG){ debug(LOW, 0, "**********************************************\n"); }
        //#if(LOG){ debug(LOW, 2, "n: %d, r: %d\n", n, r); }
        curN = n; curR = r; 
        ////if(TL){ if(gettimeofday(&t3, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
        curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
        ////if(TL){ if(gettimeofday(&t4, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
        ////t43 += (uint64_t)timeval_diff(NULL, &t4, &t3);
        btLen = gtBitLen(curNCR);
        if(btLen < MAX_BIT) { 
            //#if(LOG){ debug(LOW, 3, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (MAX_BIT-btLen));  }
            mpz_add_ui(bigSum, bigSum, curNCR); //GMP
            used = TRUE; 
            settleCarry(&sum, st, &stLen, curNCR);
            //sum += curNCR;
            //if(sum > leftMark){ settleCarry(&sum, st, &stLen); }
            //#if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            //if(od->logLevel == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
            if(used){ 
                mpz_mul_2exp(bigSum, bigSum, MAX_BIT-btLen); //GMP
                settleShift(&sum, st, &stLen, MAX_BIT-btLen);
                //sum <<= MAX_BIT-btLen;
                //printf("shift by: %d\n", MAX_BIT-btLen);
                //if(sum > leftMark){ settle(&sum, st, &stLen); }
                //#if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", MAX_BIT-btLen); } 
            }
            //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            encShifts += MAX_BIT-btLen;
            bc += MAX_BIT-btLen;
            curNCR <<= MAX_BIT-btLen; 
        }
        else { 
            //#if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); }
            mpz_add_ui(bigSum, bigSum, curNCR); //GMP
            used = TRUE; 
            settleCarry(&sum, st, &stLen, curNCR);
            //sum += curNCR;
            //if(sum > leftMark){ settleCarry(&sum, st, &stLen); }
            //#if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            //if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            //if(od->logLevel == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
        }
        prevN = curN; prevR = curR;
        r--;
    }
    //if(LOG){ debug(LOW, 1, "bigSum bit len: %d\n", mpz_sizeinbase(bigSum, 2)); }
    prevValue = curValue;
    prevNCR = curNCR;
}
bigSumLen = mpz_sizeinbase(bigSum, 2);

finalizeSettle(st, &stLen, sum);
//bigSumLen = stLen;
//printf("bitLen: %d, stLen: %d\n", bigSumLen, stLen);
//compareGmpSumBits(bigSum, st, stLen);
compareGmpSumBits(bigSum, st, bigSumLen);

writeGmpToFileFull(bigSum, mpz_sizeinbase(bigSum, 2), od);
//writeEncValueToFileFull(st, stLen, od);

//if(LOG){ debug(LOW, 1, "Length of bigSum is %d\n", bigSumLen); }
if(LOG){ debug(MEDIUM, 1, "Length of bigSum is %u\n", bigSumLen); }
//if(od->logLevel == VERY_LOW){ gmp_printf("Final encoded value bigSum: %Zd, %u\n", bigSum, bigSumLen); }
//if(LOG){ debug(LOW, 1, "%d added as the firstValDec[%d]\n", bigSumLen - encShifts, od->firstValDecLen); }




if(LOG){ debug(MEDIUM, 2, "%d added as the firstValDec[%d]\n", bigSumLen - encShifts, od->firstValDecLen); }
od->firstValDec[(od->firstValDecLen)++] = (unsigned char)(bigSumLen - encShifts);
if(od->firstValDecLen > od->firstValDecAllocSize){ printf("FATAL ERROR: firstValDecLen exceeds firstValDecAllocSize\n"); }
if(LOG){ debug(MEDIUM, 2, "%u added as the encValLen[%u]\n", (unsigned int)ceil((float)bigSumLen/(float)8), od->encValCount); }
od->encValLen[(od->encValCount)++] = (unsigned int)ceil((float)bigSumLen/(float)8);
if(od->encValCount*sizeof(unsigned int) > od->encValLenAllocSize){ printf("FATAL ERROR: encValCount exceedss encValLenAllocSize\n"); }



//Decode
//superMagicDecodePos(bigSum, nCount, rCount, rp, bigSumLen - encShifts);


if(st != NULL){ free(st); }
mpz_clear(bigSum);
if(LOG){ debug(VERY_HIGH, 0, "(superEncodePos)End Length of encoded value: %d bits\n", bigSumLen); }

//if(TL){ printf("superEncodePos(n:%d, r:%d) superNCR TIME : %lld\n", nCount, rCount, t43); }
if(TL){ if(gettimeofday(&t2, NULL)){ debug(HIGH, 0, "Failure in gettimeofday()...\n"); } }
#ifdef DEBUG
//if(TL){ printf("superEncodePos(n:%d, r:%d, curChar: %d, bitLen: %d, bpp: %.2f) TIME : %lld\n", nCount, rCount, curChar, bigSumLen, (float)(bigSumLen)/(float)rCount, (uint64_t)timeval_diff(NULL, &t2, &t1)); }
#endif
od->tG += (uint64_t)timeval_diff(NULL, &t2, &t1);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Function to encode positions using 32bit max for ncr */
/* Input: */
/* Output: */
void superEncodePosSettle(int logging, int* rp, int rCount, int nCount){
if(LOG){ debug(HIGH, 2, "nCount: %d, rCount: %d\n", nCount, rCount); }
uint32_t MAX = UINT32_MAX;
int i = 0, j = 0, n = 0, r = 0;
int encShifts = 0;

if(LOG){ debug(LOW, 0, "RP Encoded: "); }
for(j = 0; j < rCount; j++){ if(LOG){ debug(LOW, 1, "%d ", rp[j]); } }
if(LOG){ debug(LOW, 0, "\n"); }
//n+1Cr is MAX
r = rCount;
int curValue = 0, prevValue = 0, btLen = 0;
int prevN = nCount + 1, prevR = rCount, curN = 0, curR = 0;
uint32_t curNCR = 0, prevNCR = MAX;
if(LOG){ debug(LOW, 1, "prevNCR: %u\n", prevNCR); }
int bc = 0; //For crosss checking
int l = 0, checkSize = 0, carry = FALSE;
int nLast = 0, oldCheckLen = 0;
int carrySettled = TRUE;
mpz_t bigSum; mpz_init(bigSum); int used = FALSE;
mpz_t check; mpz_init(check);
mpz_t settled; mpz_init(settled);
for(i = 0; i < rCount; i++){
    curValue = rp[i];
    if(i == 0){
        for(j = 0; j < curValue - 1; j++){
            n = nCount - j;
            if(LOG){ debug(LOW, 0, "**********************************************\n"); }
            if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
            //Derive nCr from n+1Cr
            curN = n; curR = r; 
            if(curN == curR){
                if(LOG){ debug(LOW, 2, "curN(%d) and curR(%d) are equal\n", curN, curR); }
                nLast = curN;
            }
            curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
            btLen = gtBitLen(curNCR); 
            if(btLen < 32) { 
                if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (32-btLen)); }
                curNCR <<= 32-btLen;
                if(used){ mpz_mul_2exp(bigSum, bigSum, 32-btLen); if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", 32-btLen); } }
                if(used){ mpz_mul_2exp(check, check, 32-btLen); if(LOG){ debug(LOW, 1, "check shifted left by %d bits\n", 32-btLen); } }
                //if(curN < 15485 && curN > 15460){ mpz_mul_2exp(check, check, 32-btLen); gmp_printf("check: %Zd\n", check); }
                //if(curN > 61400){ mpz_mul_2exp(check, check, 32-btLen); gmp_printf("check: %Zd\n", check); }
                //displayGmpBits(check);
                if(carrySettled == TRUE){ carry = FALSE; }
                if(curN > 0){ settleBits(check, settled, carry, &carrySettled); }
                verify(bigSum, settled, check, !carrySettled);
                if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
                encShifts += 32-btLen;
                bc += 32-btLen;
            }
            else { if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); } }
            prevN = curN; prevR = curR; prevNCR = curNCR;
        }
        n = nCount - curValue + 1;
        if(LOG){ debug(LOW, 0, "**********************************************\n"); }
        if(LOG){ debug(LOW, 2, "n: %d, r: %d\n", n, r); }
        curN = n; curR = r; 
        if(curN == curR){
            if(LOG){ debug(LOW, 2, "curN(%d) and curR(%d) are equal\n", curN, curR); }
            nLast = curN;
            break;
        } else {
        curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
        btLen = gtBitLen(curNCR);
        if(btLen < 32) { 
            if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (32-btLen)); }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            oldCheckLen = mpz_sizeinbase(check, 2);
            mpz_add_ui(check, check, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to check\n", curNCR); }
            if(mpz_cmp_ui(settled, 0) != 0 && oldCheckLen == mpz_sizeinbase(check, 2) - 1){
                if(LOG){ debug(LOW, 0, "most significant bit changed from 0 to 1 due to carry\n"); }
                mpz_clrbit(check, oldCheckLen);
                mpz_setbit(check, oldCheckLen-1);
                carry = TRUE;
            } else { carry = FALSE; }
            if(logging == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
            if(used){ mpz_mul_2exp(bigSum, bigSum, 32-btLen); if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", 32-btLen); } }
            if(used){ mpz_mul_2exp(check, check, 32-btLen); if(LOG){ debug(LOW, 1, "check shifted left by %d bits\n", 32-btLen); } }
            //if(curN < 15485 && curN > 15460){ mpz_add_ui(check, check, curNCR); mpz_mul_2exp(check, check, 32-btLen); gmp_printf("check: %Zd\n", check); }
            //if(curN > 61400){ mpz_add_ui(check, check, curNCR); mpz_mul_2exp(check, check, 32-btLen); gmp_printf("check: %Zd\n", check); }
            //displayGmpBits(check);
            if(carry == TRUE){ 
                if(carrySettled == FALSE){ printf("WARNING: double carry\n"); }
                if(curN > 0){ settleBits(check, settled, TRUE, &carrySettled); }
            } else {
                if(curN > 0){ settleBits(check, settled, FALSE, &carrySettled); }
            }
            verify(bigSum, settled, check, !carrySettled);
            if(carry == TRUE && lastSettled == TRUE){ carry = FALSE; }
            if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            encShifts += 32-btLen;
            bc += 32-btLen;
            curNCR <<= 32-btLen; 
        }
        else { 
            if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; if(LOG){ debug("%u added to bigSum\n", curNCR); }
            oldCheckLen = mpz_sizeinbase(check, 2);
            mpz_add_ui(check, check, curNCR); used = TRUE; if(LOG){ debug("%u added to check\n", curNCR); }
            if(mpz_cmp_ui(settled, 0) != 0 && oldCheckLen == mpz_sizeinbase(check, 2) - 1){
                if(LOG){ debug(LOW, 0, "most significant bit changed from 0 to 1 due to carry\n"); }
                mpz_clrbit(check, oldCheckLen);
                mpz_setbit(check, oldCheckLen-1);
                carry = TRUE;
            } else { carry = FALSE; }
            //if(curN < 15485 && curN > 15460){ mpz_add_ui(check, check, curNCR); gmp_printf("check: %Zd\n", check); }
            //if(curN > 61400){ mpz_add_ui(check, check, curNCR); gmp_printf("check: %Zd\n", check); }
            //displayGmpBits(check);
            if(carry == TRUE){ 
                if(carrySettled == FALSE){ printf("WARNING: double carry\n"); }
                if(curN > 0){ settleBits(check, settled, TRUE, &carrySettled); }
            } else {
                if(curN > 0){ settleBits(check, settled, FALSE, &carrySettled); }
            }
            verify(bigSum, settled, check, !carrySettled);
            if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            if(logging == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
        }
        prevN = curN; prevR = curR;
        r--;
        }
    } else {
        for(j = 0; j < curValue - prevValue - 1; j++){
            n = nCount - prevValue - j;
            if(LOG){ debug(LOW, 0, "**********************************************\n"); }
            if(LOG){ debug(LOW, 2, "N: %d, R: %d\n", n, r); }
            curN = n; curR = r; 
            if(curN == curR){
                if(LOG){ debug(LOW, 2, "curN(%d) and curR(%d) are equal\n", curN, curR); }
                nLast = curN;
            }
            curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
            btLen = gtBitLen(curNCR);
            if(btLen < 32) { 
                if(LOG){ debug(LOW, 2, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (32-btLen)); }
                curNCR <<= 32-btLen; 
                if(used){ mpz_mul_2exp(bigSum, bigSum, 32-btLen); if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", 32-btLen); } }
                if(used){ mpz_mul_2exp(check, check, 32-btLen); if(LOG){ debug(LOW, 1, "check shifted left by %d bits\n", 32-btLen); } }
                //if(curN < 15485 && curN > 15460){ mpz_mul_2exp(check, check, 32-btLen); gmp_printf("check: %Zd\n", check); }
                //if(curN > 61400){ mpz_mul_2exp(check, check, 32-btLen); gmp_printf("check: %Zd\n", check); }
                //displayGmpBits(check);
                //
                if(carrySettled == TRUE){ carry = FALSE; }
                if(curN > 0){ settleBits(check, settled, carry, &carrySettled); }
                verify(bigSum, settled, check, !carrySettled);
                if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
                encShifts += 32-btLen;
                bc += 32-btLen;
            }
            else { if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); } }
            prevN = curN; prevR = curR; prevNCR = curNCR;
        }
        n = nCount - curValue + 1;
        if(LOG){ debug(LOW, 0, "**********************************************\n"); }
        if(LOG){ debug(LOW, 2, "n: %d, r: %d\n", n, r); }
        curN = n; curR = r; 
        if(curN == curR){
            if(LOG){ debug(LOW, 2, "curN(%d) and curR(%d) are equal\n", curN, curR); }
            nLast = curN;
            break;
        } else {
        curNCR = superNCR(prevNCR, prevN, prevR, curN, curR); 
        btLen = gtBitLen(curNCR);
        if(btLen < 32) { 
            if(LOG){ debug(LOW, 3, "curNCR: %u, (%d), newNCR: %u\n", curNCR, btLen, curNCR << (32-btLen));  }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            oldCheckLen = mpz_sizeinbase(check, 2);
            mpz_add_ui(check, check, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to check\n", curNCR); }
            if(mpz_cmp_ui(settled, 0) != 0 && oldCheckLen == mpz_sizeinbase(check, 2) - 1){
                if(LOG){ debug(LOW, 0, "most significant bit changed from 0 to 1 due to carry\n"); }
                mpz_clrbit(check, oldCheckLen);
                mpz_setbit(check, oldCheckLen-1);
                carry = TRUE;
            } else { carry = FALSE; }
            if(logging == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
            if(used){ mpz_mul_2exp(bigSum, bigSum, 32-btLen); if(LOG){ debug(LOW, 1, "bigSum shifted left by %d bits\n", 32-btLen); } }
            if(used){ mpz_mul_2exp(check, check, 32-btLen); if(LOG){ debug(LOW, 1, "check shifted left by %d bits\n", 32-btLen); } }
            //if(curN < 15485 && curN > 15460){ mpz_add_ui(check, check, curNCR); mpz_mul_2exp(check, check, 32-btLen); gmp_printf("check: %Zd\n", check); }
            //if(curN > 61400){ mpz_add_ui(check, check, curNCR); mpz_mul_2exp(check, check, 32-btLen); gmp_printf("check: %Zd\n", check); }
            //displayGmpBits(check);
            if(carry == TRUE){ 
                if(carrySettled == FALSE){ printf("WARNING: double carry\n"); }
                if(curN > 0){ settleBits(check, settled, TRUE, &carrySettled); }
            } else {
                if(curN > 0){ settleBits(check, settled, FALSE, &carrySettled); }
            }
            verify(bigSum, settled, check, !carrySettled);
            if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            encShifts += 32-btLen;
            bc += 32-btLen;
            curNCR <<= 32-btLen; 
        }
        else { 
            if(LOG){ debug(LOW, 2, "curNCR: %u, (%d)\n", curNCR, btLen); }
            mpz_add_ui(bigSum, bigSum, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to bigSum\n", curNCR); }
            oldCheckLen = mpz_sizeinbase(check, 2);
            mpz_add_ui(check, check, curNCR); used = TRUE; if(LOG){ debug(LOW, 1, "%u added to check\n", curNCR); }
            if(mpz_cmp_ui(settled, 0) != 0 && oldCheckLen == mpz_sizeinbase(check, 2) - 1){
                if(LOG){ debug(LOW, 0, "most significant bit changed from 0 to 1 due to carry\n"); }
                mpz_clrbit(check, oldCheckLen);
                mpz_setbit(check, oldCheckLen-1);
                carry = TRUE;
                if(carrySettled == FALSE){ printf("WARNING: double carry\n"); }
                if(curN > 0){ settleBits(check, settled, TRUE, &carrySettled); }
            } else { 
                carry = FALSE; 
                if(curN > 0){ settleBits(check, settled, FALSE, &carrySettled); }
            }
            //if(curN < 15485 && curN > 15460){ mpz_add_ui(check, check, curNCR); gmp_printf("check: %Zd\n", check); }
            //if(curN > 61400){ mpz_add_ui(check, check, curNCR); gmp_printf("check: %Zd\n", check); }
            //displayGmpBits(check);
            verify(bigSum, settled, check, !carrySettled);
            if(LOG){ debug(LOW, 1, "bigSum len: %d\n", mpz_sizeinbase(bigSum, 2)); }
            if(logging == VERY_LOW){ gmp_printf("%Zd", bigSum); } if(LOG){ debug(LOW, 0, "\n"); }
        }
        prevN = curN; prevR = curR;
        r--;
        }
    }
    if(LOG){ debug(LOW, 1, "bigSum bit len: %d\n", mpz_sizeinbase(bigSum, 2)); }
    prevValue = curValue;
    prevNCR = curNCR;
}
printf("nLast: %d\n", nLast);
//Finally settle the last bits

gmp_printf("unsettled bigSum is %Zd (%d)\n", check, mpz_sizeinbase(check, 2));
int unsettledLen = mpz_sizeinbase(check, 2);
mpz_mul_2exp(settled, settled, unsettledLen);
if(mpz_cmp_ui(settled, 0) != 0){ mpz_clrbit(check, unsettledLen-1); } //take care
gmp_printf("unsettled bigSum is %Zd\n", check);
mpz_add(settled, settled, check);
gmp_printf("after final settlement settled len is %d\n", mpz_sizeinbase(settled, 2));
//FILE* f1 = (FILE*)openFile(LOW, "settled.txt", "wb"); gmp_fprintf(f1, "%Zd", settled); fclose(f1);

gmp_printf("bigSumLen: %d\n", mpz_sizeinbase(bigSum, 2));
//FILE* f2 = (FILE*)openFile(LOW, "bigSum.txt", "wb"); gmp_fprintf(f2, "%Zd", bigSum); fclose(f2);


if(mpz_cmp(settled, bigSum) != 0){
    printf("Settle failure...\n");
} else {
    printf("Settle Verified...\n");
}

if(logging == VERY_LOW){ gmp_printf("Final encoded value bigSum: %Zd, %d\n", bigSum, mpz_sizeinbase(bigSum, 2)); }
//superSuperDecodePos(bigSum, nCount, rCount, rp, encShifts);
//superMagicDecodePos(bigSum, nCount, rCount, rp, encShifts);
//superDecodePos(bigSum, nCount, rCount, rp, encShifts);
//superNonGmpDecodePos(bigSum, nCount, rCount, rp, encShifts);
//superGmpDecodePos(bigSum, nCount, rCount, rp, encShifts);
//superGmpDecodePos(settled, nCount, rCount, rp, encShifts);
mpz_clear(bigSum);
mpz_clear(check);
mpz_clear(settled);
}
