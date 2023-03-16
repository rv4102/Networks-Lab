#include "mysocket.h"

int main(){
    int			sockfd, newsockfd ; /* Socket descriptors */
	socklen_t			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[100];		/* We will use this buffer for communication */
	
	if ((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	printf("Server running\n");

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	if (my_bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

    my_listen(sockfd, 1);
    while (1) {

		printf("\nWaiting for the client connection\n");
		clilen = sizeof(cli_addr);
		newsockfd = my_accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}

		printf("Client connected\n");

        char buff[100];
        memset(buff, '\0', 100);
        int n = my_recv(newsockfd, buff, 100, 0);
        printf("Received: %s\nLength: %d\n", buff, n);
		memset(buff, '\0', 100);
		n = my_recv(newsockfd, buff, 100, 0);
        printf("Received: %s\nLength: %d\n", buff, n);
		memset(buff, '\0', 100);
		n = my_recv(newsockfd, buff, 100, 0);
        printf("Received: %s\nLength: %d\n", buff, n);
		memset(buff, '\0', 100);
		strcpy(buff, "hello from the server");
		n = my_send(newsockfd, buff, strlen(buff), 0);
		memset(buff, '\0', 100);
		n = my_recv(newsockfd, buff, 100, 0);
        printf("Received: %s\nLength: %d\n", buff, n);
		
		my_close(newsockfd);
	}
	return 0;

}