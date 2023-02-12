// A Simple UDP Server that sends a HELLO message
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <time.h>

  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Create socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    printf("\nServer Running....\n");
  
    int n = -1; 
    socklen_t len;
    char buffer[100]; 
    len = sizeof(cliaddr);
 
    //establishing connenction with the client
    recvfrom(sockfd, (char *)buffer, 100, 0, (struct sockaddr *) &cliaddr, &len);
    time_t tm;
	time(&tm);
	strcpy(buffer,ctime(&tm));

    //sending date and time
    n = sendto(sockfd, (const char *)buffer, strlen(buffer) + 1, 0, 
		(const struct sockaddr *) &cliaddr, len); 

    printf("Date and time sent\n");
    close(sockfd);
    printf("Server closed\n");
    return 0; 
} 