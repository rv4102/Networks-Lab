#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <poll.h>

#define MSG_SIZE 500
#define MAX_TIME 5

int main(int argc, char **argv) {
    int lb_cfacing_sockfd, lb_cfacing_newsockfd, lb_sfacing_sockfd; 
    int lb_cfacing_port, serv1_port, serv2_port, poll_res;
	socklen_t clilen;
	struct sockaddr_in serv1_addr, serv2_addr, lb_cfacing_addr, cli_addr;
    struct pollfd fd;
    int l1, l2; // load1 and load2

	char msg[MSG_SIZE], load_s1[MSG_SIZE], load_s2[MSG_SIZE];		/* We will use this buffer for communication */

    if( argc < 3 ){
        printf("Insufficient arguments provided.\n");
        exit(1);
    }
    sscanf(argv[1], "%d", &lb_cfacing_port);
    sscanf(argv[2], "%d", &serv1_port);
    sscanf(argv[3], "%d", &serv2_port);

    // open a socket
    if( (lb_cfacing_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket() failed.\n");
        exit(1);
    }
    // if( (lb_sfacing_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
    //     perror("socket() failed.\n");
    //     exit(1);
    // }

    // specify the server address
    lb_cfacing_addr.sin_family = AF_INET;
    lb_cfacing_addr.sin_addr.s_addr = INADDR_ANY;
    lb_cfacing_addr.sin_port = htons(lb_cfacing_port);

    // specify the address of S1 server 
    serv1_addr.sin_family = AF_INET;
	inet_aton("127.0.0.1", &serv1_addr.sin_addr); // assuming our server runs on our own machine
	serv1_addr.sin_port	= htons(serv1_port);

    // specify the address of S2 server
    serv2_addr.sin_family = AF_INET;
	inet_aton("127.0.0.1", &serv2_addr.sin_addr); // assuming our server runs on our own machine
	serv2_addr.sin_port	= htons(serv2_port);

    // bind the address to socket
    if( bind(lb_cfacing_sockfd, (struct sockaddr *) &lb_cfacing_addr, sizeof(lb_cfacing_addr)) < 0 ){
        perror("bind() failed.\n");
        exit(1);
    }

    // listen for clients trying to connect to this server
    listen(lb_cfacing_sockfd, 5); /* this specifies that upto 5 concurrent client 
                        requests will be queued up while the system 
                        is executing the "accept" call */
    printf("Load Balancer listening ...\n");

    fd.fd = lb_cfacing_sockfd;
    fd.events = POLLIN;

    time_t start, end;
    double elapsed;  // seconds
    // infinite loop
    while(1){
        msg[0] = '\0';
        strcpy(msg, "Send Load");
        // connect to S1
        if( (lb_sfacing_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
            perror("socket() failed.\n");
            exit(1);
        }
        if( connect(lb_sfacing_sockfd, (struct sockaddr *) &serv1_addr, sizeof(serv1_addr)) < 0){
            perror("connect() error.\n");
            exit(1);
        }
        // send message
        if( send(lb_sfacing_sockfd, msg, MSG_SIZE, 0) < 0 ){
            perror("send() error.\n");
            exit(1);
        }
        // receive load from S1
        if( recv(lb_sfacing_sockfd, load_s1, MSG_SIZE, 0) < 0 ){
            perror("recv() error.\n");
            exit(1);
        }
        close(lb_sfacing_sockfd);
        printf("Load received from IP:%s, Port Num:%d - %s\n", inet_ntoa(serv1_addr.sin_addr), serv1_port, load_s1);

        // connect to S2
        if( (lb_sfacing_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
            perror("socket() failed.\n");
            exit(1);
        }
        if( connect(lb_sfacing_sockfd, (struct sockaddr *) &serv2_addr, sizeof(serv2_addr)) < 0){
            perror("connect() error.\n");
            exit(1);
        }
        // send message
        if( send(lb_sfacing_sockfd, msg, MSG_SIZE, 0) < 0 ){
            perror("send() error.\n");
            exit(1);
        }
        // receive load from S2
        if( recv(lb_sfacing_sockfd, load_s2, MSG_SIZE, 0) < 0 ){
            perror("recv() error.\n");
            exit(1);
        }
        close(lb_sfacing_sockfd);
        printf("Load received from IP:%s, Port Num:%d - %s\n", inet_ntoa(serv1_addr.sin_addr), serv2_port, load_s2);

        // compare the two loads
        sscanf(load_s1, "%d", &l1);
        sscanf(load_s2, "%d", &l2);

        start = time(NULL);
        while(1){
            end = time(NULL);
            elapsed = difftime(end, start);
            if (elapsed >= MAX_TIME * 1.0 /* seconds */)
                break; // break when time elapsed exceeds 5 seconds

            // otherwise continue with normal execution
            // poll the lb_cfacing_sockfd
            poll_res = poll(&fd, 1, MAX_TIME * 1000); // change
            if(poll_res == -1){
                perror("poll() failed.\n");
                exit(1);
            }
            else if(poll_res == 0){
                break;
            }
            else{ // we have a client to be serviced
                clilen = sizeof(cli_addr);
                if( (lb_cfacing_newsockfd = accept(lb_cfacing_sockfd, (struct sockaddr *) &cli_addr, &clilen)) < 0 ){
                    perror("accept() failed.\n");
                    exit(1);
                }

                if(fork() == 0){ // the child process handles its request
                    close(lb_cfacing_sockfd);

                    if( (lb_sfacing_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
                        perror("socket() failed.\n");
                        exit(1);
                    }

                    if(l1 < l2){ // now make connection to server with serv_addr as its address and send "Send Time"
                        if( connect(lb_sfacing_sockfd, (struct sockaddr *) &serv1_addr, sizeof(serv1_addr)) < 0 ){
                            perror("connect() error.\n");
                            exit(1);
                        }
                    }
                    else{
                        if( connect(lb_sfacing_sockfd, (struct sockaddr *) &serv2_addr, sizeof(serv2_addr)) < 0 ){
                            perror("connect() error.\n");
                            exit(1);
                        }
                    }
                    msg[0] = '\0';
                    // send the message
                    strcpy(msg, "Send Time");
                    if( send(lb_sfacing_sockfd, msg, MSG_SIZE, 0) < 0 ){
                        perror("send() error.\n");
                        exit(1);
                    }
                    // receive the time from server
                    if( recv(lb_sfacing_sockfd, msg, MSG_SIZE, 0) < 0 ){
                        perror("recv() error.\n");
                        exit(1);
                    }
                    close(lb_sfacing_sockfd);

                    // send this time to the client requesting service
                    if( send(lb_cfacing_newsockfd, msg, MSG_SIZE, 0) < 0 ){
                        perror("send() error.\n");
                        exit(1);
                    }
                    close(lb_cfacing_newsockfd);
                    exit(0);
                }
                close(lb_cfacing_newsockfd);
            }

            
        }

    }
    
    return 0;
}