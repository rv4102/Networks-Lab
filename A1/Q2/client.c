#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define RET_SIZE 30
#define BUF_SIZE 200

int isoperator(char c){
    return (c=='+' || c=='-' || c=='*' || c=='/' || c=='(' || c==')');
}

int min(a, b){
    if(a <= b) 
        return a;
    return b;
}

int main() {
	int sockfd ;
	struct sockaddr_in serv_addr;

    int i, j;
    size_t op_found=0, n=0, len;
	char *inp = NULL, res[RET_SIZE], packet[BUF_SIZE];

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

    while(1){
        // ask for the arithmetic expression from the user
        printf("Enter the arithmetic expression you want to evaluate.\n");

        // take the input all at once 
        len = getline(&inp, &n, stdin);
        if(len <= 3 && atoi(inp) == -1){ 
            // check for -1
            if(send(sockfd, inp, strlen(inp)+1, 0) != strlen(inp)+1){
                perror("Unable to send the data.\n");
                exit(0);
            }
            break;
        }
        
        // create a packet of 200 characters
        for(i=0; i<len; i+=BUF_SIZE){
            for(j=0; j<BUF_SIZE; j++){
                if(j+i < len){
                    if(inp[j+i] != '\n')
                        packet[j] = inp[j+i];
                    else
                        packet[j] = '\0';
                }
            }

            // send this packet to the server
            if(send(sockfd, packet, BUF_SIZE, 0) != BUF_SIZE){
                perror("Unable to send the data.\n");
                exit(0);
            }
        }

        // receive the output after all packets are sent
        if(recv(sockfd, res, RET_SIZE, 0) == 0){
            perror("Unable to receive the result from the server.\n");
            exit(0);
        }

        printf("The result of this expression : %s\n\n", res);
    }

    // close the connection
    if(close(sockfd) < 0){
        perror("Unable to close socket.\n");
        exit(0);
    }
    printf("Connection to server has been closed.\n");

    return 0;
}