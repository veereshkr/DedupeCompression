#include <stdio.h>
#include <stdlib.h>

//#define DEBUG

#define FILE_BLOCK_SIZE (500*1024)

#define DOUBLEUP_MARGIN 32

#define FULL_GMP FALSE

#define MAX_CHARS 256
#define MAX_FILENAME 256

#define SUCCESS 1
#define FAILURE 0

#define TRUE 1
#define FALSE 0

//For Logging
#define VERY_LOW 5
#define LOW 4
#define MEDIUM 3
#define HIGH 2
#define VERY_HIGH 1
#define NO_LOG 0
#define LOG_LEVEL VERY_LOW

#define LOG_ON 1
#define LOG_OFF 0
#define LOG LOG_ON
//#define LOG LOG_OFF

#define TL TRUE
//#define TL FALSE

#define NONE 0
#define COMPRESS 1
#define DECOMPRESS 2

#define _32BITMAX
#define MAX_BIT 32

#define BLOCK_TYPES_COUNT_8 8
#define BLOCK_TYPES_COUNT_4 4
#define TYPE_8 7
#define TYPE_7 6
#define TYPE_6 5
#define TYPE_5 4
#define TYPE_4 3
#define TYPE_3 2
#define TYPE_2 1
#define TYPE_1 0

#define D_BLOCK_SIZE 64
#define D_BLOCK_TYPES 4

//Modes for blockIds compression
#define NORMAL_WITHOUT_MTF 1
#define NORMAL_WITH_MTF 2
#define EXPANDED_WITH_MTF 3

#define SETTLE_BITS 52
