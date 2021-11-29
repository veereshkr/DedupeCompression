#include <stdio.h>
#include <stdint.h>

struct decompressData {
    unsigned int inDataLen; //Not used
    unsigned int inCharCount;
    unsigned char* charData; unsigned short charDataCount;
    //FILE* of; int ofDataLen;
    //unsigned char* fileBuf; int fileBufLen;
    unsigned char* outBuf; unsigned int outBufLen;
    unsigned char blockIdCompressMode;
    unsigned char* blockIdBuf; unsigned int blockIdBufLen;
    unsigned char* blockIds; unsigned int blockIdCount, blockIdN, blockIdOffset;
    unsigned char* partialBytesBuf; unsigned short partialBytesBufLen;
    unsigned char* partialBytes;
    int emptyBytesCount;
    int distinctCharCount;
    unsigned char* nCountBuf; unsigned short nCountBufLen;
    unsigned char* nCountBytes;
    unsigned char densityEncodedBufCount;
    unsigned int* nCountRandomBuf; unsigned short nCountRandomLen;
    unsigned int* nCountValues;
    unsigned int* rCountBuf; unsigned short rCountBufLen;
    unsigned char* splitChars; unsigned char splitCharsCount;
    unsigned char* splitDensityIdBits; unsigned short splitDensityIdBitsCount;
    unsigned char* splitDensityIdBytes; unsigned char splitDensityIdBytesLen;
    unsigned int* rp[MAX_CHARS];
    unsigned int rpCount[MAX_CHARS];
    unsigned char* decData; unsigned int decDataLen;
    unsigned char* decCharData;
    FILE* decFile;
    unsigned char* filename; unsigned char filenameLength;
    int logLevel;
    unsigned char* inBuf; unsigned int inBufLen;
    unsigned char* firstValDec; unsigned short firstValDecLen;
    unsigned int* encValLen; unsigned short encValCount;
    uint64_t tG;
};

typedef struct decompressData DD;
