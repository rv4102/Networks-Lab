#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define MSG_SIZE 500
#define RANGE 100

int main(int argc, char **argv) {
    int sockfd, newsockfd; /* Socket descriptors */
    int lb_cli_port;
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;

	char msg[MSG_SIZE];		/* We will use this buffer for communication */

    if( argc < 2 ){
        printf("Insufficient arguments provided.\n");
        exit(1);
    }
    sscanf(argv[1], "%d", &lb_cli_port);

    // open a socket
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket() failed.\n");
        exit(1);
    }

    // specify the server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(lb_cli_port);

    // bind the address to socket
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("bind() failed.\n");
        exit(1);
    }

    // listen for clients trying to connect to this server
    listen(sockfd, 5); /* this specifies that upto 5 concurrent client 
                        requests will be queued up while the system 
                        is executing the "accept" call */
    printf("Server listening ...\n");

    srand(time(NULL)+lb_cli_port);   // change seed value dynamically
    while(1){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) & cli_addr, &clilen);

        if(newsockfd < 0){
            perror("accept() failed.\n");
            exit(1);
        }
        printf("Connected. ");

        // receive a message from load balancer
        if( recv(newsockfd, msg, MSG_SIZE, 0) < 0 ){
            perror("recv() failed.\n"); 
        }

        if( strcmp(msg, "Send Load") == 0 ){
            int r = rand() % RANGE + 1; // rand() generates a random number b/w 0-99 (inclusive)
            sprintf(msg, "%d", r);
            printf("Load sent: %d . ", r);
        }
        else if( strcmp(msg, "Send Time") == 0 ){
            // get the current system time
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            size_t ret = strftime(msg, sizeof(msg), "%c", tm);
            if( ret == 0 ){
                perror("Unable to get system time.\n");
                exit(1);
            }
        } 
        else{
            perror("Invalid message received.\n");
            exit(1);
        }

        // send this to the load balancer
        if( send(newsockfd, msg, strlen(msg)+1, 0) < 0 ){
            perror("send() failed.\n");
            exit(1);
        }

        // close the client now
        if(close(newsockfd) < 0){
            perror("close() failed.\n");
            exit(1);
        }
        printf("Closed.\n");
    }
    return 0;
}