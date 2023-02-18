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
#include <poll.h>
#include <fcntl.h>
#include <sys/wait.h>

#define BUFSIZE 200
#define MAXRETSIZE 1000
#define MAXFILENAMELEN 200

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

void parse_addr(char *url, char *ip, char *port, char *fname, char *path, char *command){ 
    char url_copy[MAXRETSIZE];
    memset(url_copy, '\0', MAXRETSIZE);
    strcpy(url_copy, url);

    if( strcmp(command, "GET")==0 ){
        char *tok = strtok(url_copy, "/"); // tok == "http:"
        tok = strtok(NULL, "/"); // tok == ip
        strcpy(ip, tok);
        tok = strtok(NULL, "\0");
    
        int ptr=strlen(tok)-1;
        while(ptr >= 0 && tok[ptr] != ':') ptr--;
        ptr++;
        if(ptr != 0){
            strcpy(port, tok+ptr);
        }
        else{
            strcpy(port, "80");
        }

        ptr=strlen(tok)-1;
        while(ptr >= 0 && tok[ptr] != '/') ptr--;
        ptr++;

        int i=0;
        while(ptr<strlen(tok) && tok[ptr] != ':'){
            fname[i++] = tok[ptr];
            ptr++;
        }
    }
    else if( strcmp(command, "PUT")==0 ){
        char *tok = strtok(url_copy, "/"); // tok == "http:"
        tok = strtok(NULL, "/"); // tok == ip
        strcpy(ip, tok);
        tok = strtok(NULL, "\0");

        int ptr=strlen(tok)-1;
        while(ptr >= 0 && tok[ptr] != ':') ptr--;
        ptr++;
        if(ptr != 0){
            strcpy(port, tok+ptr);
        }
        else{
            strcpy(port, "80");
        }
        int j = 0;
        strcpy(path, "/");
        int k = 1;
        while(tok[j]!=':' && tok[j]!='\0'){
            path[k++] = tok[j];
            j++;
        }
    }
}

void get_content_type(char *url, char *type){
    char url_copy[MAXRETSIZE];
    strcpy(url_copy, url);
    char *tok;
    tok = strtok(url_copy, ":"); // the first token has "http"
    tok = strtok(NULL, ":"); // the second token has "//ip/path/to/file.xyz"

    int ptr=strlen(tok);
    while(tok[ptr] != '.') ptr--;
    ptr++;
    strcpy(tok, tok+ptr);

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

void get_file_type_put(char *filename, char *type){
    char file_name_copy[BUFSIZE];
    strcpy(file_name_copy,filename);
    char *tok = strtok(file_name_copy, ".");
    tok = strtok(NULL, "\0");
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

void frame_request_get(char **cmd_list, char *filetype, char *accept_lang, char *ip, char *port,
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
    strcat(http_request, ":");
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
    strcat(http_request, "Content-type: ");
    strcat(http_request, filetype);
    strcat(http_request, "\r\n");
    strcat(http_request, "\r\n");

    return;
}

void frame_request_put(char **cmd_list, char *ip, char *port, char *path, 
                char *fname, FILE *fp, char *filetype, char *http_request){
    char current_time[40], last_modified_time[40];

    get_times(last_modified_time, current_time);
    if(path[strlen(path)-1] != '/') 
        strcat(path, "/");

    strcpy(http_request, cmd_list[0]);
    strcat(http_request, " ");
    strcat(http_request, path);
    strcat(http_request, fname);
    strcat(http_request, " HTTP/1.1\r\n");
    strcat(http_request, "Host: ");
    strcat(http_request, ip);
    strcat(http_request, ":");
    strcat(http_request, port);
    strcat(http_request, "\r\n");
    strcat(http_request, "Connection: close\r\n");
    strcat(http_request, "Date: ");
    strcat(http_request, current_time);
    strcat(http_request, "\r\n");
    strcat(http_request, "If-Modified-Since: ");
    strcat(http_request, last_modified_time);
    strcat(http_request, "\r\n");
    strcat(http_request, "Content-language: ");
    strcat(http_request, "en-us");
    strcat(http_request, "\r\n");

    fseek(fp, 0, SEEK_END);
    long fl = ftell(fp);
    char content_length[20];
    sprintf(content_length, "%ld", fl);
    fseek(fp, 0, SEEK_SET);
    strcat(http_request, "Content-length: ");
    strcat(http_request, content_length);
    strcat(http_request, "\r\n");

    strcat(http_request, "Content-type: ");
    strcat(http_request, filetype);
    strcat(http_request, "\r\n");
    strcat(http_request, "\r\n");

    return;
}

int check_and_parse_header(char *header, char *command, char *fileSize){
    printf("Response received:\n%s\n", header);
    char header_copy[MAXRETSIZE], firstLine[BUFSIZE];
    strcpy(header_copy, header);
    char ver[20], status[10], response[10];

    char *tok = strtok(header_copy, "\n");
    strcpy(firstLine, tok);
    int i=0;
    while(1){
        tok = strtok(NULL, "\n");
        if(strcmp(tok, "\r")==0) 
            break;

        char header_name[BUFSIZE];
        memset(header_name, '\0', BUFSIZE);
        i=0;
        while(1){
            if(tok[i] == ':'){
                strncpy(header_name, tok, i);
                break;
            }
            i++;
        }

        if( strcmp(header_name, "Content-length")==0 || strcmp(header_name, "Content-Length")==0 ){
            i++;
            while( tok[i]==' ' ) i++;
            int j = 0;
            while( tok[i]!='\r' ){
                fileSize[j++] = tok[i];
                i++;
            }
        }
    }
    
    memset(ver, '\0', 20);
    memset(status, '\0', 10);
    memset(response, '\0', 10);
    tok = strtok(firstLine, " ");
    strcpy(ver, tok);
    tok = strtok(NULL, " ");
    strcpy(status, tok);
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

void input_body(FILE* fd, int sockfd, char rem_string[], int tot_size, int rem_string_size, int *status){
    if(rem_string_size){
        fwrite(rem_string, 1, rem_string_size, fd);
    }
    int curr_size = rem_string_size;
    char buffer[101];

    struct pollfd pfd[1];
    pfd[0].fd = sockfd;
    pfd[0].events = POLLIN;

    while(1){
        int rc = poll(pfd, 1, 3000);
        if(rc < 0){
            perror("poll() failed.\n");
            *status = 1;
            break;
        }
        else if(rc == 0){
            printf("Timeout.\n");
            *status = 1;
            break;
        }

        for(int i = 0;i<101;i++) buffer[i] = '\0';
        int bytes_received = recv(sockfd, buffer, 100, 0);
		if(bytes_received<0){
			perror("recv");
			exit(1);
		}
        curr_size += bytes_received;
        fwrite(buffer, 1, bytes_received, fd);
        if(curr_size >= tot_size) break;
    }
}

int msg_rcv(char buf[], int sockfd, char rem_string[], int *status){
    char *body_start  = NULL;
    int k = 0;
    int end = 0;
	int start = 0;
    char buffer[101];
	int a = 0;

    struct pollfd fd[1];
    fd[0].fd = sockfd;
    fd[0].events = POLLIN;

    while(1){
        int rc = poll(fd, 1, 3000);
        if(rc < 0){
            perror("poll() failed.\n");
            *status = 1;
            break;
        }
        else if(rc == 0){
            printf("Timeout.\n");
            *status = 1;
            break;
        }

        for(int i = 0;i<101;i++) buffer[i] = '\0';
        int bytes_received = recv(sockfd, buffer, 100, 0);
		a += bytes_received;
        for(int i = 0;i<bytes_received;i++){
            if(body_start){
                rem_string[start++] = buffer[i];
                continue;
            }
            buf[k++] = buffer[i];
            body_start = strstr(buf, "\r\n\r\n");
            if(body_start!=NULL){
                end = 1;
            }
        }
        if(end) break;
    }
	return start;
}

int main(){
    int sockfd, port_;
    struct sockaddr_in serv_addr;
    int poll_failed=0;
    
    char *msg, **cmd_list, *ip, *port, *http_request, *rem_str, *filename, *type, *header, *fileSize, *path;
    msg = (char *)malloc(MAXRETSIZE * sizeof(char));
    cmd_list = (char **)malloc(sizeof(char *) * 3);
    for(int i=0; i<3; i++){
        cmd_list[i] = (char *)malloc(MAXRETSIZE * sizeof(char));
    }
    ip = (char *)malloc(16 * sizeof(char));
    port = (char *)malloc(6 * sizeof(char));
    http_request = (char *)malloc(BUFSIZE * sizeof(char));
    rem_str = (char *)malloc(BUFSIZE * sizeof(char));
    filename = (char *)malloc(MAXFILENAMELEN * sizeof(char));
    type = (char *)malloc(20 * sizeof(char));
    header = (char *)malloc(MAXRETSIZE * sizeof(char));
    fileSize = (char *)malloc(100 * sizeof(char));
    path = (char *)malloc(BUFSIZE * sizeof(char));


    while(1){
        poll_failed = 0;
        if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
            perror("socket() failed.\n");
            exit(-1);
        }

        printf("MyOwnBrowser> ");

        size_t n=0;
        getline(&msg, &n, stdin);
        split_command(msg, strlen(msg), cmd_list);

        if( strcmp(cmd_list[0], "GET") == 0 ){
            // parse the address
            memset(ip, '\0', 16);
            memset(port, '\0', 6);
            memset(filename, '\0', MAXFILENAMELEN);
            memset(path, '\0', BUFSIZE);
            parse_addr(cmd_list[1], ip, port, filename, path, cmd_list[0]);
            sscanf(port, "%d", &port_);

            // connect to the server
            serv_addr.sin_family = AF_INET;
            inet_aton(ip, &serv_addr.sin_addr);
            serv_addr.sin_port = htons(port_);

            if( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ){
                perror("connect() failed.\n");
                exit(1);
            }

            // the type of content can be inferred
            memset(type, '\0', 20);
            get_content_type(cmd_list[1], type);

            // create an http request
            memset(http_request, '\0', BUFSIZE);
            frame_request_get(cmd_list, type, "en-us", ip, port, "en-us", "\0", http_request);

            printf("Request sent:\n%s\n\n", http_request);

            // send this as packets
            // send_as_packet(http_request, sockfd);
            send(sockfd, http_request, strlen(http_request), 0);

            // get the header
            memset(rem_str, '\0', BUFSIZE);
            memset(header, '\0', MAXRETSIZE);
            int rem_size = msg_rcv(header, sockfd, rem_str, &poll_failed);
            if(poll_failed){
                close(sockfd);
                continue;
            }

            // get file size from header
            memset(fileSize, '\0', 100);
            int v = check_and_parse_header(header, cmd_list[0], fileSize);
            if(v != 0){
                // open the file to be written to
                FILE* fd = fopen(filename, "wb");
                // get the data
                int fileSize_;
                sscanf(fileSize, "%d", &fileSize_);
                input_body(fd, sockfd, rem_str, fileSize_, rem_size, &poll_failed);
                if(poll_failed){
                    close(sockfd);
                    continue;
                }

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

                fclose(fd);
            }

            // close connection to the server
            close(sockfd);
        }
        else if( strcmp(cmd_list[0], "PUT") == 0 ){
            memset(ip, '\0', 16);
            memset(port, '\0', 6);
            memset(filename, '\0', MAXFILENAMELEN);
            memset(path, '\0', BUFSIZE);
            parse_addr(cmd_list[1], ip, port, filename, path, cmd_list[0]);

            sscanf(port, "%d", &port_);

            // connect to the server
            serv_addr.sin_family = AF_INET;
            inet_aton(ip, &serv_addr.sin_addr);
            serv_addr.sin_port = htons(port_);

            if( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ){
                perror("connect() failed.\n");
                exit(1);
            }

            // the type of content can be inferred
            memset(type, '\0', 20);
            get_file_type_put(cmd_list[2], type);

            memset(http_request, '\0', BUFSIZE);
            FILE *fp = fopen(cmd_list[2], "r");
            if(fp == NULL){
                printf("file not found\n");
                close(sockfd);
                continue;
            }
            frame_request_put(cmd_list, ip, port, path, cmd_list[2], fp, type, http_request);

            send(sockfd, http_request, strlen(http_request), 0);

            // now we send the file itself
            int b;
            char buffer[MAXRETSIZE];
            memset(buffer, '\0', MAXRETSIZE);
            while( (b = fread(buffer, 1, MAXRETSIZE, fp)) > 0 ){
                send(sockfd, buffer, b, 0);
                memset(buffer, '\0', MAXRETSIZE);
            }
            fclose(fp);
            
            char response[5000];
            memset(response, '\0', 5000);
            memset(rem_str, '\0', BUFSIZE);
            msg_rcv(response, sockfd, rem_str, &poll_failed);
            if(poll_failed){
                close(sockfd);
                continue;
            }
            printf("Response received:\n%s\n\n", response);

            close(sockfd);
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