#ifndef SOCK_MyTCP
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

<<<<<<< HEAD
=======
pthread_mutex_t send_buf_mutex, received_buf_mutex;
pthread_cond_t received_active, send_active;

struct Queue *send_message, *received_message;
int global_socket;

>>>>>>> 1c6e543 (Revert "working")
int my_socket(int domain, int type, int protocol);
int my_bind(int socket, const struct sockaddr *address, socklen_t address_len);
int my_listen(int sockfd, int backlog);
int my_accept(int socket, struct sockaddr *address, socklen_t *address_len);
int my_connect(int sockfd, const struct sockaddr *address, socklen_t address_len);
ssize_t my_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t my_recv(int sockfd, void *buf, size_t len, int flags);
int my_close(int fd);

#endif