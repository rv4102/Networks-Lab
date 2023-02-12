#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

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

void split_command(char *msg, size_t len, char **split){
    int i;
    size_t cmd_ptr=0, idx=0;
    for(i=0; i<len; i++){
        if(msg[i] != ' '){
            split[cmd_ptr][idx++] = msg[i];
        }
        else if(msg[i] == '\n'){
            continue;
        }
        else{
            split[cmd_ptr][idx++] = '\0';
            cmd_ptr++;
            idx = 0;
        }
    }
}

void parse_ip(char *url, char *ip, int *port){
    size_t ptr=7, idx=0; // "http://" uses 7 characters
    while(isdigit(url[ptr]) || url[ptr] == '.'){
        ip[idx++] = url[ptr];
    }

    ptr=strlen(url)-1;
    if( !isdigit(url[ptr]) )
        return;
    
    while( isdigit(url[ptr]) ) ptr--;
    ptr++;

    char port_str[10];
    strcpy(port_str, url+ptr);

    // string to integer
    sscanf(port_str, "%d", port);
}

void frame_as_request(char **cmd_list, char *http_request){
    size_t ptr_1=0, ptr_2=0;
    http_request[0] = '\0';
    strcat(http_request, "GET ");

    // get the url
    ptr_1 = 7;
    while(cmd_list[1][ptr_1] != '/') ptr_1++;
    while(cmd_list[1][ptr_1] != '\0'){
        http_request[ptr_2++] = cmd_list[ptr_1++];
    }

    // HTTP/1.1
    ptr_1 = 0;
    strcat(http_request, " HTTP/1.1\n");
    

    http_request[ptr_2++] = '\n';

    // User-Agent
    user_agent[]
}

int main(){
    int sockfd;
    struct sockaddr_in serv_addr;
    size_t n=0;
    char msg[MAXRETSIZE], **cmd_list, ip[16];
    int port=80; // default is 80
    cmd_list = (char **)malloc(sizeof(char *) * 3);
    for(int i=0; i<3; i++){
        cmd_list[i] = (char *)malloc(sizeof(char) * MAXRETSIZE);
    }

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("socket() failed.\n");
		exit(-1);
	}

    while(1){
        printf("MyOwnBrowser> ");
        getline(&msg, &n, stdin);
        split_command(msg, strlen(msg), cmd_list);
        if( strcmp(cmd_list[0], "GET") ){
            // parse the IP address
            parse_ip(cmd_list[1], ip, &port);

            // connect to the server
            serv_addr.sin_family = AF_INET;
            inet_aton(ip, &serv_addr.sin_addr);
            serv_addr.sin_port = htons(port);

            if( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ){
                perror("connect() failed.\n");
                exit(1);
            }

            // create an http request


        }
        else if( strcmp(cmd_list[0], "PUT") ){

        }
        else if( strcmp(cmd_list[0], "QUIT") ){
            printf("Quitting now.\nBye.\n");
            break;
        }
        else{
            printf("Invalid command. Try again.\n");
        }
    }

    return 0;
}