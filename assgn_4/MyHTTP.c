#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

// function to get the request body and write it to a file
void input_body(FILE* fd, int sockfd, char rem_string[], int tot_size, int rem_string_size){

    if(rem_string_size){
        fwrite(rem_string, 1, rem_string_size, fd);
    }
    int curr_size = rem_string_size;

    char buffer[101];
    while(1){
        for(int i = 0;i<101;i++) buffer[i] = '\0';
        int bytes_received = recv(sockfd, buffer, 100, 0);
    
		if(bytes_received<0){
			perror("recv");
			exit(1);
		}
        printf("Received %d bytes\n", bytes_received);
        curr_size += bytes_received;
        fwrite(buffer, 1, bytes_received, fd);
        if(curr_size >= tot_size) break;
    }
    
}

// function to get the header message from the client
int msg_rcv(char buf[], int sockfd, char rem_string[]){
    char *body_start  = NULL;
    int k = 0;
    int end = 0;
	int start = 0;
    char buffer[101];
	int a = 0;
    while(1){
        for(int i = 0;i<101;i++) buffer[i] = '\0';
        int bytes_received = recv(sockfd, buffer, 100, 0);
        printf("Received %d bytes\n", bytes_received);
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

int main()
{
	int			sockfd, newsockfd ; /* Socket descriptors */
	socklen_t			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;

    // create a socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

    // bind the socket to an address
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(8080);

    // bind the socket to the address
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}
	printf("Server running\n");

	listen(sockfd, 5); 


    // open the AccessLog file to write the logs
    FILE *file_log = fopen("AccessLog.txt", "a");
    if(file_log == NULL){
        printf("File not found\n");
        return 0;
    }
	while (1) {

		clilen = sizeof(cli_addr);

        // accept the connection from the client
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}

        // create a child process to handle the request
		if (fork() == 0) {

            
			close(sockfd);

            // header message from the client will be stored in the buffer array
            // remaing string corresponding to the request body will be stored in the rem_string array
			char buffer[5000], rem_string[5000];
            
            int rem_string_size = msg_rcv(buffer, newsockfd, rem_string);

            printf("Request:\n%s\n", buffer);
            char *dup = strdup(buffer);
            
            time_t current_time = time(NULL);
            struct tm *time_struct = localtime(&current_time);

            char *token = strtok(buffer, " ");

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &cli_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

            // write the log to the AccessLog file
            fprintf(file_log, "%02d%02d%02d:%02d%02d%02d:%s:%d:",time_struct->tm_mday, time_struct->tm_mon+1, time_struct->tm_year-100, time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec, client_ip, ntohs(cli_addr.sin_port));

            printf("Response:\n");
            if(strcmp(token, "GET")==0){

                // get the path of the file to be sent
                char *path  = strtok(NULL, " ");

                // write the log to the AccessLog file
                fprintf(file_log, "%s:%s\n", token, path);
                char *http_version = strtok(NULL, "\n");
                if(strcmp(http_version, "HTTP/1.1\r")){

                    // send the 400 Bad Request response to the client if the HTTP version is not 1.1
                    char response_[1000];
                    memset(response_, '\0', 1000);
                    strcat(response_, "HTTP/1.1 400 Bad Request\r\n");
                    send(newsockfd, response_, strlen(response_), 0);
                    printf("%s", response_);
                    memset(response_, '\0', 1000);
                    strcat(response_, "\r\n");
                    send(newsockfd, response_, strlen(response_), 0);
                    printf("%s", response_);
                    close(newsockfd);
                    exit(0);
                }
                FILE *file = fopen(path+1, "r");

                char response_[1000];
               
                if(file == NULL){

                    // send the 404 Not Found response to the client if the file is not found
                    memset(response_, '\0', 1000);
                    strcat(response_, "HTTP/1.1 404 Not Found\r\n");
                    send(newsockfd, response_, strlen(response_), 0);
                    printf("%s", response_);

                    memset(response_, '\0', 1000);
                    strcat(response_, "\r\n");
                    send(newsockfd, response_, strlen(response_), 0);
                    printf("%s", response_);

                    close(newsockfd);
                    exit(0);
                }

                // send the 200 OK response to the client
                memset(response_, '\0', 1000);
                strcat(response_, "HTTP/1.1 200 OK\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                // generating the response string and sending the response to the client as well as printing it
                
                time_t current_time = time(NULL);
                struct tm *time_struct = localtime(&current_time);
                time_struct->tm_mday  += 3;
                time_t new_time = mktime(time_struct);
                char *time_string = ctime(&new_time);
                memset(response_, '\0', 1000);
                strcat(response_, "Expires: ");
                strcat(response_, time_string);

                for(int i = 0;;i++){
                    if(response_[i] == '\n'){
                        response_[i] = '\r';
                        response_[i+1] = '\n';
                        break;
                    }
                }
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);
                

                memset(response_, '\0', 1000);
                strcat(response_, "Cache-Control: no-store\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);


                memset(response_, '\0', 1000);
                strcat(response_, "Content-language: en-us\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);


                memset(response_, '\0', 1000);
                strcat(response_, "Content-length: ");
                fseek(file,0,SEEK_END);
                long fl = ftell(file);
                char len_str[20];
                sprintf(len_str, "%ld", fl);
                strcat(response_, len_str);
                strcat(response_, "\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                char *dup_path = strdup(path);
                memset(response_, '\0', 1000);
                strcat(response_, "Content-type: ");
                char *ext = strtok(dup_path, ".");
                ext = strtok(NULL, ".");
                strcat(response_, ext);
                strcat(response_, "\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                memset(response_, '\0', 1000);
                strcat(response_, "Last modified: \r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                memset(response_, '\0', 1000);
                strcat(response_, "\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                // fclose(file);
                memset(response_, '\0', 1000);

                // sending the file bit stream to the client
                // FILE* file_fd  = fopen(path+1, "rb");
                int b;
                fseek(file, 0, SEEK_SET);
                while((b = fread(response_,1,1000,file))>0){
                    int n = send(newsockfd, response_, b, 0);
                    memset(response_, '\0', 1000);
                }
                fclose(file);

            }else if(strcmp(token, "PUT")==0){

                // get the path of the file to be sent
                char *path = strtok(NULL, " ");

                // write the log to the AccessLog file
                fprintf(file_log, "%s:%s\n", token, path);
                char *html_version = strtok(NULL, "\n");
                if(strcmp(html_version, "HTTP/1.1\r")){

                    // send the 400 Bad Request response to the client if the HTTP version is not 1.1
                    char response_[1000];
                    memset(response_, '\0', 1000);
                    strcat(response_, "HTTP/1.1 400 Bad Request\r\n");
                    send(newsockfd, response_, strlen(response_), 0);
                    printf("%s", response_);

                    memset(response_, '\0', 1000);
                    strcat(response_, "\r\n");
                    send(newsockfd, response_, strlen(response_), 0);
                    printf("%s", response_);

                    close(newsockfd);
                    exit(0);
                }

                char *host = strtok(NULL, "\n");
                char *host_name;
                int i = 0;

                while(1){
                    if(host[i] == ':'){
                        host_name = host+i+1;
                        if(host_name[0]==' '){
                            host_name = host_name+1;
                        }
                        break;
                    }
                    i++;
                }
                char content_length[100], content_type[100];

                // getting the required headers
                // getting the content length and content type from the header
                while(1){
                    char *header = strtok(NULL, "\n");
                    if(strcmp(header, "\r")==0){
                        break;
                    }
                    char header_name[100];
                    memset(header_name, '\0', 100);
                    i = 0;
                    while(1){
                        if(header[i] == ':'){
                            strncpy(header_name, header, i);
                            break;
                        }
                        i++;
                    }
                    if(strcmp(header_name, "Content-length")==0){
                        i++;
                        while(header[i]==' ') i++; 
                        int j = 0;  
                        while(header[i]!='\r'){
                            content_length[j++] = header[i];
                            i++;
                        }
                    }else if(strcmp(header_name, "Content-type")==0){
                        i++;
                        while(header[i]==' ') i++;
                        while (header[i]!='\r')
                        {
                            content_type[i] = header[i];
                            i++;
                        }
                        
                    }

                }

                int len = 0;
                char response_[1000];
                sscanf(content_length, "%d", &len);

                FILE *file_put = fopen(path+1, "wb");
                if(file_put == NULL){

                    // send the 404 Not Found response to the client if the file is not found
                    memset(response_, '\0', 1000);
                    strcat(response_, "HTTP/1.1 404 Not Found\r\n");
                    send(newsockfd, response_, strlen(response_), 0);
                    printf("%s", response_);
                    memset(response_, '\0', 1000);
                    strcat(response_, "\r\n");
                    send(newsockfd, response_, strlen(response_), 0);
                    printf("%s", response_);
                    close(newsockfd);
                    exit(0);
                }

                // getting the file content from the client
                input_body(file_put, newsockfd, rem_string, len, rem_string_size);
                
                fclose(file_put);

                // send the 200 OK response to the client
                memset(response_, '\0', 1000);
                strcat(response_, "HTTP/1.1 200 OK\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                // sending the required headers

                time_t current_time = time(NULL);
                struct tm *time_struct = localtime(&current_time);
                time_struct->tm_mday  += 3;
                time_t new_time = mktime(time_struct);
                char *time_string = ctime(&new_time);
                memset(response_, '\0', 1000);
                strcat(response_, "Expires: ");
                strcat(response_, time_string);

                for(int i = 0;;i++){
                    if(response_[i] == '\n'){
                        response_[i] = '\r';
                        response_[i+1] = '\n';
                        break;
                    }
                }
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);
                
                memset(response_, '\0', 1000);
                strcat(response_, "Cache-Control: no-store\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                memset(response_, '\0', 1000);
                strcat(response_, "Content-language: en-us\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                memset(response_, '\0', 1000);
                strcat(response_, "Content-length: \r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                char *dup_path = strdup(path);
                memset(response_, '\0', 1000);
                strcat(response_, "Content-type: ");
                char *ext = strtok(dup_path, ".");
                ext = strtok(NULL, ".");
                strcat(response_, ext);
                strcat(response_, "\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                time_t last_modified;
                time(&last_modified);
                memset(response_, '\0', 1000);
                strcat(response_, "Last modified: ");
                strcat(response_, ctime(&last_modified));
                for(int i = 0;;i++){
                    if(response_[i] == '\n'){
                        response_[i] = '\r';
                        response_[i+1] = '\n';
                        break;
                    }
                }
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                memset(response_, '\0', 1000);
                strcat(response_, "\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

            }else{

                // send the 400 Bad Request response to the client if the request is invalid
                fprintf(file_log, "Invalid\n");
                char response_[1000];
                memset(response_, '\0', 1000);
                strcat(response_, "HTTP/1.1 400 Bad Request\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                memset(response_, '\0', 1000);
                strcat(response_, "\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                printf("%s", response_);

                close(newsockfd);
                exit(0);
                
            }

            // closing the socket and the log file
            fclose(file_log);
			close(newsockfd);
			exit(0);
		}
        
    }

    close(sockfd);
	
	return 0;
}
			

