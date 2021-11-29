#include <stdio.h>
#include "gmp.h"
#include "ncr.h"

int main(int argc, char** argv){
FILE* f1 = fopen("floatValues.h", "wb");
if(f1 == NULL){
    printf("floatValues.h can be opened for writing. Exiting...\n");
}

int max = 500000;
int i = 0;
float f = 0;

fprintf(f1, "/* Generated by generateFloat.c */\n");
fprintf(f1, "#include <stdio.h>\n");
fprintf(f1, "#define FLOAT_MAX %d\n", max);
fprintf(f1, "double floatMap[FLOAT_MAX] = {\n");

for(i = 1; i <= max; i++){
    //f = (float)(2147483648)/ (float)i;
    f = (float)(1)/ (float)i;
    if(i == max){
        fprintf(f1, "\t\t\t\t %f \t// (1/%d)\n", f, i);
    } else {
        fprintf(f1, "\t\t\t\t %f, \t// (1/%d)\n", f, i);
    }
}

fprintf(f1, "\t\t\t};\n");
fclose(f1);
}
