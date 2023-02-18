#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
	int sockfd ;
	struct sockaddr_in serv_addr;

    int i;
	char time[64];

    // open a socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Unable to open a socket!\n");
        exit(0);
    }
    printf("Socket successfully created.\n");

    // specify the address of this server
    serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr); // assuming our server runs on our own machine
	serv_addr.sin_port	= htons(20000);

    // connect to the server
    if((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
    printf("Connection to server has been established.\n");

    // receive the time from the server upon successful connection
    for(i=0; i < 64; i++) 
        time[i] = '\0';

    if(recv(sockfd, time, 64, 0) == 0){
        perror("Unable to receive message from server!\n");
        exit(0);
    }

    // print the time received from the server
    printf("Time as received from server:\n");
    printf("%s\n", time);

    // close the connection
    if(close(sockfd) < 0){
        perror("Unable to close socket.\n");
        exit(0);
    }
    printf("Connection to server has been closed.\n");

    return 0;
}