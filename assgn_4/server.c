/*
			NETWORK PROGRAMMING WITH SOCKETS

In this program we illustrate the use of Berkeley sockets for interprocess
communication across the network. We show the communication between a server
process and a client process.


*/

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
#include <fcntl.h>

			/* THE SERVER PROCESS */

int recv_msg(char *buf, int sockfd, int n){
    // for(int i=0; i < 10000; i++) buf[i] = '\0';
    memset(buf, '\0', n);

	int k = 0;
	while(1){
		char buffer[n];
		for(int i = 0;i<n;i++) buffer[i] = '\0';
		int nn = recv(sockfd, buffer, n, 0);
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

int main()
{
	int			sockfd, newsockfd ; /* Socket descriptors */
	socklen_t			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	// char buf[500000];		/* We will use this buffer for communication */

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(8000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}
	printf("Server running\n");

	listen(sockfd, 5); 



    FILE *file_log = fopen("AccessLog.txt", "a");
    if(file_log == NULL){
        printf("File not found\n");
        return 0;
    }
	while (1) {

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}

		if (fork() == 0) {

			/* This child process will now communicate with the
			   client through the send() and recv() system calls.
			*/
			close(sockfd);	/* Close the old socket since all
					   communications will be through
					   the new socket.
					*/

			char *buffer;
            buffer = (char *)malloc(10);
            int n = recv_msg(buffer, newsockfd, 10);
            char *dup = strdup(buffer);
            
            // char new_time[100];
            time_t current_time = time(NULL);
            struct tm *time_struct = localtime(&current_time);
            struct tm *current_times = time_struct;
            time_struct->tm_mday += 3;
            time_t new_time = mktime(time_struct);
            char *time_string = ctime(&new_time);

            char *token = strtok(buffer, " ");

            printf("Command: %s\n", token);

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &cli_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            fprintf(file_log, "%02d%02d%02d:%02d%02d%02d:%s:%d:",current_times->tm_mday-3, current_times->tm_mon+1, current_times->tm_year-100, current_times->tm_hour, current_times->tm_min, current_times->tm_sec, client_ip, ntohs(cli_addr.sin_port));

            if(strcmp(token, "GET")==0){
                char *path  = strtok(NULL, " ");
                printf("path: %s\n", path);

                fprintf(file_log, "%s:%s\n", token, path);
                char *http_version = strtok(NULL, "\n");
                printf("http_version: %s\n", http_version);
                if(strcmp(http_version, "HTTP/1.1\r")){
                    printf("Invalid HTTP version\n");
                    close(newsockfd);
                    exit(0);
                }

                char *host = strtok(NULL, "\n");
                char *host_name = strtok(host, ":");
                host_name = strtok(NULL, ":");

                printf("host_name: %s\n", host_name);

                FILE *file = fopen(path+1, "r");
                int file_fd  = open(path+1, O_RDONLY);
                char response_[1000];
               
                if(file == NULL){
                    printf("File not found\n");
                    memset(response_, '\0', 1000);
                    strcat(response_, "HTTP/1.1 404 Not Found\r\n");
                    send(newsockfd, response_, strlen(response_), 0);
                    memset(response_, '\0', 1000);
                    send(newsockfd, response_, strlen(response_)+1, 0);
                    close(newsockfd);
                    exit(0);
                }
                
                memset(response_, '\0', 1000);
                strcat(response_, "HTTP/1.1 200 OK\r\n");
                send(newsockfd, response_, strlen(response_), 0);
                
                memset(response_, '\0', 1000);
                strcat(response_, "Expires: ");
                printf("time_string: %s", time_string);
                strcat(response_, time_string);

                for(int i = 0;;i++){
                    if(response_[i] == '\n'){
                        response_[i] = '\r';
                        response_[i+1] = '\n';
                        break;
                    }
                }
                send(newsockfd, response_, strlen(response_), 0);
                
                memset(response_, '\0', 1000);
                strcat(response_, "Cache-Control: no-store\r\n");
                send(newsockfd, response_, strlen(response_), 0);

                memset(response_, '\0', 1000);
                strcat(response_, "Content-language: en-us\r\n");
                send(newsockfd, response_, strlen(response_), 0);

                memset(response_, '\0', 1000);
                strcat(response_, "Content-length: ");
                fseek(file,0,SEEK_END);
                long fl = ftell(file);
                char len_str[20];
                sprintf(len_str, "%ld", fl);
                strcat(response_, len_str);
                strcat(response_, "\r\n");
                send(newsockfd, response_, strlen(response_), 0);

                memset(response_, '\0', 1000);
                strcat(response_, "Content-type: pdf\r\n");
                send(newsockfd, response_, strlen(response_), 0);

                memset(response_, '\0', 1000);
                strcat(response_, "Last modified: \r\n");
                send(newsockfd, response_, strlen(response_), 0);

                memset(response_, '\0', 1000);
                strcat(response_, "\r\n");
                send(newsockfd, response_, strlen(response_), 0);

                memset(response_, '\0', 1000);

                int b;
                while((b = read(file_fd, response_, 1000))>0){
                    send(newsockfd, response_, b, 0);
                }
                memset(response_, '\0', 1000);
                
                fclose(file);
                close(file_fd);

            }else if(strcmp(token, "PUT")==0){

                char *path = strtok(NULL, " ");
                printf("path: %s\n", path);
                char *html_version = strtok(NULL, "\n");
                if(strcmp(html_version, "HTTP/1.1\r")){
                    printf("Invalid HTTP version\n");
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
                printf("host_name: %s\n", host_name);
                char *content_length, *content_type;
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
                    if(strcmp(header_name, "Content-Length")==0){
                        content_length = header+i+1;
                    }else if(strcmp(header_name, "Content-Type")==0){
                        content_type = header+i+1;
                    }

                }

                printf("content_length: %s\n", content_length);
                printf("content_type: %s\n", content_type);

                char *body_start = strstr(dup, "\r\n\r\n");
                body_start = body_start+4;

                char body[1000000];
                memset(body, '\0', 1000000);
                strcpy(body, body_start);
                printf("body: %s\n", body);

                FILE *file = fopen(path+1, "w");
                fprintf(file, "%s", body);
                fclose(file);

            }else{
                printf("Invalid command\n");

                char response_[1000];
                memset(response_, '\0', 1000);
                strcat(response_, "HTTP/1.1 400 Bad Request\n");
                send(newsockfd, response_, strlen(response_), 0);
                memset(response_, '\0', 1000);
                send(newsockfd, response_, strlen(response_)+1, 0);
                
            }
			close(newsockfd);
			exit(0);
		}

		close(newsockfd);
	}
	return 0;
}
			

