
/*    THE CLIENT PROCESS */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

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
	int			lc_sockfd ;
	struct sockaddr_in	serv_addr1, serv_addr2;
    int         ls_sockfd, lbnewsockfd;
    struct sockaddr_in cli_addr, lbserv_addr;
    struct pollfd fd[1];


	int i;
	char *buf;
    buf = (char *)malloc(1000);

    if ((ls_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

    int port_no = atoi(argv[1]);
    int server_1_port = atoi(argv[2]);
    int server_2_port = atoi(argv[3]);   

	serv_addr1.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr1.sin_addr);
	serv_addr1.sin_port	= htons(server_1_port);

 	serv_addr2.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr2.sin_addr);
	serv_addr2.sin_port	= htons(server_2_port);   

    lbserv_addr.sin_family		= AF_INET;
	lbserv_addr.sin_addr.s_addr	= INADDR_ANY;
	lbserv_addr.sin_port		= htons(port_no);

    if (bind(ls_sockfd, (struct sockaddr *) &lbserv_addr,
					sizeof(lbserv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

    listen(ls_sockfd, 5);

    int load_s1= 0, load_s2= 0;
    while(1){
        printf("\n");
        int success = 1;
        if ((lc_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Unable to create socket\n");
            success = 0;
        }
        if(success && (connect(lc_sockfd, (struct sockaddr *) &serv_addr1 ,  sizeof(serv_addr1)))<0){
            perror("Unable to connect to server\n"); 
            success = 0;
        }
        if(success){
            for(int i = 0;i<1000;i++) buf[i] = '\0';

            strcpy(buf, "Send Load");
            send(lc_sockfd, buf, strlen(buf)+1, 0);
            recv_msg(buf, lc_sockfd, 1000);
            load_s1 = atoi(buf);
            printf("Load received from server %s %d: %d\n","127.0.0.1", server_1_port, load_s1);
        }
        close(lc_sockfd);
        success = 1;
        int lc_sockfd2;
        if ((lc_sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Unable to create socket\n");
            success = 0;
        }
        if(success && (connect(lc_sockfd2, (struct sockaddr *) &serv_addr2 ,  sizeof(serv_addr2)))<0){
            perror("Unable to connect to server\n"); 
            success = 0;
        }
        if(success){
            for(int i = 0;i<1000;i++) buf[i] = '\0';
            strcpy(buf, "Send Load");
            send(lc_sockfd2, buf, strlen(buf)+1, 0);
            recv_msg(buf, lc_sockfd2, 1000);
            load_s2 = atoi(buf);
            printf("Load received from server %s %d: %d\n","127.0.0.1", server_2_port, load_s2);
        }
        close(lc_sockfd2);
        fd[0].fd = ls_sockfd;
        fd[0].events = POLLIN;
        int nfd = 1;

        time_t start_t, end_t;
        int diff;
        start_t = time(NULL);
        end_t = start_t + 5;
        while(1){
            int diff = difftime(end_t,  start_t);
            // printf("%d\n", diff);
            int pr = poll(fd, nfd, diff*1000);
            start_t = time(NULL);
            if(pr==0){
                break;
            }
            else if(pr<0){
                printf("Error while polling\n");
                break;
            }
            else if(pr>0){
                int clilen = sizeof(cli_addr);
                lbnewsockfd = accept(ls_sockfd, (struct sockaddr *) &cli_addr, &clilen );
                if(lbnewsockfd<0){
                    printf("Error in connection with the client\n");
                    continue;
                }
                else{
                    printf("Client connected\n");
                }
                if(fork()==0){
                    close(ls_sockfd);
                    int lc_sockfd_tmp;
                    if ((lc_sockfd_tmp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                        perror("Unable to create socket\n");
                        success = 0;
                    }
                    struct sockaddr_in sa;
                    if(load_s1<load_s2) sa = serv_addr1;
                    else sa = serv_addr2;
                    if((connect(lc_sockfd_tmp, (struct sockaddr *) &sa ,  sizeof(sa)))<0){
                        perror("Unable to connect to server\n"); 
                        break;
                    }
                    strcpy(buf, "Send Time");

                    printf("Sending client request to %s %d\n", "127.0.0.1",  ntohs(sa.sin_port));   
                    send(lc_sockfd_tmp, buf, strlen(buf)+1, 0);
                    printf("Sent message: %s\n", buf);
                    recv_msg(buf, lc_sockfd_tmp, 1000);
                    printf("Message received: %s\n", buf);
                    close(lc_sockfd_tmp);

                    send(lbnewsockfd, buf, strlen(buf)+1, 0);
                    close(lbnewsockfd);
                    exit(0);
                }
                close(lbnewsockfd);
            }
        }                           
    }
    close(ls_sockfd);
    free(buf);
	return 0;

}

