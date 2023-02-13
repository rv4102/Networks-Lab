#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

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

void get_content_type(char *url, char *type){
    char *tok;
    tok = strtok(url, ":"); // the first token has "http"
    printf("%s\n", tok);
    tok = strtok(NULL, ":"); // the second token has "//ip/path/to/file.xyz"
    printf("%s\n", tok);

    strcpy(type, tok + strlen(tok)-3);
    printf("%s\n", type);
}

void parse_ip(char *url, char *ip, char *port){
    size_t ptr=7, idx=0; // "http://" uses 7 characters
    while(isdigit(url[ptr]) || url[ptr] == '.'){
        ip[idx++] = url[ptr];
        ptr++;
    }

    ptr=strlen(url)-1;
    if( !isdigit(url[ptr]) )
        return;
    
    while( isdigit(url[ptr]) ) ptr--;
    ptr++;

    strcpy(port, url+ptr);
}

// Adjust date by a number of days +/-
void DatePlusDays( struct tm* date, int days ){
    const time_t ONE_DAY = 24 * 60 * 60 ;

    // Seconds since start of epoch
    time_t date_seconds = mktime( date ) + (days * ONE_DAY) ;

    // Update caller's date
    // Use localtime because mktime converts to UTC so may change date
    *date = *localtime( &date_seconds ) ;
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

    char *type;
    // the type of content can be inferred from
    get_content_type(cmd_list[1], type);

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
}

int main(){
    char msg[100] = "GET http://127.0.0.1/x/y/z.pdf:8000\n", **cmd_list;
    cmd_list = (char **)malloc(sizeof(char *) * 3);
    for(int i=0; i<3; i++){
        cmd_list[i] = (char *)malloc(sizeof(char) * 100);
    }
    split_command(msg, strlen(msg), cmd_list);

    for(int i=0; i<3; i++){
        printf("%s\n", cmd_list[i]);
    }

    char url[50] = "hello:8080";
    size_t ptr=strlen(url)-1;
    
    while( isdigit(url[ptr]) ) ptr--;
    
    ptr++;
    printf("%zu\n", ptr);

    // check the content type parsing
    char type[4], url1[50] = "http://127.0.0.1/x/y/z.pdf: 8080", ip[20], port[6]="\0";
    get_content_type(url1, type);
    parse_ip(url1, ip, port);
    // printf("%s\n", type);
    printf("%s\n", ip);
    printf("%s\n", port);
    int port_=80;
    // if()
    sscanf(port, "%d", &port_);
    printf("%d\n", port_);

    // create an http request
    char curr[30], last[30];

    char http_request[2048], content_length[20] = "\0";

    parse_ip(cmd_list[1], ip, port);
            sscanf(port, "%d", &port_);

            // // connect to the server
            // serv_addr.sin_family = AF_INET;
            // inet_aton(ip, &serv_addr.sin_addr);
            // serv_addr.sin_port = htons(port_);

            // if( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ){
            //     perror("connect() failed.\n");
            //     exit(1);
            // }

            // create an http request
            get_times(last, curr);
            frame_request(cmd_list, curr, "en-us", ip, port, last, "en-us", "\0", http_request);

            printf("%s", http_request);
    
    

    return 0;
}