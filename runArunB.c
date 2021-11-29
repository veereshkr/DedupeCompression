#include "lib.h"
Int32 RunAB(UChar* last,UInt32 count,UInt32 *ptrCount)
{
UInt32 num,temp,BitOn,BitOff;
num = count;
	while(num)
	{
		BitOff=0;
		BitOn =0;
		temp=num&1;//Check For last bit weather is ON or OFF
		if(temp) BitOn =1;
	  	else     BitOff=1;
		if(BitOn)
		{
	  	   last[(*ptrCount)++]=RUNA;
	  	   num=num>>1;
	  	   continue;
		}
		if(BitOff)
	   	{
	  	  last[(*ptrCount)++]=RUNB;
	  	  num=num>>1;
	  		while(num>1)
	        	{	
			  temp=num&3;
			  if(temp%2==0)
		 	    {
			 	last[(*ptrCount)++]=RUNA;
			 	num=num>>1;
		 	    }
			  if(temp%2!=0)
		 	    {	
				last[(*ptrCount)++]=RUNB;
				num=num>>1;
		 	    }
			}
		}
  		if(num==1) break;
	}
}
UInt32 RunARunB(UChar *last ,UInt32 fileSize,EState* s)
{
UChar* ptrArr = (UChar* )s->arr2;
UInt32 i,j;
UInt32 count,arrCount=0;
//for(i=0;i<file_size;i++)
//printf("%d ",last[i]);
//printf("\n");
for(i=0;i<fileSize;)
   {	
	count=0;
	if(last[i]==0)
	{
		for(j=0;j<fileSize-i;j++)
		{
	 		if(last[i]==last[i+j])
	 		count++;
			else break;
		}
			RunAB(ptrArr,count,&arrCount);
	 		i=i+count;
	}
	else 
	{
		ptrArr[arrCount++]=last[i]+1;
		i++;
	}
   }
//for(i=0;i<arr_count;i++)
//printf("%d ",temp_last[i]);
return arrCount;
}
/********************************************************************/
/********************* Un Do RunARunB********************************/
/********************************************************************/
UInt32 UnDoRunARunB(UChar*  block,UChar* unrotated,UInt32 fileSize)
{
UInt32 i,count,j,k,blockCount=0,totalRunCount;
	for(i=0;i<fileSize;)
	{ 
		//count=1;
		count=0;
		totalRunCount=0;
		if(unrotated[i]==RUNA || unrotated[i]==RUNB)
		{
			for(j=0;j<fileSize-i;j++)
                        {
                                if(unrotated[i+count]==RUNA)
                                        totalRunCount=totalRunCount+(1<<count);  //2pow0
                                else if(unrotated[i+count]==RUNB)
                                        totalRunCount=totalRunCount+(2<<count);  //2pow1
                                else break;
                                count++;

                        }

			/*if(unrotated[i]==RUNA) totalRunCount=1;
			else 		       totalRunCount=2;	
			while( (unrotated[i+count]==RUNA || unrotated[i+count]==RUNB) && (i+count < fileSize) )
			{
				if(unrotated[i+count]==RUNA)
					totalRunCount = totalRunCount + (1<<count);
				else if(unrotated[i+count]==RUNB)
					totalRunCount = totalRunCount + (2<<count);
				count++;
			}*/
			for(k=0;k<totalRunCount;k++)
			block[blockCount++] = RUNA;
			i=i+count;
		}
		else
		{
			block[blockCount++]= unrotated[i]-1;
			i++;
		}
	}
return blockCount;
}		
