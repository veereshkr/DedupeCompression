#include "lib.h"
#include "Initilize.h"
#include "blocksort.h"
#include "runArunB.h"
unsigned char range = 70;

UChar *sufix=".Vz2";
void printError()
{
	printf("\n\tUsage: vajra [flag] -e --compression || -d --decompress fileName\n");
	printf("\tTerminating......!\n\n");
}

void ClearAll(File *inFile,File *outFile ,UChar *OutName)
{
		if( inFile!=NUL) fclose(inFile);
		if(outFile!=NUL) fclose(outFile);
		if(OutName!=NUL) free(OutName);
}
static Bool fileExists ( UChar* name )
{
   File *tmp   = fopen ( name, "rb" );
   Bool exists = (tmp != NUL);
   if (tmp != NUL) fclose ( tmp );
   return exists;
}
void writeFile(unsigned char* block, int rawSize)
{
    //FILE *fid=fopen("decode.txt","wb");
    //fwrite(block,1,rawSize,fid);
    //fclose(fid);
}

UInt32 main(UInt32 argc,UChar *argv[])
{
	Bool EncodeMode=False;
	Bool DecodeMode=False;
	if(argc==1 || argc < 3 || argc > 3)
	  {
		printError();
		exit(0);
	  }
	  argv++;
	 if(**argv == '-')
	 {
		(*argv)++;
		if((**argv=='e'|| **argv == 'E') && argv[0][1] == '\0')
		   {
			EncodeMode=True;
		   	DecodeMode=False;
			argv++;
		   }
		else if((**argv=='d'|| **argv == 'D') && argv[0][1] == '\0')
		   {
			EncodeMode=False;
		   	DecodeMode=True;
			argv++;
		   }
		else
		   {
			printError();
			exit(0);
		   }
	 }
	else
	{
		printError();
		exit(0);
	}

	File *inFile,*outFile;	
	inFile=NUL;	
	outFile=NUL;
	if(EncodeMode)
	{
		EState *s = NUL;
		UInt32 rawSize=0,i=0;
		s=Initilize(s,rawSize);//Initilize the structure Estate-allocates memory
		// check if infile with .Vz2 sufix
		{
			UChar *ptr;
			Int32 Length;
			Length = strlen(argv[0])-4;
			if(Length >= 0)
			  {
				ptr = &(argv[0][Length]); 
				if(strcmp(ptr,sufix) == 0)
				{
					printf("Input file sufix  already with \"%s\" extension \n",sufix);
					UnInitilize(s);
					exit(0);
				}
				 
			  }	
		}
		UChar* OutName=NUL;
		{
			OutName=(UChar*) malloc ( (strlen(argv[0])+strlen(sufix)+1) * sizeof(UChar) );
			if(OutName == NUL)
			{	
				printf("failed to allocate memory\n");
				UnInitilize(s);
				exit(0);
			}
			strcpy(OutName,argv[0]);
			strcat(OutName,sufix);
		}
		if(fileExists(OutName))
		{
			printf("OutPut file \"%s\" Already  Exists\n",OutName);
			ClearAll(inFile,outFile,OutName);
			UnInitilize(s);
			exit(0);
		}
		inFile  = fopen(argv[0],"rb");
		if(inFile == NUL)
		{
			printf("There is no such file  \"%s\" in Current Directory\n",argv[0]);
			UnInitilize(s);
			ClearAll(inFile,outFile,OutName);
			exit(0);
	 	}
		
		outFile = fopen(OutName,"wb");
		if(outFile == NUL)
		{
			printf("Unable to open Outfile \n");
			UnInitilize(s);
			ClearAll(inFile,outFile,OutName);
			exit(0);
	 	}
		while( (rawSize = fread(s->block, sizeof(UChar), maxBlockSize , inFile) )!= 0)
		{
			s->nblock=rawSize;	
			blockSort (s) ;// for BWT sorting from BZIP2
		        
			UChar* last = NUL; 
			/// here last can be used as alias to s->ptr , s->ptr contains the rotated index,
			/// No need to allocate memory for *last
			/// it has to be changed with error checking
			last = (UChar *)s->ptr; 
			for (i = 0; i < rawSize; i++)
          		{
                   		if (s->ptr[i] != 0)
                    		{
                        		last[i] = s->block[s->ptr[i] - 1];
                    		}
                    		else
                    		{
                        		s->origPtr = i;
                        		last[i] = s->block[rawSize   - 1];
                    		}
                	}
			// last contains the BWT sorted data 
			//s->arr2 can be used with conversion
			// last pointing to s->arr1 alias of s->ptr
			//int tempSize = rawSize;
			printf("rawSize:%d ",rawSize);
			int tt,z=0,tfive=0,bz=0;
			moveToFront(last,rawSize);
			for(tt=0;tt<rawSize;tt++)
			{
				if(last[tt]==255)
				   tfive++;
				if(last[tt]==0)
				bz++;
			}
			/////RunA RunB function call
			char runFlag=0;
			int  rem=0;
			if( bz>0 && tfive == 0)
			{
			  rem = (bz  * 100/ s->nblock)  ;
			  //printf("%d :%d /%d * 100: %d \n",tfive , bz,s->nblock,rem);
			  if(rem > range)
			  {
				rawSize = RunARunB(last,rawSize,s);
				s->nblock=rawSize;
			/////// now s->arr2 contains data after RunA RunB , last pointing to s->arr2
				last = (UChar *)s->arr2;
				runFlag=1;
			  }
			}
			printf("runFlag:%d ,afer AB rawSize:%d ",runFlag,rawSize);
			
			for(tt=0;tt<rawSize;tt++)
			{
				if(last[tt]==0) z++;
			}
			//printf("[%d]total:%7d zero:%7d two55 : %7d ",chak,bz,z,tfive);
			/*************************************************************/
			//if(rawSize < 200000)
			//	last=(UChar *)s->arr2;
			//else { 
			//	last = (UChar *)s->ptr;
			//	rawSize=tempSize;}
				
			unsigned char* outBuf = NULL; 
			int outBufLen = 0;
			compressBuf(last, rawSize, &outBuf, &outBufLen); 
			//printf("outBufLen:%d \n",outBufLen);
			/*************************************************************/
			printf("after Vajra:outbufLen:%d \n",outBufLen);
			
			fwrite(&s->origPtr   , sizeof(Int32) , 1        , outFile);
			fwrite(&runFlag      , sizeof(char)  , 1        , outFile);
			fwrite(&outBufLen    , sizeof(Int32), 1        , outFile);
	                fwrite(outBuf        , sizeof(UChar) , outBufLen, outFile);
			if(outBuf != NULL) free(outBuf); 
		}

	/*************************************/
		UnInitilize(s);
		ClearAll(inFile,outFile,OutName);
	}
	if(DecodeMode)
	{	
		if( strlen(argv[0]) < (strlen(sufix)+1))
		{
			printf("Input file should be with \"%s\" extension \n",sufix);
			exit(0);
		}
		//Decode Structure and initilize
		//initilize decode structure here
		DState *s=NUL;
		s=DInitilize(s);
		File *inFile  = NUL,
		     *outFile = NUL;
		Int32 Length;
		UInt32 i;
		UChar *OutName = NUL,
		      *ptr     = NUL;	
		/*********** This piece of code checks for .Vz2 File Extension*********/
		Length=strlen(argv[0])-strlen(sufix);
		OutName=(UChar*)malloc( (Length + 1 )*sizeof(UChar) );
		if(OutName == NUL)
		{
			printf("failed to allocate memory\n");
			DUnInitilize(s);
			exit(0);	
		}
		ptr=&(argv[0][Length]);
		if(strcmp(ptr,sufix) != 0)
		{
			printf("Not Valid Input file\n");
			printf("Input file should be with \"%s\" extension \n",sufix);
			DUnInitilize(s);
			ClearAll(inFile,outFile,OutName);
			exit(0);
		}
		/*********************************************************************/
		for(i=0;i<Length;i++) OutName[i]=argv[0][i];OutName[i+1]='\0';
		// check if input file name already exists	
		if(fileExists(OutName))
		{
			printf("Output file \"%s\"  already Exists\n",OutName);
			DUnInitilize(s);
			ClearAll(inFile,outFile,OutName);
			exit(0);
		}
		inFile  = fopen(argv[0],"rb");
                if(inFile == NUL)
                {
			printf("There is no such file  \"%s\" in Current Directory\n",argv[0]);
			DUnInitilize(s);
			ClearAll(inFile,outFile,OutName);
                        exit(0);
                }
                outFile = fopen(OutName,"wb");
                if(outFile == NUL)
                {
			DUnInitilize(s);
			ClearAll(inFile,outFile,OutName);
                        printf("Unable to open Outfile\n");
                        exit(0);
                }
		/****************UnDoBWT starts here***********************/
		UInt32* pred = s->arr1;
		UChar* block = (UChar* )s->block;
		UChar* unrotated = (UChar* )s->arr2;
		UInt32 sum,count[MAX_CHAR],index;
		Int32 rawSize=0;
		Int32 j;
		while(fread(&index, sizeof(Int32), 1, inFile) != 0)
		 {
			char runFlag=0;
			fread(&runFlag,sizeof(char),1,inFile);
			unsigned char* outBuf = NULL; int outBufLen = 0;
       			fread(&rawSize, sizeof(Int32), 1, inFile);
			block = (UChar* )s->block; //it required coz if runFlag it true then block pointing to outBuf
			printf("runFlag falg :%d read from file :rawSize:%d ",runFlag,rawSize);
       			fread(block, sizeof(UChar), rawSize, inFile);
		//	int z = 0;
		//	if(rawSize == 203618){ for(z = 0; z < rawSize; z++){ printf("%d ", block[z]); } }
			//for(i=0;i<rawSize;i++)
			//printf("%d ",block[i]);
			decodeBuf(block, rawSize, &outBuf, &outBufLen); 
			//printf("outBufLen:%d ",outBufLen);
			//for(i=0;i<outBufLen;i++)
			//printf("%d ",outBuf[i]);
       			//fread(unrotated, sizeof(UChar), rawSize, inFile);
			rawSize=outBufLen;
			printf("after Vajra :rawSize:%d ",rawSize);
			if(runFlag)
			rawSize = UnDoRunARunB(block,outBuf,rawSize);
			else block  = (unsigned char*)outBuf;
			printf("after runAb :rawSize:%d \n",rawSize);
			UnDoMoveToFront(block,rawSize);
			for(i = 0; i < MAX_CHAR; i++)
        		{
	            		count[i] = 0;
        		}
			for (i = 0; i < rawSize; i++)
		        {
            			pred[i] = count[block[i]];
            			count[block[i]]++;
        		}
			sum = 0;
        		for(i = 0; i < MAX_CHAR; i++)
	        	{
           			j = count[i];
            			count[i] = sum;
            			sum += j;
        		}
			i = index;
		        for(j = rawSize - 1; j >= 0; j--)
        		{
     			       unrotated[j] = block[i];
    			        i = pred[i] + count[block[i]];
        		}
        		fwrite(unrotated, sizeof(UChar), rawSize,outFile);
			if(outBuf != NULL) { free(outBuf); }
		}
		DUnInitilize(s);
                ClearAll(inFile,outFile,OutName);
	}
	remove(argv[0]);
   return 0;
}
