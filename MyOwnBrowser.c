#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>

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
            exit(1);
        }
    }
}

void split_command(char *msg, size_t len, char **split){
    // printf("%zu\n", len);
    int i;
    size_t cmd_ptr=0, idx=0;
    for(i=0; i<len; i++){
        if(msg[i] == '\n'){
            split[cmd_ptr][idx++] = '\0';
        }
        else if(msg[i] != ' '){
            split[cmd_ptr][idx++] = msg[i];
        }
        else{
            split[cmd_ptr][idx++] = '\0';
            cmd_ptr++;
            idx = 0;
        }
    }
}

void parse_addr(char *url, char *ip, char *port, char *fname){
    size_t ptr=7, idx=0; // "http://" uses 7 characters
    while(isdigit(url[ptr]) || url[ptr] == '.'){
        ip[idx++] = url[ptr];
        ptr++;
    }
    ip[idx] = '\0';

    ptr=strlen(url)-1;
    if( !isdigit(url[ptr]) ){
        strcpy(port, "80");
        return;
    }
    
    while( isdigit(url[ptr]) ) ptr--;
    ptr++;
    strcpy(port, url+ptr);

    while( url[ptr] != '/') ptr--;
    ptr++;

    size_t ptr1=0;
    while(url[ptr] != ':'){
        fname[ptr1++] = url[ptr];
        ptr++;
    }   
}

void get_content_type(char *url, char *type){
    char *tok;
    tok = strtok(url, ":"); // the first token has "http"
    tok = strtok(NULL, ":"); // the second token has "//ip/path/to/file.xyz"

    strcpy(tok, tok + strlen(tok)-3);

    if( strcmp(tok, "html") == 0 ){
        strcpy(type, "text/html");
    }
    else if( strcmp(tok, "pdf") == 0 ){
        strcpy(type, "application/pdf");
    }
    else if( strcmp(tok, "jpg") == 0 ){
        strcpy(type, "image/jpg");
    }
    else{
        strcpy(type, "text/*");
    }
}

// Adjust date by a number of days +/-
void DatePlusDays( struct tm* date, int days ){
    const time_t ONE_DAY = 24 * 60 * 60 ;

    // Seconds since start of epoch
    time_t date_seconds = mktime( date ) + (days * ONE_DAY) ;

    // Update caller's date
    // Use localtime because mktime converts to UTC so may change date
    *date = *localtime( &date_seconds ) ; ;
}

void get_times(char *last_modified, char *current){
    time_t t = time(NULL);
    struct tm *time_struct = localtime(&t);
    char *tmp = ctime(&t);
    strcpy(current, tmp);
    current[strlen(current)-1] = '\0';

    DatePlusDays(time_struct, -2);
    time_t new_time = mktime(time_struct);
    tmp = ctime(&new_time);
    strcpy(last_modified, tmp);
    last_modified[strlen(last_modified)-1] = '\0';
}

void frame_request(char **cmd_list, char *filetype, char *accept_lang, char *ip, char *port,
    char *content_language, char *content_length, char *http_request) {

    size_t ptr_1=0, ptr_2=0;
    char current_time[40], last_modified_time[40];

    get_times(last_modified_time, current_time);

    strcpy(http_request, cmd_list[0]);
    strcat(http_request, " ");

    // get the url
    ptr_1 = 7;
    while(cmd_list[1][ptr_1] != '/') ptr_1++;
    ptr_2 = strlen(http_request);
    while(cmd_list[1][ptr_1] != ':' && cmd_list[1][ptr_1] != '\0') {
        http_request[ptr_2++] = cmd_list[1][ptr_1];
        ptr_1++;
    }
    http_request[ptr_2] = '\0';

    strcat(http_request, " HTTP/1.1\r\n");
    strcat(http_request, "Host: ");
    strcat(http_request, ip);
    strcat(http_request, ": ");
    strcat(http_request, port);
    strcat(http_request, "\r\n");
    strcat(http_request, "Connection: close\r\n");
    strcat(http_request, "Date: ");
    strcat(http_request, current_time);
    strcat(http_request, "\r\n");
    strcat(http_request, "Accept: ");
    strcat(http_request, filetype);
    strcat(http_request, "\r\n");
    strcat(http_request, "Accept-Language: ");
    strcat(http_request, accept_lang);
    strcat(http_request, "\r\n");
    strcat(http_request, "If-Modified-Since: ");
    strcat(http_request, last_modified_time);
    strcat(http_request, "\r\n");
    strcat(http_request, "Content-language: ");
    strcat(http_request, content_language);
    strcat(http_request, "\r\n");
    strcat(http_request, "Content-length: ");
    strcat(http_request, content_length);
    strcat(http_request, "\r\n");
    strcat(http_request, "Content-type: ");
    strcat(http_request, filetype);
    strcat(http_request, "\r\n");

    return;
}

int check_header(char *header, char *command){
    char ver[20], status[10], response[10], *tok;
 
    tok = strtok(header, " ");
    strcpy(ver, tok);
    tok = strtok(NULL, " ");
    strcpy(status, " ");
    tok = strtok(NULL, "\r");
    strcpy(response, tok);

    if( strcmp(command, "GET")==0 ){
        if( strcmp(status, "200")==0 ){
            printf("Request successful. File is being received.\n");
            return 1;
        }
        else if( strcmp(status, "400")==0 ){
            printf("Bad Request.\n");
        }
        else if( strcmp(status, "403")==0 ){
            printf("Forbidden.\n");
        }
        else if( strcmp(status, "404")==0 ){
            printf("File Not Found on server.\n");
        }
    }
    else if( strcmp(command, "PUT")==0 ){
        if( strcmp(status, "200")==0 ){
            printf("Request successful. File is being sent.\n");
            return 1;
        }
        else if( strcmp(status, "400")==0 ){
            printf("Bad Request.\n");
        }
        else if( strcmp(status, "403")==0 ){
            printf("Forbidden.\n");
        }
    }

    return 0;
}

// receive data in packets and return the length of received data
void recv_and_parse(char *buf, char *command, char *fname, int fsize, int sockfd, int bufsize){
    size_t statusOK=0;
    char *body_start=NULL;
    memset(buf, '\0', bufsize);
    int k=0, curr_fsize=0, body=0;
    int fd = open(fname, O_WRONLY | O_CREAT, 0666);

    // add polling here

    while(1){
        char buffer[bufsize], str[2];
        for(int i=0; i<bufsize; i++) buffer[i] = '\0';

        int bytes_received;
        if( (bytes_received = recv(sockfd, buffer, bufsize, 0)) < 0 ){
            perror("recv() failed.\n");
            exit(1);
        }

        for(int i=0; i<bytes_received; i++){
            if(body){
                str[0] = buffer[i];
                str[1] = '\0';
                write(fd, str, 1);
                curr_fsize++;
                continue;
            }
                if(k && k%bufsize == 0)
                    buf = (char *)realloc(buf, (k/bufsize + 1)*bufsize);
                buf[k++] = buffer[i];
                body_start = strstr(buf, "\r\n\r\n");

            if(body_start != NULL){
                printf("Received header:\n%s\n\n", buf);
                // if( !statusOK && !check_header(buf, command) )
                //     break;
                // else
                //     statusOK = 1;
                body = 1;
            }
            
        }
        if(curr_fsize >= fsize) break;
    }
    close(fd);
    free(buf);
}

int main(){
    int sockfd, port_;
    struct sockaddr_in serv_addr;
    size_t n=0;
    char *msg, **cmd_list, ip[16], port[6], http_request[4096], *http_response, header[4096], filename[100], type[20];

    msg = (char *)malloc(MAXRETSIZE * sizeof(char));
    http_response = (char *)malloc(10 * sizeof(char));
    cmd_list = (char **)malloc(sizeof(char *) * 3);
    for(int i=0; i<3; i++){
        cmd_list[i] = (char *)malloc(sizeof(char) * MAXRETSIZE);
    }

    while(1){
        http_response = (char *)malloc(1000 * sizeof(char));

        if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
            perror("socket() failed.\n");
            exit(-1);
        }

        printf("MyOwnBrowser> ");
        getline(&msg, &n, stdin);
        split_command(msg, strlen(msg), cmd_list);

        if( strcmp(cmd_list[0], "GET") == 0 ){
            // parse the address
            parse_addr(cmd_list[1], ip, port, filename);
            sscanf(port, "%d", &port_);

            // connect to the server
            serv_addr.sin_family = AF_INET;
            inet_aton(ip, &serv_addr.sin_addr);
            serv_addr.sin_port = htons(port_);

            if( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ){
                perror("connect() failed.\n");
                exit(1);
            }

            // the type of content can be inferred from
            get_content_type(cmd_list[1], type);
            // create an http request
            frame_request(cmd_list, type, "en-us", ip, port, "en-us", "\0", http_request);

            printf("Request sent:\n\n%s\n\n", http_request);

            // send this as packets
            send_as_packet(http_request, sockfd);

            // parse and receive the response
            recv_and_parse(http_response, cmd_list[0], filename, 113598, sockfd, 1000);

            // close connection to the server
            close(sockfd);

            // open the saved file inside an appropriate application 
            char *argv[256];
            for(int i=0; i<256; i++){
                argv[i] = (char *)malloc(sizeof(char));
            }
            strcpy(argv[0], "open");
            strcpy(argv[1], "-a");

            if( strcmp(type, "application/pdf")==0 || strcmp(type, "image/jpg")==0)
                strcpy(argv[2], "Preview");
            else if( strcmp(type, "text/*")==0 )
                strcpy(argv[2], "TextEdit");
            else if( strcmp(type, "text/html")==0 )
                strcpy(argv[2], "Firefox");

            strcpy(argv[3], filename);
            argv[4] = 0;
            
            pid_t pid = fork();
            if( pid == 0 ){ // open child process
                execvp(argv[0], argv);
            }
            else{
                waitpid(-1, NULL, 0);
            }
        }
        else if( strcmp(cmd_list[0], "PUT") == 0 ){

        }
        else if( strcmp(cmd_list[0], "QUIT") == 0 ){
            printf("Quitting now.\nBye.\n");
            break;
        }
        else{
            printf("Invalid command. Try again.\n");
        }
    }

    return 0;
}