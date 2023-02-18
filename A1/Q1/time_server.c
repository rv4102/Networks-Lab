#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main() {
    int sockfd, newsockfd ; /* Socket descriptors */
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;

	char curr_time[64];		/* We will use this buffer for communication */

    // open a socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Unable to create socket!\n");
        exit(0);
    }
    printf("Socket successfully created.\n");

    // specify the server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    // bind the address to socket
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("Unable to bind address to socket!\n");
        exit(0);
    }
    printf("Socket successfully binded to address.\n");

    // listen for clients trying to connect to this server
    listen(sockfd, 5); /* this specifies that upto 5 concurrent client 
                        requests will be queued up while the system 
                        is executing the "accept" call */
    printf("Server listening ...\n");
    while(1){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) & cli_addr, &clilen);

        if(newsockfd < 0){
            perror("Unable to accept client's connect request.\n");
            exit(0);
        }
        printf("Server has connected to a client.\n");

        // get the current system time
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        size_t ret = strftime(curr_time, sizeof(curr_time), "%c", tm);

        if(ret == 0){
            perror("Unable to get current time.\n");
            exit(0);
        }

        // send this time to the client server
        if(send(newsockfd, curr_time, strlen(curr_time)+1, 0) != strlen(curr_time)+1){
            perror("Sent message differs in length from the message that was to be sent.\n");
            exit(0);
        }

        // close the client now
        if(close(newsockfd) < 0){
            perror("Unable to close socket.\n");
            exit(0);
        }
        printf("Connection to client has been closed.\n");
    }
    return 0;
}