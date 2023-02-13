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

void parse_ip(char *url, char *ip, char *port){
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

void frame_request(char **cmd_list, char *time, char *accept_lang, char *ip, char *port,
        char *modified_since, char *content_language, char *content_length, char *http_request) {
    size_t ptr_1=0, ptr_2=0;

    char type[20];
    // the type of content can be inferred from
    get_content_type(cmd_list[1], type);
    http_request[0]='\0';
    strcat(http_request, cmd_list[0]);
    // strcpy(http_request, cmd_list[0]);
    strcat(http_request, " ");

    // get the url
    ptr_1 = 7;
    while(cmd_list[1][ptr_1] != '/') ptr_1++;
    ptr_2 = strlen(http_request);
    while(cmd_list[1][ptr_1] != ':' && cmd_list[1][ptr_1] != '\0') {
        http_request[ptr_2++] = cmd_list[1][ptr_1];
        ptr_1++;
    }

    strcat(http_request, " HTTP/1.1\r\n");

    strcat(http_request, "Host: ");
    strcat(http_request, ip);
    strcat(http_request, ": ");
    strcat(http_request, port);
    strcat(http_request, "\r\n");

    strcat(http_request, "Connection: close\r\n");
    strcat(http_request, "Date: ");
    strcat(http_request, time);
    strcat(http_request, "\r\n");
    strcat(http_request, "Accept: ");
    strcat(http_request, type);
    strcat(http_request, "\r\n");
    strcat(http_request, "Accept-Language: ");
    strcat(http_request, accept_lang);
    strcat(http_request, "\r\n");
    strcat(http_request, "If-Modified-Since: ");
    strcat(http_request, modified_since);
    strcat(http_request, "\r\n");
    strcat(http_request, "Content-language: ");
    strcat(http_request, content_language);
    strcat(http_request, "\r\n");
    strcat(http_request, "Content-length: ");
    strcat(http_request, content_length);
    strcat(http_request, "\r\n");
    strcat(http_request, "Content-type: ");
    strcat(http_request, type);
    strcat(http_request, "\r\n");

    return;
}

void parse_response(){

}

int main(){
    int sockfd, port_;
    struct sockaddr_in serv_addr;
    size_t n=0;
    char *msg, **cmd_list, ip[16], port[6], http_request[4096], current_time[40], last_modified_time[40];
    msg = (char *)malloc(MAXRETSIZE * sizeof(char));
    cmd_list = (char **)malloc(sizeof(char *) * 3);
    for(int i=0; i<3; i++){
        cmd_list[i] = (char *)malloc(sizeof(char) * MAXRETSIZE);
    }

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("socket() failed.\n");
		exit(-1);
	}

    while(1){
        port_=80;
        printf("MyOwnBrowser> ");
        getline(&msg, &n, stdin);
        split_command(msg, strlen(msg), cmd_list);
        if( strcmp(cmd_list[0], "GET") == 0 ){
            // parse the IP address
            parse_ip(cmd_list[1], ip, port);
            sscanf(port, "%d", &port_);

            // connect to the server
            serv_addr.sin_family = AF_INET;
            inet_aton(ip, &serv_addr.sin_addr);
            serv_addr.sin_port = htons(port_);

            if( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ){
                perror("connect() failed.\n");
                exit(1);
            }

            // create an http request
            get_times(last_modified_time, current_time);
            frame_request(cmd_list, current_time, "en-us", ip, port, last_modified_time, "en-us", "\0", http_request);

            // send this as packets
            

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