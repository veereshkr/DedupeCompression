#include <stdio.h>

int main(int argc, char** argv){
FILE* f1 = fopen("fastDiv.h", "wb");
if(f1 == NULL){
    printf("fastDiv.h can be opened for writing. Exiting...\n");
}

unsigned long long int maxNum = 1, result = 0; maxNum <<= 63;
unsigned int maxDivisor = 1048576; //1MB
//unsigned int maxDivisor = 1024; //1MB
int i = 0;

fprintf(f1, "/* Generated by generateFastDiv.c */\n");
fprintf(f1, "#include <stdio.h>\n");
fprintf(f1, "//maxNum is %llu\n", maxNum);
fprintf(f1, "#define maxDivisor %u\n", maxDivisor);
fprintf(f1, "unsigned long long int fastDivMap[maxDivisor] = {\n");

for(i = 0; i < maxDivisor-1; i++){
    result = maxNum / (i+1);
    fprintf(f1, "\t\t\t\t %lluULL, \t//%d \n", result, i+1);
}
    result = maxNum / (i+1);
    fprintf(f1, "\t\t\t\t %lluULL \t//%d \n", result, i+1);


fprintf(f1, "\t\t\t};\n");
fclose(f1);
}
