/*    THE CLIENT PROCESS */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

// void recv_msg(char *buf, int sockfd, int n){
//     for(int i=0; i < 1000; i++) buf[i] = '\0';
// 	int k = 0;
// 	while(1){
// 		char buffer[n];
// 		for(int i = 0;i<n;i++) buffer[i] = '\0';
// 		int nn = recv(sockfd, buffer, n, 0);
// 		int end = 0;
// 		for(int i = 0;i<n;i++){
// 			if(buffer[i]=='\0'){
// 				end = 1; break;
// 			}
//             if(k && k%1000==0) buf = realloc(buf, (k/1000 + 1)*1000);
// 			buf[k++] = buffer[i];
// 		}
// 		if(end) break;
// 	}

// }


// int recv_msg(char *buf, int sockfd, int n){
//     // for(int i=0; i < 10000; i++) buf[i] = '\0';
//     memset(buf, '\0', n);

// 	int k = 0;
// 	while(1){
// 		char buffer[n];
// 		for(int i = 0;i<n;i++) buffer[i] = '\0';
// 		int nn = recv(sockfd, buffer, n, 0);
// 		// printf("%s", buffer);
// 		for(int i = 0;i<nn;i++){
// 			printf("%c", buffer[i]);
// 		}
// 		int end = 0;
// 		for(int i = 0;i<nn;i++){
// 			// if(buffer[i]=='\0'){
// 			// 	end = 1; break;
// 			// }
//             if(k && k%n==0) buf = (char *)realloc(buf, (k/n + 1)*n);
// 			buf[k++] = buffer[i];
// 		}
// 		if(end) break;
// 	}
//     return k;

// }

// int recv_msg(char *buf, int sockfd, int n, int tot){
//     memset(buf, '\0', n);
//     char *body_start;
//     int body = 0;
// 	int k = 0;
// 	int fd = open("a.pdf", O_WRONLY | O_CREAT, 0666);

//     int curr_tot = 0;
// 	while(1){
// 		char buffer[n];
// 		for(int i = 0;i<n;i++) buffer[i] = '\0';
// 		int nn = recv(sockfd, buffer, n, 0);
// 		for(int i = 0;i<nn;i++){
//             if(body){
//                 curr_tot++;
//                 continue;
//             }
//             if(k && k%n==0) buf = (char *)realloc(buf, (k/n + 1)*n);
// 			buf[k++] = buffer[i];
//             body_start = strstr(buf, "\r\n\r\n");
//             if(body_start!=NULL){
//                 body = 1;

// 				// int n = header_parse(buf);
// 				// error handling
            
// 			}
// 		}
// 		write(fd, buffer, nn);
//         if(curr_tot >= tot) break;
// 	}
// 	close(fd);



//     return k;

// }

void input_body(FILE* fd, int sockfd, char rem_string[], int tot_size, int rem_string_size){

    if(rem_string_size){
        fwrite(rem_string, 1, rem_string_size, fd);
    }
    int curr_size = rem_string_size;
	printf("tot_size: %d\n", tot_size);
	printf("curr_size: %d\n", curr_size);
    char buffer[101];
    while(1){
        for(int i = 0;i<101;i++) buffer[i] = '\0';
        int bytes_received = recv(sockfd, buffer, 100, 0);
		if(bytes_received<0){
			perror("recv");
			exit(1);
		}
        curr_size += bytes_received;
		printf("bytes: %d %d\n", bytes_received, curr_size);
        fwrite(buffer, 1, bytes_received, fd);
        if(curr_size >= tot_size) break;
    }
    
}

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
		a += bytes_received;
		// printf("bytes: %d\n", bytes_received);
        for(int i = 0;i<bytes_received;i++){
            if(body_start){
                rem_string[start++] = buffer[i];
                continue;
            }
            buf[k++] = buffer[i];
            body_start = strstr(buf, "\r\n\r\n");
            if(body_start!=NULL){
                end = 1;
                // break;
            }
        }
        if(end) break;
    }
	printf("a: %d\n", a);
	return start;
}
int main(int argc, char *argv[])
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	// char buf[1000];

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
    // int port_no = atoi(argv[1]);    
	serv_addr.sin_port	= htons(8080);

	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	printf("Server connected\n");

    char buf[] = "PUT /pp.pdf HTTP/1.1\r\nHost:localhost:8080\r\nContent-length: 113598\r\n\r\n";
    send(sockfd, buf, strlen(buf)+1, 0);

	FILE *file = fopen("A4.pdf", "rb");
	char buffer[1000];

	int b;
	while((b = fread(buffer,1,1000,file))>0){
		int n = send(sockfd, buffer, b, 0);
		printf("bytes: %d\n", n);
		memset(buffer, '\0', 1000);
	}
	fclose(file);



	char header[5000];
	char rem_string[5000];
	int rem_str_size = msg_rcv(header, sockfd, rem_string);
	printf("%s", header);
	
	// //header parsing for finding the file size
	// FILE* fd = fopen("aa.pdf", "wb");

	// printf("%s", header);

	// input_body(fd, sockfd, rem_string, 113598, rem_str_size);

	// fclose(fd);
	close(sockfd);
	// free(buf);
	return 0;

}