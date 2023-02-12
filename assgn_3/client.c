
/*    THE CLIENT PROCESS */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void recv_msg(char *buf, int sockfd, int n){
    for(int i=0; i < 1000; i++) buf[i] = '\0';
	int k = 0;
	while(1){
		char buffer[n];
		for(int i = 0;i<n;i++) buffer[i] = '\0';
		int nn = recv(sockfd, buffer, n, 0);
		int end = 0;
		for(int i = 0;i<n;i++){
			if(buffer[i]=='\0'){
				end = 1; break;
			}
            if(k && k%1000==0) buf = realloc(buf, (k/1000 + 1)*1000);
			buf[k++] = buffer[i];
		}
		if(end) break;
	}

}

int main(int argc, char *argv[])
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char *buf;
	buf = (char *)malloc(1000);

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
    int port_no = atoi(argv[1]);    
	serv_addr.sin_port	= htons(port_no);

	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	printf("Server connected\n");

	recv_msg(buf,  sockfd ,1000);

	printf("%s", buf);		
	close(sockfd);
	free(buf);
	return 0;

}

