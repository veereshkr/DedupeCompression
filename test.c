#include <stdio.h>
int main(int argc, char** argv){
unsigned int n = 40231841U;
int result = 0;
printf("res: %d\n", n>>(32-(result+1)) & 1);
while(((n>>(32-(result+1))) & 1) == 0){
      result++;
    }
printf("result: %d\n", result);
}
