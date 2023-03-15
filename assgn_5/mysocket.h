#ifndef SOCKET_HEADER
#define SOCKET_HEADER

#define SOCK_MyTCP 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/poll.h>
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <pthread.h>
#include <assert.h>
#include "queue.h"

#define SLEEP_TIME 4
#define TABLE_SIZE 10
#define MAX_MSG_SIZE 5000 // 5000 bytes

int my_socket(int domain, int type, int protocol);
int my_bind(int socket, const struct sockaddr *address, socklen_t address_len);
int my_listen(int sockfd, int backlog);
int my_accept(int socket, struct sockaddr *address, socklen_t *address_len);
int my_connect(int sockfd, const struct sockaddr *address, socklen_t address_len);
ssize_t my_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t my_recv(int sockfd, void *buf, size_t len, int flags);
int my_close(int fd);



void *thread_S(void *arg);
void *thread_R(void *arg);
#endif