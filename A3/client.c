#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

#define MSG_SIZE 500

int main(int argc, char **argv) {
	int sockfd;
    int lb_serv_port;
	struct sockaddr_in serv_addr;

    if( argc < 2 ){
        perror("Insufficient arguments passed.\n");
        exit(1);
    }
    sscanf(argv[1], "%d", &lb_serv_port);

    int i;
	char time[MSG_SIZE];

    // open a socket
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket() failed.\n");
        exit(1);
    }

    // specify the address of the load balancer
    serv_addr.sin_family = AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr); // assuming our server runs on our own machine
	serv_addr.sin_port	= htons(lb_serv_port);

    // connect to the load balancer
    if( (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0 ) {
		perror("connect() failed.\n");
		exit(1);
	}
    printf("Connection to load balancer has been established.\n");

    // receive the time from the server upon successful connection
    for(i=0; i < MSG_SIZE; i++) 
        time[i] = '\0';

    if( recv(sockfd, time, MSG_SIZE, 0) < 0 ){
        perror("recv() failed.\n");
        exit(1);
    }

    // print the time received from the server
    printf("Time as received from server:\n");
    printf("%s\n", time);

    // close the connection
    if( close(sockfd) < 0 ){
        perror("close() failed.\n");
        exit(1);
    }
    printf("Connection to server has been closed.\n");

    return 0;
}