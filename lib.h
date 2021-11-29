#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*-- General stuff. --*/


typedef char            Char;
typedef unsigned char   Bool;
typedef unsigned char   UChar;
typedef int             Int32;
typedef unsigned int    UInt32;
typedef short           Int16;
typedef unsigned short  UInt16;
typedef FILE  		File;

#define True  1
#define False 0
#define RUNA 0
#define RUNB 1

#define RADIX    2
#define QSORT    12
#define SHELL    18
#define OVERSHOOT (RADIX + QSORT + SHELL + 2)
#define maxBlockSize 899981
//#define maxBlockSize 500981
//#define maxBlockSize 2024000 
#define MAX_CHAR     256
#define NUL  NULL



/*-- Structure holding all the compression-side stuff. --*/

typedef
   struct {

      /* for doing the block sorting */
      UInt32*  arr1;
      UInt32*  arr2;
      UInt32*  ftab;
      Int32    origPtr;

      /* aliases for arr1 and arr2 */
      UInt32*  ptr;
      UChar*   block;
      Int32    workFactor;

      /* input and output limits and current posns */
      Int32    nblock;

   }
   EState;



/*-- Structure holding all the compression-side stuff. --*/

typedef
   struct {

      /* for undoing the block sorting */
      UInt32*  arr1;
      UInt32*   arr2;
      UInt32*  block;

      /* input and output limits and current posns */
      Int32    nblock;

   }
   DState;
