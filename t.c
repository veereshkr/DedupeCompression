#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

long long timeval_diff(struct timeval *difference, struct timeval *end_time, struct timeval *start_time){
struct timeval temp_diff;

if(difference==NULL){
    difference=&temp_diff;
}

difference->tv_sec =end_time->tv_sec -start_time->tv_sec ;
difference->tv_usec=end_time->tv_usec-start_time->tv_usec;

/* Using while instead of if below makes the code slightly more robust. */

while(difference->tv_usec<0) { 
    difference->tv_usec+=1000000;
    difference->tv_sec -=1;
}

return 1000000LL*difference->tv_sec+ difference->tv_usec;

} /* timeval_diff() */

int main(void){
struct timeval earlier;
struct timeval later;
struct timeval interval;

/*There are two ways to call timeval_diff.  The first way simply returns the time difference as microseconds, a single integer. */

if(gettimeofday(&earlier,NULL)){
    perror("first gettimeofday()"); exit(1);
}
printf("current time is %ld seconds, %ld microseconds\n", earlier.tv_sec, earlier.tv_usec);

sleep(3);

if(gettimeofday(&later,NULL)){
    perror("second gettimeofday()"); exit(1);
}
printf("current time is %ld seconds, %ld microseconds\n", later.tv_sec, later.tv_usec);

printf("difference is %lld microseconds\n", timeval_diff(NULL,&later,&earlier));

/*The second way to call timeval_diff returns the difference broken down as seconds and microseconds. */

if(gettimeofday(&earlier,NULL)){
    perror("third gettimeofday()"); exit(1);
}

sleep(4);

if(gettimeofday(&later,NULL)){
    perror("fourth gettimeofday()"); exit(1);
}

timeval_diff(&interval,&later,&earlier);

printf("difference is %ld seconds, %ld microseconds\n", interval.tv_sec, interval.tv_usec);

/* Well, actually there's a third way.  You can actually combine the first two ways.*/

if(gettimeofday(&earlier,NULL)){
    perror("fifth gettimeofday()"); exit(1);
}

sleep(5);

if(gettimeofday(&later,NULL)){
    perror("sixth gettimeofday()"); exit(1);
}

printf("difference is %lld microseconds", timeval_diff(&interval,&later,&earlier));

printf(" (%ld seconds, %ld microseconds)\n", interval.tv_sec, interval.tv_usec);

return 0;

} /* main() */
