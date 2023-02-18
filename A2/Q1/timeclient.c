#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <poll.h>

#define MAX_TRIES 5
#define MAX_TIME 3000
#define BUFSIZE 64

int main(){
    int client_sock, poll_res; 
    struct sockaddr_in serv_addr;
    struct pollfd fd;
    socklen_t len;
    size_t i;
    char time[BUFSIZE] = "hi";

    // Creating socket file descriptor 
    client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if ( client_sock < 0 ) { 
        perror("socket() failed.\n"); 
        exit(-1); 
    } 
  
    memset(&serv_addr, 0, sizeof(serv_addr)); 
      
    // Server information 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(20000); 
    inet_aton("127.0.0.1", &serv_addr.sin_addr); 

    fd.fd = client_sock;
    fd.events = POLLIN;

    if(sendto(client_sock, (const char *)time, BUFSIZE, 0, 
        (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("sendto() failed.\n");
        exit(-1);
    }
    for(i = 0;i<BUFSIZE;i++) time[i] = '\0'; 
    for(i=0; i<MAX_TRIES; i++){
        poll_res = poll(&fd, 1, MAX_TIME);
        if(poll_res == 0){
            continue;
        }
        else if(poll_res == -1){
            perror("poll() failed.\n");
            exit(-1);
        }
        else{
            // implies (fd.revents & POLLIN) != 0
            len = sizeof(serv_addr);
            if(recvfrom(client_sock, (char *)time, BUFSIZE, 0, 
                    (struct sockaddr *)&serv_addr, &len) < 0)
            {
                perror("recvfrom() failed.\n");
                exit(-1);
            }
            printf("Time: %s\n", time);
            break;
        }
    }
    if(i==5)printf("timeout\n" );

    return 0;
}
