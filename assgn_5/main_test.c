// #include "mysocket.h"
#include "queue.h"

int main(){
    char buf[100];
    for(int i = 0;i<100;i++) buf[i] = '\0';
    for(int i = 0;i<12;i++) buf[i] = 'a' + i;
    
    // struct Qeueu *q = createQueue();
    int n = my_send(1, buf, strlen(buf), 0);
    // int len = strlen(buf);
    // char *msg = (char *)malloc(sizeof(char) * (MAX_MSG_SIZE+2));
    // for(int i=0; i<len; i++){
    //     msg[i] = buf[i];
    // }

    // msg[len] = '\r';
    // msg[len+1] = '\n';
    printf("Done\n");
    printf("%d",n);

}