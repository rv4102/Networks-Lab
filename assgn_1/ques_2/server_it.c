
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
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[20];		/* We will use this buffer for communication */

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5); 

	printf("Server running\n");
	while (1) {

		printf("\nWaiting for the client connection\n");
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}

		printf("Client connected\n");

		while(1){
			printf("\nWaiting for the expression\n");
			char token[20];
			int operat;
			double num = 0;
			int m = 1;
			int divi = 0;
			int brack_open = 0;
			double ans_first = 0;
			double ans_sec = 0;
			char prev_op = '+';
			int first_char = 1;
			char brac_op = '+';
			int ex = 0;

			//receiving chunks of expression from the client and solving the expression simultaneously
			while(1){
				for(int i = 0;i<20;i++) buf[i] = '\0';
				int n = recv(newsockfd, buf, 20, 0);
				if(first_char && buf[0]=='e'){
					close(newsockfdw);
					ex = 1;
					break;
				}
				first_char = 0;
				int ending = 0;
				int last_in = 19;
				for(int i = 0;i<20;i++) if(buf[i]=='\n'){
					ending = 1;
					last_in = i;
					break;
				}

				for(int i = 0;i<=last_in;i++){
					if(buf[i]<='9' && buf[i]>='0'){
						if(divi) {num = num+((double)(buf[i]-'0'))/m; m*=10;}
						else num = num*10 + (buf[i]-'0');
					}
					else if(buf[i]=='.'){
						divi = 1;
						m = 10;
					}
					else if(buf[i]=='('){
						brack_open = 1;
						brac_op = prev_op;
						prev_op = '+';
						ans_sec = 0;
					}
					else if(buf[i]==')'){
						if(prev_op=='+'){
							if(!brack_open) ans_first += num;
							else ans_sec += num;
							num = 0;
							divi = 0;
						}
						else if(prev_op=='-'){
							if(!brack_open) ans_first -= num;
							else ans_sec -= num;
							num = 0;
							divi = 0;
						}
						else if(prev_op=='/'){
							if(!brack_open) ans_first /= num;
							else ans_sec /= num;
							num = 0;
							divi = 0;
						}
						else if(prev_op=='*'){
							if(!brack_open) ans_first *= num;
							else ans_sec *= num;
							num = 0;
							divi = 0;
						}

						if(brac_op=='+'){
							ans_first += ans_sec;
							num = 0;
							divi = 0;
						}
						else if(brac_op=='-'){
							ans_first-=ans_sec;
							num = 0;
							divi = 0;
						}
						else if(brac_op=='/'){
							ans_first/=ans_sec;
							num = 0;
							divi = 0;
						}
						else if(brac_op=='*'){
							ans_first*=ans_sec;
							num = 0;
							divi = 0;
						}
						brack_open = 0;
						prev_op = '+';
						ans_sec = 0;
					}
					else{
						if(prev_op=='+'){
							if(!brack_open) ans_first += num;
							else ans_sec += num;
							num = 0;
							divi = 0;
						}
						else if(prev_op=='-'){
							if(!brack_open) ans_first -= num;
							else ans_sec -= num;
							num = 0;
							divi = 0;
						}
						else if(prev_op=='/'){
							if(!brack_open) ans_first /= num;
							else ans_sec /= num;
							num = 0;
							divi = 0;
						}
						else if(prev_op=='*'){
							if(!brack_open) ans_first *= num;
							else ans_sec *= num;
							num = 0;
							divi = 0;
						}
						if(buf[i]!='\n') prev_op = buf[i];
					}
				}
				if(ending){
					break;
				}
			}
			if(ex){
				printf("Client disconnencted\n");
				break;
			}

			char str[20];
			sprintf(str, "%f", ans_first);
			printf("Received the expression from the client\n");
			printf("Answer: %s\n", str);
			int len = (int)strlen(str);
			send(newsockfd, str, len, 0);
			printf("Answer sent to the client\n");
		}

	}
	return 0;
}
			

