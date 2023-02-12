
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

//function to receive a message 
void recv_msg(char *buf, int sockfd, int n){
    for(int i=0; i < n; i++) buf[i] = '\0';
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
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char *buf;		/* We will use this buffer for communication */
	buf = (char *)malloc(1000);
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	printf("Server running\n");

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
    int port_no = atoi(argv[1]);
	serv_addr.sin_port		= htons(port_no);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5);

	while (1) {

		printf("\nWaiting for the Load server connection\n");
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}

		printf("Load server connected\n");

		//using the recv_msg function
		recv_msg(buf, newsockfd, 1000);

		printf("Request from Load server: %s\n", buf);
        if(strcmp(buf, "Send Load")==0){
			srand(time(0) + port_no);
            int a = rand();
            a = a%100;
            a++;
            for(int i = 0;i<1000;i++) buf[i] = '\0';
            sprintf(buf, "%d", a);
            send(newsockfd, buf, strlen(buf)+1, 0);
            printf("Load sent: %d\n", a);
        }else if(strcmp(buf, "Send Time")==0){
            time_t tm;
            time(&tm);
			for(int i = 0;i<1000;i++) buf[i] = '\0';
            strcpy(buf,ctime(&tm));
            send(newsockfd, buf, strlen(buf)+1, 0);
            printf("Date and time sent\n");
        }

		close(newsockfd);
	}
	free(buf);
	return 0;
}
			

