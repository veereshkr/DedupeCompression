#include<stdio.h>
#include "lib.h"
EState* Initilize(EState *s,UInt32 rawSize)
{	
	UInt32 n;
	s=NULL;
	s = (EState *)malloc( sizeof(EState) );	
		if(s==NULL)
		{
			 printf("Lack Of Memory\n");
	 		 exit(0);
		}
	 s->arr1 = NULL;
	 s->arr2 = NULL;
   	 s->ftab = NULL;
	 n = 900000 ;
	 s->arr1 = (UInt32 *)malloc( n                  * sizeof(UInt32) );
	 s->arr2 = (UInt32 *)malloc((n+OVERSHOOT)       * sizeof(UInt32) );
   	 s->ftab = (UInt32 *)malloc( 65537              * sizeof(UInt32) );
	
	 if (s->arr1 == NULL || s->arr2 == NULL || s->ftab == NULL) 
	 {
      		if (s->arr1 != NULL) free(s->arr1);
                if (s->arr2 != NULL) free(s->arr2);
                if (s->ftab != NULL) free(s->ftab);
                if (s       != NULL) free(s);
		printf("Lack Of Memory\n");
	 	exit(0);
                      
	 }
	 s->workFactor=30;
	 s->block           = (UChar*)s->arr2;
   	 s->ptr             = (UInt32*)s->arr1;
         return s;
}

void  UnInitilize(EState *s)
{
	 if (s->arr1 != NULL) free(s->arr1);
	 if (s->arr2 != NULL) free(s->arr2);
	 if (s->ftab != NULL) free(s->ftab);
	 if (s       != NULL) free(s);
}

DState* DInitilize(DState *s)
{	
	UInt32 n;
	s=NULL;
	s = (DState *)malloc( sizeof(DState) );	
		if(s==NULL)
		{
			 printf("Lack Of Memory\n");
	 		 exit(0);
		}
	 s->arr1 = NULL;
	 s->arr2 = NULL;
	 n = 900000 ;
	 s->arr1  = (UInt32 *)malloc( n * sizeof(UInt32 ) );
	 s->arr2  = (UInt32 *)malloc( n * sizeof(UInt32 ) );
	 s->block = (UInt32 *)malloc( n * sizeof(UInt32 ) );
	
	 if (s->arr1 == NULL || s->arr2 == NULL || s->block == NULL) 
	 {
      		if (s->arr1 != NULL) free(s->arr1);
                if (s->arr2 != NULL) free(s->arr2);
                if (s->block!= NULL) free(s->arr2);
                if (s       != NULL) free(s);
		printf("Lack Of Memory\n");
	 	exit(0);
                      
	 }
         return s;
}
void  DUnInitilize(DState *s)
{
	 if (s->arr1  != NULL) free(s->arr1);
	 if (s->arr2  != NULL) free(s->arr2);
	 if (s->block != NULL) free(s->block);
	 if (s        != NULL) free(s);
}

