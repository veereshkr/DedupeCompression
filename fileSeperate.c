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
FILE* fp1 = fopen("/home/neeraj/code/ROCK/part1.txt", "wb");
FILE* fp2 = fopen("/home/neeraj/code/ROCK/part2.txt", "wb");
int freq[256], sm = 0;
int count = 0, uniqueCount = 0;
int sum = 0, diff = 0, maxDiff = 0, prev = 0, pos = 0;
for(i = 0 ; i < 256; i++)
	freq[i] = 0;
if(fp != NULL){
	printf("File open success");
	int c;
	while((c = getc(fp)) != EOF){
                sm += c;
		//if(count >= 64*1024){
                if(c < 1){
		//if(c == 2 || c == 3 || c == 4 || c == 5){
			fputc(c, fp1);
		} else { fputc(c-1, fp2); }
		freq[c]++;
		count++;
		pos++;
	}
}
printf("sm: %d\n", sm);
printf("Total number of characters: %d\n",count);
for(i = 0 ; i < 256 ; i++){
	sum = sum + freq[i];	
	if(freq[i] != 0){uniqueCount++;}
	printf("Total number of %d(%c): %d\n", i,i, freq[i]);
}
printf("Sum is: %d\n", sum);
printf("Unique Count is: %d\n", uniqueCount);
printf("maxDiff for 0 is: %d\n", maxDiff);
if(fp != NULL)
	fclose(fp);
fclose(fp1); fclose(fp2);
}
