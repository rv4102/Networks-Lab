#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <time.h>

#define BUFSIZE 64

int main(){
    int server_sock; 
    struct sockaddr_in serv_addr, cli_addr; 
    char current_time[BUFSIZE];

    // Create socket file descriptor 
    server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if ( server_sock < 0 ) { 
        perror("socket() failed.\n"); 
        exit(-1); 
    } 

    memset(&serv_addr, 0, sizeof(serv_addr)); 
    memset(&cli_addr, 0, sizeof(cli_addr)); 
      
    serv_addr.sin_family    = AF_INET; 
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    serv_addr.sin_port = htons(20000); 
      
    // Bind the socket with the server address 
    if ( bind(server_sock, (const struct sockaddr *)&serv_addr,  
            sizeof(serv_addr)) < 0 ) 
    { 
        perror("bind() failed.\n"); 
        exit(-1); 
    } 
    
    printf("\nServer Running....\n");

    socklen_t len = sizeof(cli_addr);
    int n = recvfrom(server_sock, (char *)current_time, BUFSIZE, 0, 
			( struct sockaddr *) &cli_addr, &len);

    // get the current system time
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    size_t ret = strftime(current_time, sizeof(current_time), "%c", tm);
    if(ret == 0){
        perror("strftime() failed.\n");
        exit(-1);
    }
    
    if(sendto(server_sock, (const char *)current_time, strlen(current_time)+1, 0, 
        (const struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
    {
        perror("sendto() failed.\n");
        exit(-1);
    }

    return 0;
}