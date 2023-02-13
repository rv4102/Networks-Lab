#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

int main(){
    pid_t pid = fork();
    char *argv[256];
    for(int i=0; i<256; i++){
        argv[i] = (char *)malloc(sizeof(char));
    }
    strcpy(argv[0], "open");
    strcpy(argv[1], "-a");
    strcpy(argv[2], "Firefox");
    strcpy(argv[3], "A4.txt");
    argv[4] = 0;

    if(pid == 0){
        execvp(argv[0], argv);
    }

    return 0;
}