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

int recv_msg(char *buf, int sockfd, int n){
    // for(int i=0; i < 10000; i++) buf[i] = '\0';
    memset(buf, '\0', n);

	int k = 0;
	while(1){
		char buffer[n];
		for(int i = 0;i<n;i++) buffer[i] = '\0';
		int nn = recv(sockfd, buffer, n, 0);
        printf("%s", buffer);
		int end = 0;
		for(int i = 0;i<nn;i++){
			if(buffer[i]=='\0'){
				end = 1; break;
			}
            if(k && k%n==0) buf = (char *)realloc(buf, (k/n + 1)*n);
			buf[k++] = buffer[i];
		}
		if(end) break;
	}
    return k;

}

// receive data in packets and return the length of received data
void recv_and_parse(char *command, char *fname, int sockfd){
    size_t end=0, i, j, h_fixed_ptr=0, statusOK=0;
    char packet[BUFSIZE];
    int gettingHeader=1;
    char version[20], code[5], response[5], header[4096]="\0", data[256];

    FILE *fp;
    fp = fopen(fname, "w");

    // add polling here

    while(!end){
        if( recv(sockfd, packet, BUFSIZE, 0) < 0 ){
            perror("recv() error.\n");
            exit(1);
        }

        printf("%s\n", packet);

        // check if this is the last packet
        for(i=0; i<BUFSIZE; i++){
            if(packet[i]=='\0') end=1;
        }

        // if we are receiving the header currently
        if(gettingHeader){
            strcat(header, packet);
            for(i=h_fixed_ptr; i<strlen(header); i++){
                if( i<strlen(header)-3 && header[i]=='\r' && header[i+1]=='\n' && header[i+2]=='\r' && header[i+3]=='\n' ){
                    // we have characters after "\r\n\r\n"
                    i += 3;
                    header[i] = '\0';
                    // printf("The header received is:\n%s\n", header);

                    // any content after the header is concatenated to body
                    if( i+1<strlen(header) ) strcat(data, header+i+1);
                    // change the flag
                    gettingHeader = 0;
                }
            }
            h_fixed_ptr=i; // can remove this pointer
        }
        // we are getting the body of the message
        else{
            if( data[0] != '\0' ){
                // fprintf(fp, "%s", data);
                // printf("%s", data);
                bzero(data, 256); // empty the data
                continue;
            }
            // check status of the header
            if( !statusOK && !check_header(header, command) )
                break;
            else
                statusOK = 1;
            
            // fprintf(fp, "%s", packet);
            // printf("%s", packet);
            bzero(packet, BUFSIZE);
        }
    }

    if( fclose(fp)!=0 ){
        perror("Unable to close file stream.\n");
        exit(1);
    }
}

int main(){
    int sockfd, port_;
    struct sockaddr_in serv_addr;
    size_t n=0;
    char *msg, **cmd_list, ip[16], port[6], http_request[4096], http_response[4096], header[4096], filename[100], type[20];

    msg = (char *)malloc(MAXRETSIZE * sizeof(char));
    cmd_list = (char **)malloc(sizeof(char *) * 3);
    for(int i=0; i<3; i++){
        cmd_list[i] = (char *)malloc(sizeof(char) * MAXRETSIZE);
    }

    while(1){
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

            // printf("Request sent:\n\n%s\n\n", http_request);

            // send this as packets
            send_as_packet(http_request, sockfd);

            // parse and receive the response
            // recv_and_parse(cmd_list[0], filename, sockfd);
            recv_msg(http_response, sockfd, 4096);

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