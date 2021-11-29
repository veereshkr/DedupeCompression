#include <stdint.h>
#include <stdio.h>

#define MAX_CHARS 256

//For count of characters and for sorting the characters based on their frequencies
struct order {
	unsigned char ch;
	int count;
};

typedef struct order ORDER;

struct compressData {
    unsigned int inCharCount;
    unsigned char* charData; unsigned short charDataCount, charDataAllocSize;
    FILE* of; int ofDataLen;
    unsigned char* fileBuf; unsigned int fileBufAllocSize; unsigned int fileBufLen;
    unsigned char* outBuf; unsigned int outBufLen, outBufAllocSize;
    unsigned char blockIdCompressMode;
    unsigned char* blockIdBuf; unsigned int blockIdBufLen, blockIdBufAllocSize;
    unsigned char* partialBytesBuf; unsigned short partialBytesBufLen;
    unsigned char* partialBytes;
    int emptyBytesCount;
    int distinctCharCount;
    unsigned char* nCountBuf; unsigned short nCountBufLen, nCountBufAllocSize;
    unsigned char* nCountBytes;
    unsigned char densityEncodedBufCount;
    unsigned int* nCountRandomBuf; unsigned short nCountRandomBufLen, nCountRandomBufAllocSize;
    unsigned int* rCountBuf; unsigned short rCountBufLen, rCountBufAllocSize;
    unsigned char* splitChars; unsigned short splitCharsCount, splitCharsAllocSize;
    unsigned char* splitDensityIdBits; unsigned short splitDensityIdBitsCount, splitDensityIdBitsAllocSize;
    unsigned char* splitDensityIdBytes; unsigned char splitDensityIdBytesLen;
    unsigned char* filename; unsigned char filenameLength;
    int logLevel;
    unsigned char* inBuf; unsigned int inBufLen;
    unsigned char* firstValDec; unsigned short firstValDecLen, firstValDecAllocSize;
    unsigned int* encValLen; unsigned short encValCount, encValLenAllocSize;
    uint64_t tG;
};

typedef struct compressData CD;

struct blockData {
    int curChar;
    unsigned short blockSize;
    unsigned char blockTypesCount;
    unsigned int blockCount;
    unsigned char* blockId;
    unsigned int blockIdLen;
    unsigned int** blockRP;
    unsigned int* blockRPCount;
    unsigned int* blockRPTypesCount;
    unsigned int* blockRPNCount;
    unsigned int blockStartPos;
    unsigned int allBlocksRPCount;
    unsigned int allBlocksRPNCount;
    unsigned int* allRP;
};

typedef struct blockData BD;

struct splitData {
    unsigned int* rpZero;
    unsigned int rpZeroCount;
    unsigned int rpZeroN;
    unsigned int* rpOne;
    unsigned int rpOneCount;
    unsigned int rpOneN;
    unsigned char* splitId;
    unsigned int splitIdCount;
    unsigned int splitIdZeroCount;
    unsigned int* rpSplitId;
    unsigned int rpSplitIdN;
    unsigned int rpSplitIdCount;
    unsigned int* allRP;
};

typedef struct splitData SD;
