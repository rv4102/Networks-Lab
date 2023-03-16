#include "mysocket.h"

int main(){
    int	sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[20];

	if ((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}
	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	if ((my_connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	printf("Server connected\n");

	char buff[100];
    memset(buff, '\0', 100);
    strcpy(buff, "hello1");
	int n = my_send(sockfd, buff, strlen(buff)+1, 0);
	memset(buff, '\0', 100);
	strcpy(buff, "variable length message2");
	n = my_send(sockfd, buff, strlen(buff)+1, 0);
	memset(buff, '\0', 100);
	strcpy(buff, "hello3");
	n = my_send(sockfd, buff, strlen(buff)+1, 0);
	memset(buff, '\0', 100);
	n = my_recv(sockfd, buff, 100, 0);
	printf("Received: %s\nLength: %d\n", buff, n);
	memset(buff, '\0', 100);
	strcpy(buff, "hello4");
	n = my_send(sockfd, buff, strlen(buff)+1, 0);
	
	my_close(sockfd);
}