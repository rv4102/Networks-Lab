/*
			NETWORK PROGRAMMING WITH SOCKETS

In this program we illustrate the use of Berkeley sockets for interprocess
communication across the network. We show the communication between a server
process and a client process.


*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

			/* THE SERVER PROCESS */

int main()
{
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[500000];		/* We will use this buffer for communication */

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}
	printf("Server running\n");

	listen(sockfd, 5); 
	while (1) {

		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}

		if (fork() == 0) {

			/* This child process will now communicate with the
			   client through the send() and recv() system calls.
			*/
			close(sockfd);	/* Close the old socket since all
					   communications will be through
					   the new socket.
					*/

			/* We initialize the buffer, copy the message to it,
			   and send the message to the client. 
			*/
			
			strcpy(buf,"LOGIN:");

			//sending the 'LOGIN' string to the client
			send(newsockfd, buf, strlen(buf) + 1, 0);

			// We again initialize the buffer, and receive the username from the client. 
			
			for(i=0; i < 500000; i++) buf[i] = '\0';

			recv(newsockfd, buf, 5000000, 0);

			printf("%d\n", (int)strlen(buf));
			
			//checking the username in the list of usernames present in the users.txt file
			// FILE * filepointer;
			// filepointer = fopen("users.txt", "r");
			// char users[50];
			// int found = 0;
			// while(fgets(users, 50, filepointer)!=NULL){
			// 	users[(int)strlen(users)-1] = '\0';
			// 	if(strcmp(users, buf)==0) found = 1;
			// }
			// for(i=0; i < 50; i++) buf[i] = '\0';
			
			// if(found) strcpy(buf, "FOUND");
			// else strcpy(buf, "NOT-FOUND");
			// printf("Username %s\n", buf);

			// //Sending the message to the client whether the username exist in the database.
			// send(newsockfd, buf, strlen(buf) + 1, 0);
			// char buffer[5000];
			// for(int i = 0;i<5000;i++) buffer[i] = '\0';
			// if(found){

			// 	//accepting commands and executing them one by one
			// 	while(1){
			// 		for(int i = 0;i<5000;i++) buffer[i] = '\0';
			// 		int ending = 0;
			// 		int kk = 0;

			// 		//receiving the command in chunks of size 50 and concatenating them to a string of bigger size.
			// 		while(1){
			// 			int end = 0;
			// 			for(int i = 0;i<50;i++) buf[i]='\0';
			// 			recv(newsockfd, buf, 50, 0);
						
			// 			if(strcmp(buf, "exit\n")==0){
			// 				ending = 1;
			// 				break;
			// 			}
			// 			for(int i = 0;i<50;i++){
			// 				if(buf[i]=='\n'){
			// 					end = 1;
			// 					break;
			// 				}
			// 				buffer[kk++] = buf[i];
			// 			}
			// 			if(end) break;
			// 		}

			// 		if(ending) break;
			// 		printf("Message from the client: %s\n", buffer);

			// 		//Separating the command and argument from the received message 
			// 		char command[10], argument[5000];
			// 		for(int i = 0;i<10;i++) command[i] = '\0';
			// 		for(int i = 0;i<5000;i++) argument[i] = '\0';
			// 		int i = 0;
			// 		int k = 0;
			// 		while(buffer[i]!=' ' && buffer[i]!='\0' && k<10) command[k++] = buffer[i++];
			// 		if(buffer[i]==' ') i++;
			// 		k = 0;
			// 		while(buffer[i]!='\0') argument[k++] = buffer[i++];
			// 		printf("command: %s\n", command);
			// 		printf("argument: %s\n", argument);

			// 		if(strcmp(command, "pwd")==0){
			// 			if(getcwd(buffer, sizeof(buffer))==NULL){
			// 				strcpy(buffer, "####");
			// 			}
			// 		}
			// 		else if(strcmp(command, "dir")==0){
			// 			DIR *dir;
			// 			struct dirent *e;
			// 			if(argument[0]=='\0'){
			// 				strcpy(argument, "./");
			// 			}
			// 			if((dir=opendir(argument))==NULL){
			// 				strcpy(buffer, "####");
			// 			}else{
			// 				for(int i = 0;i<50;i++) buffer[i] = '\0';
			// 				while((e = readdir(dir))!=NULL){
			// 					char aa[50] = " ";
			// 					strcat(aa, e->d_name);
			// 					strcat(buffer,aa);
			// 				}
			// 				closedir(dir);
			// 			}
			// 		}
			// 		else if(strcmp(command, "cd")==0){
			// 			if(chdir(argument)) strcpy(buffer, "####");
			// 			else strcpy(buffer, "Changed directory successfully");
			// 		}
			// 		else{
			// 			strcpy(buffer, "$$$$");
			// 		}
			// 		// The result of the command or the error message to be sent to the client is stored in the 'buffer' string.
			// 		int ind = 0;

			// 		//sending the message to client in chunks of size 50
			// 		while(1){
			// 			int end = 0;
			// 			for(int i = 0;i<50;i++)  buf[i] = '\0';
			// 			for(int i = 0;i<50;i++){
			// 				if(buffer[ind]=='\0'){
			// 					end = 1;
			// 					break;
			// 				}
			// 				buf[i] = buffer[ind++];
			// 			}
			// 			send(newsockfd, buf, strlen(buf), 0);
			// 			if(end) break;
			// 		}
			// 		printf("Message sent to the client\n\n");
			// 	}
			// }
			printf("Client disconnected\n");
			close(newsockfd);
			exit(0);
		}

		close(newsockfd);
	}
	return 0;
}
			

