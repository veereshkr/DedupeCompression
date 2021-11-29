#include <stdio.h>
int main(int argc, char **argv){
if(argc != 3){
        printf("Usage: %s -f filename\n", argv[0]); exit(0);
}
if(strcmp(argv[1], "-f") != 0){
        printf("Usage: %s -f filename\n", argv[0]); exit(0);
}
int i = 0;
char cwd[1024];
if(getcwd(cwd, sizeof(cwd)) != NULL){
        printf("Current Working Directory: %s\n", cwd);
} else
{
        printf("Current Working Directory cannot be obtained. Exiting...\n");
        exit(0);
}
char filename[1024];
sprintf(filename,"%s/%s", cwd, argv[2]);
FILE *fp = fopen(filename, "rb");
int freq[256];
int count = 0, uniqueCount = 0;
int sum = 0, diff = 0, maxDiff = 0, prev = 0, pos = 0, s = 0;
int c128 = 0;
for(i = 0 ; i < 256; i++)
	freq[i] = 0;
if(fp != NULL){
	printf("File open success");
	int c;
	while((c = getc(fp)) != EOF){
		if(c == 0){
			diff = count - prev - 1;
			if(diff > 20){
				//printf("diff(20): %d\n", diff);
			} else {
				//printf("diff: %d\n", diff);
			}
			prev = count;
			if(diff > maxDiff){maxDiff = diff;};
		}
		//printf("#%c#%d\n",c,c);
		printf("%d ",c); s += c+1;
                if(c >= 128){ c128++; }
		//if(c == 0){
		//	printf("pos: %d(%d) ",pos, freq[c]+1);
		//}
		//if(count % 10000 == 1){printf("\n");}
		freq[c]++;
		count++;
		pos++;
                //printf("%d count: %d\n", c, count);
	}
}
printf("Total number of characters: %d\n",count);
for(i = 0 ; i < 256 ; i++){
	sum = sum + freq[i];	
	if(freq[i] != 0){uniqueCount++;}
	printf("Total number of %d(%c): %d\n", i,i, freq[i]);
}
printf("s is: %d\n", s);
printf("Sum is: %d\n", sum);
printf("Unique Count is: %d\n", uniqueCount);
printf("maxDiff for 0 is: %d\n", maxDiff);
printf("c128 is: %d\n", c128);
if(fp != NULL)
	fclose(fp);
}
