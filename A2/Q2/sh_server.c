#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#define _POSIX_SOURCE

#define BUFSIZE 50
#define MAXRETSIZE 30045 // large sized buffer to accomodate dir command output

void send_as_packet(char* message, int cli_sock){
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
        if(send(cli_sock, packet, BUFSIZE, 0) < 0){
            perror("send() error.\n");
            exit(-1);
        }
    }
}

// receive data in packets and return the length of received data
size_t receive_as_packet(char *command, int cli_sock){
    size_t end=0, i, ptr=0;
    char packet[BUFSIZE];
    while(!end){
        if( recv(cli_sock, packet, BUFSIZE, 0) < 0 ){
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

int valid_username(FILE* fp, char *username){
    // check username against list of usernames
    char line[BUFSIZE];

    while (fgets(line, sizeof(line), fp) != NULL){   
        line[strlen(line)-1] = '\0';
        if(strcmp(line, username) == 0){
            return 1;
        }
    }
    return 0;
}

int main(){
    int server_sock, cli_sock;
    socklen_t cli_addr_len;
    struct sockaddr_in serv_addr, cli_addr;
    char msg[BUFSIZE];
    char text[MAXRETSIZE]="\0", cmd[5], args[200]="\0";

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock < 0){
        perror("socket() failed.\n");
        exit(-1);
    }

    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(20000);

    if (bind(server_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("bind() failed.\n");
		exit(-1);
	}

    listen(server_sock, 5);/* This specifies that up to 5 concurrent client
			      requests will be queued up while the system is
			      executing the "accept" system call below.
			   */

    while(1){
        cli_addr_len = sizeof(cli_addr);
        cli_sock = accept(server_sock, (struct sockaddr *)&cli_addr, &cli_addr_len);
        printf("Connected to a client.\n");

        // check if users.txt exists
        FILE* fp;
        fp = fopen("users.txt" , "r");
        if(fp == NULL){
            perror("users.txt not in the same directory.\n");
            exit(-1);
        }

        if(cli_sock < 0){
            perror("accept() error.\n");
            exit(-1);
        }

        // go to the child process after connection has been made
        if(fork() == 0){
            // close the original socket through which connection was made
            close(server_sock);

            strcpy(msg, "LOGIN: ");
            if( send(cli_sock, msg, strlen(msg)+1, 0) < 0 ){
                perror("send() error.\n");
                exit(-1);
            }

            // get username
            if( recv(cli_sock, msg, BUFSIZE, 0) < 0 ){
                perror("recv() error.\n");
                exit(-1);
            }

            // check username is correct or not
            if( !valid_username(fp, msg) ) {
                strcpy(msg, "NOT-FOUND");
                if( send(cli_sock, msg, strlen(msg)+1, 0) < 0 ){
                    perror("send() error.\n");
                    exit(-1);
                }
            }
            else {
                strcpy(msg, "FOUND");
                if( send(cli_sock, msg, strlen(msg)+1, 0) < 0 ){
                    perror("send() error.\n");
                    exit(-1);
                }

                // accept the commands that the user sends and execute them
                while(1){
                    strcpy(text, "\0");
                    strcpy(cmd, "\0");
                    strcpy(args, "\0");

                    // get the entire command from client
                    receive_as_packet(text, cli_sock);

                    // split this input into cmd and args
                    size_t nargs = sscanf(text, "%s %s", cmd, args);
                    strcpy(text, "\0"); // reset it

                    if(strcmp(cmd, "pwd") == 0){
                        // get the return value of getcwd in a string
                        getcwd(text, MAXRETSIZE);

                        send_as_packet(text, cli_sock);
                    }
                    else if(strcmp(cmd, "dir") == 0){
                        DIR *dir;
                        struct dirent *de;  // Pointer for directory entry 
                        
                        // dir command accepts a path as an argument
                        // if there is an argument, open that directory
                        if(nargs > 1)
                            dir = opendir(args);
                        // else open current directory
                        else
                            dir = opendir("./");

                        // traverse this directory
                        if(dir == NULL)
                            strcpy(text, "####");
                        else{
                            while((de = readdir(dir)) != NULL){
                                strcat(text, de->d_name);
                                strcat(text, " ");
                            }
                        }
                        closedir(dir);

                        // send to the client
                        send_as_packet(text, cli_sock);
                    }
                    else if(strcmp(cmd, "cd") == 0){
                        // change the working directory
                        if(nargs > 1){   
                            if(chdir(args) == 0){
                                // getcwd(text, MAXRETSIZE);
                                if( send(cli_sock, text, 1, 0) < 0 ){
                                    perror("send() error.\n");
                                    exit(-1);
                                }
                            }
                            else{
                                strcpy(text, "####");
                                send_as_packet(text, cli_sock);
                            }
                        }
                        else{
                            chdir(getenv("HOME"));
                            // getcwd(text, MAXRETSIZE);
                            if( send(cli_sock, text, 1, 0) < 0){
                                perror("send() error.\n");
                                exit(-1);
                            }
                        }
                    }
                    else if(strcmp(cmd, "exit") == 0){
                        break;
                    }
                    else{
                        strcpy(text, "$$$$");
                        if(send(cli_sock, text, strlen(text)+1, 0) < 0){
                            perror("send() error.\n");
                            exit(-1);
                        }
                    }
                }
            }

            if( close(cli_sock) < 0 ){
                perror("close() failed.\n");
                exit(-1);
            }
            exit(0);
        }
        if( close(cli_sock) < 0 ){
            perror("close() failed.\n");
            exit(-1);
        }
    }

    return 0;
}