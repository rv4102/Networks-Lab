// A Simple Client Implementation
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/poll.h>
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    inet_aton("127.0.0.1", &servaddr.sin_addr); 
    int nfd = 1;
    int n;
    socklen_t len;
    char hello[] = "hello";

    // to establish the udp connection from the client to server
    sendto(sockfd, (const char *)hello, strlen(hello), 0, 
		(const struct sockaddr *) &servaddr, sizeof(servaddr)); 

    char buf[100];
    for(int i = 0;i<100;i++) buf[i] = '\0';
    struct pollfd fd[1];
    int i = 0;
    fd[0].fd = sockfd;
    fd[0].events = POLLIN;
    int timeout = 3*1000;

    while(i<5){
        int rc = poll(fd, nfd, timeout);            //polling
        if(rc<0){
            printf("Error!\n");
            break;
        }
        else if(rc>0){
            int len  = sizeof(servaddr);
            recvfrom(sockfd, (char *)buf, 100, 0, (struct sockaddr *) &servaddr, &len);
            printf("%s", buf);
            break;
        }
        i++;
    }
    if(i==5){
        printf("Timeout!\n");
    }
           
    close(sockfd); 
    return 0; 
} 