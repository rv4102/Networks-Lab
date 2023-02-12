
/*    THE CLIENT PROCESS */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
	int	sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[20];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}
	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	printf("Server connected\n");
	int num = 1;
	int size_in = 0;
	while(1){
		
		int first_char = 1;
		//entering expression
		printf("Enter an expression (enter -1 to exit): \n");
		while(1){

			size_in = 0;
			int ending  = 0;
			while(size_in<20){
				char c = ' ';
				while(c==' ') scanf("%c", &c);
				if(first_char && c=='-'){
					char ending[1];
					ending[0] = 'e';
					send(sockfd, ending, 1, 0);
					close(sockfd);
					exit(0);
				}
				first_char = 0;
				if(c=='\n'){
					ending = 1;
					buf[size_in++]=c;
					break;
				}
				buf[size_in++] = c;
			}
		
			if(ending){
				int n = send(sockfd, buf, size_in, 0);
				break;
			}
		
			int sn = send(sockfd, buf, 20, 0);
			if(sn<0){
				printf("Error in sending expression\n");
				exit(1);
			}

		}
		for(int i = 0;i<20;i++) buf[i] = '\0';
		int rn = recv(sockfd, buf, 20, 0);
		printf("Answer: %s\n", buf);
		
	}
	return 0;

}

