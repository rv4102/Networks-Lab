#include <stdio.h>
#include <time.h>

int main(){
    time_t start_t, end_t;
    // int diff;
    start_t = time(NULL);
    printf("%s\n", ctime(&start_t));
    end_t = start_t + 5;
    printf("%s", ctime(&end_t));
    int diff = difftime(end_t, start_t);
    printf("%d\n", diff);
}