#include <stdio.h>
#include "mysocket.h"
#include <signal.h>

#define SOCK_MYTCP 1964
const int BUF_S = 102400;

void SIG_PIPE_handler(int signo)
{
	printf("SIGPIPE caught\n");
}

int main()
{
	signal(SIGPIPE, SIG_PIPE_handler);
	int sockfd, newsockfd; /* Socket descriptors */
	int clilen;
	struct sockaddr_in cli_addr, serv_addr;

	int i;
	char buf[BUF_S]; /* We will use this buffer for communication */

	if ((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0)) < 0)
	{
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(20000);

	if (my_bind(sockfd, (struct sockaddr *)&serv_addr,	sizeof(serv_addr)) < 0){
		perror("Unable to bind local address\n");
		exit(0);
	}
	my_listen(sockfd, 5);

	clilen = sizeof(cli_addr);
	newsockfd = my_accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
	if (newsockfd < 0){
		perror("Accept error\n");
		exit(0);
	}
	
	memset(buf, 0, BUF_S);
	strcpy(buf, "Message from server 1");
	my_send(newsockfd, buf, strlen(buf) + 1, 0);
	
	memset(buf, 0, BUF_S);
	my_recv(newsockfd, buf, BUF_S, 0);
	printf("%s\n", buf);

	memset(buf, 0, BUF_S);
	strcpy(buf, "Message from server 11");
	my_send(newsockfd, buf, strlen(buf) + 1, 0);
	
	memset(buf, 0, BUF_S);
	my_recv(newsockfd, buf, BUF_S, 0);
	printf("%s\n", buf);

	memset(buf, 0, BUF_S);
	strcpy(buf, "Message from server 111");
	my_send(newsockfd, buf, strlen(buf) + 1, 0);
	
	memset(buf, 0, BUF_S);
	my_recv(newsockfd, buf, BUF_S, 0);
	printf("%s\n", buf);

	memset(buf, 0, BUF_S);
	strcpy(buf, "Message from server 1111");
	my_send(newsockfd, buf, strlen(buf) + 1, 0);
	
	memset(buf, 0, BUF_S);
	my_recv(newsockfd, buf, BUF_S, 0);
	printf("%s\n", buf);

	memset(buf, 0, BUF_S);
	strcpy(buf, "Message from server 11111");
	my_send(newsockfd, buf, strlen(buf) + 1, 0);
	
	memset(buf, 0, BUF_S);
	my_recv(newsockfd, buf, BUF_S, 0);
	printf("%s\n", buf);

	memset(buf, 0, BUF_S);
	strcpy(buf, "Message from server 111111");
	my_send(newsockfd, buf, strlen(buf) + 1, 0);
	
	memset(buf, 0, BUF_S);
	my_recv(newsockfd, buf, BUF_S, 0);
	printf("%s\n", buf);

	memset(buf, 0, BUF_S);
	strcpy(buf, "Message from server 1111111");
	my_send(newsockfd, buf, strlen(buf) + 1, 0);
	
	memset(buf, 0, BUF_S);
	my_recv(newsockfd, buf, BUF_S, 0);
	printf("%s\n", buf);

	memset(buf, 0, BUF_S);
	strcpy(buf, "Message from server 11111111");
	my_send(newsockfd, buf, strlen(buf) + 1, 0);
	
	memset(buf, 0, BUF_S);
	my_recv(newsockfd, buf, BUF_S, 0);
	printf("%s\n", buf);

	memset(buf, 0, BUF_S);
	my_recv(newsockfd, buf, BUF_S, 0);
	printf("%s\n", buf);

	my_close(newsockfd);
	return 0;
}