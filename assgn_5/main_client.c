#include "mysocket.h"

int main(){
    int	sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[20];

	if ((sockfd = my_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
    for(int i = 0;i<100;i++) buff[i] = '\0';
    for(int i = 0;i<12;i++) buff[i] = 'a' + i;
	printf("socket: %d\n", sockfd);
	int n = my_send(sockfd, buff, strlen(buff)+1, 0);
	thread_S((void *)&sockfd);


	my_close(sockfd);
}