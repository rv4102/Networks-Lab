#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 50
#define MAXRETSIZE 30045 // large sized buffer to accomodate dir command output

void send_as_packet(char* message, int sockfd){
    size_t i, j, len=strlen(message);
    char packet[BUFSIZE];

    // create a packet
    for(i=0; i<len; i+=BUFSIZE){
        for(j=0; j<BUFSIZE; j++){
            if(i+j < len)
                packet[j] = message[j+i];
            else
                packet[j] = '\0';
        }

        // send this packet to the server
        if(send(sockfd, packet, BUFSIZE, 0) < 0){
            perror("send() error.\n");
            exit(-1);
        }
    }
}

// receive data in packets and return the length of received data
size_t receive_as_packet(char *command, int sockfd){
    size_t end=0, i, ptr=0;
    char packet[BUFSIZE];
    while(!end){
        if( recv(sockfd, packet, BUFSIZE, 0) < 0 ){
            perror("recv() error.\n");
            exit(-1);
        }

        // check if this is the last packet
        for(i=0; i<BUFSIZE; i++){
            if(packet[i]=='\0') end=1;
        }

        // concatenate this to the final expression
        for(i=0; i<BUFSIZE; i++){
            command[ptr++] = packet[i];
        }
    }

    return strlen(command);
}

int main(){
    int sockfd ;
	struct sockaddr_in serv_addr;
    size_t n=0;
    char *msg, username[BUFSIZE];
    msg = (char *)(malloc(sizeof(char) * MAXRETSIZE));

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("socket() failed.\n");
		exit(-1);
	}

    serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

    if ( (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0 ) {
		perror("connect() failed.\n");
		exit(-1);
	}

    // receive the LOGIN string from server
    receive_as_packet(msg, sockfd);
    printf("%s", msg);

    // take the username as input from user
    scanf("%s", username);
    send_as_packet(username, sockfd);

    // receive the verification status from server
    receive_as_packet(msg, sockfd);
    if(strcmp(msg, "NOT-FOUND") == 0){
        printf("Invalid username.\n");
        close(sockfd);
    }
    // if verified, now ask for commands to be run
    else{
        fflush(stdin);
        while(1){
            msg = NULL;
            printf("%s $ ", username);
            getline(&msg, &n, stdin);
            msg[strlen(msg)-1] = '\0';

            // send this command to the server
            send_as_packet(msg, sockfd);

            if(strcmp(msg, "exit") == 0){
                break;
            }

            // receive the result from server
            receive_as_packet(msg, sockfd);

            if(strcmp(msg, "$$$$") == 0){
                printf("Invalid command.\n");
            }
            else if(strcmp(msg, "####") == 0){
                printf("Error in running command.\n");
            }
            else{
                if(strcmp(msg, "\0")) 
                    printf("%s\n", msg);
            }
        }
        if( close(sockfd) < 0 ){
            perror("close() failed.\n");
            exit(-1);
        }
    }

    return 0;
}