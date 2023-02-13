
/*    THE CLIENT PROCESS */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
	serv_addr.sin_port	= htons(8000);

	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	printf("Server connected\n");

    char buf[] = "GET /A4.pdf HTTP/1.1\r\nHost:localhost:8000\r\nUser-Agent:Mozilla/5.0 (X11; Linux x86_64; rv:60.0) Gecko/20100101 Firefox/60.0\r\nAccept:text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept:pdf\r\nAccept-Language:en-US,en;q=0.5\r\nConnection:close\r\n\r\nksjdhashdjkasdjkasndjknasjkdnaksndkasndasndjkasndjkasndjknasjkdnasjkndasjndjkasnk";
    // printf("%d", int(strlen(buf));
    send(sockfd, buf, strlen(buf)+1, 0);



    char *buffer;
    buffer = (char *)malloc(10);
    int n = recv_msg(buffer, sockfd, 10);

    // printf("%d", n);
	printf("%s", buffer);		
	close(sockfd);
	// free(buf);
	return 0;

}

