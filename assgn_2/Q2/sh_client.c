
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
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[500000];

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}
	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);

	/* With the information specified in serv_addr, the connect()
	   system call establishes a connection with the server process.
	*/
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	//receiving 'LOGIN' from server
	for(i=0; i < 50000; i++) buf[i] = '\0';
	recv(sockfd, buf, 50000, 0);
	printf("%s", buf);


	for(i=0; i < 500000; i++) buf[i] = '\0';
	char username[500000];
	
	for(int i = 0;i<480000;i++) username[i] = 'a';
	//taking username as input from the user
	// scanf("%s", username);
	strcpy(buf, username);

	//sending the username to the server
	send(sockfd, buf, strlen(buf) + 1, 0);
	
	for(int i = 0; i<50;i++) buf[i] = '\0';

	//receiving whether the username exists in the database or not from the server
	recv(sockfd, buf, 50, 0);

	if(strcmp(buf, "NOT-FOUND")==0){
		printf("%s\n", "Invalid username\n");
		close(sockfd);
		return 0;
	}
	else{
		//flushing
		char c;
		scanf("%c", &c);
		while(1){
			printf("Enter the command to be executed: ");
			int close_socket = 0;
			int ending = 0;

			//taking command input from the user in chunks of size 50 and sending the chunks one by one to the server, until the user presses enter.
			while(1){
				for(int i = 0;i<50;i++) buf[i] = '\0';
				int s = 0;
				while(s<50){
					char c; 
					scanf("%c", &c);
					if(c=='\n'){
						ending=1;
						buf[s++] = c;
						break;
					}
					buf[s++] = c;
				}
				if(strcmp(buf, "exit\n")==0){
					send(sockfd, buf, strlen(buf), 0);
					close_socket = 1;
					break;
				}
				send(sockfd, buf, strlen(buf), 0);
				if(ending)  break;
			}
			if(close_socket) break;


			for(int i = 0;i<50;i++) buf[i] = '\0';
			char buffer[5000];
			for(int i = 0;i<5000;i++) buffer[i] = '\0';
			int k = 0;

			//receiving the message from the server in chunks of size 50 and concatenating them to a string of bigger size.
			while(1){
				for(int i = 0;i<50;i++) buf[i] = '\0';
				recv(sockfd, buf, 50, 0);
				int end = 0;
				for(int i = 0;i<50;i++){
					if(buf[i]=='\0'){
						end = 1;
						break;
					}
					buffer[k++] = buf[i];
				}
				if(end) break;
			}

			if(strcmp(buffer, "$$$$")==0){
				printf("Invalid command\n");
			}
			else if(strcmp(buffer, "####")==0){
				printf("Error in running command\n");
			}
			else{
				printf("%s\n", buffer);
			}
		}
	}
	close(sockfd);
	return 0;

}

