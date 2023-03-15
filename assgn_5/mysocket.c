#include "mysocket.h"

<<<<<<< HEAD
<<<<<<< HEAD
pthread_mutex_t send_buf_mutex, received_buf_mutex;
pthread_cond_t received_active, send_active;

struct Queue *send_message, *received_message;
int send_sockfd, recv_sockfd;
=======
pthread_mutex_t send_buf_mutex, received_buf_mutex;
pthread_cond_t received_active, send_active;
pthread_cond_t recv_pushed, send_popped;
pthread_t R, S;

struct Queue *send_message, *received_message;
int global_socket;
>>>>>>> ad7c317 (done part1)

char *remaining_string;
int rem_string_index;


<<<<<<< HEAD
=======
>>>>>>> 1c6e543 (Revert "working")
=======
>>>>>>> ad7c317 (done part1)
void *thread_S(void *arg){
    while(1){
        sleep(SLEEP_TIME);
        pthread_mutex_lock(&send_buf_mutex);
        while((send_message != NULL && send_message->length == 0 ) || global_socket == -1){
            pthread_cond_wait(&send_active, &send_buf_mutex);
        }

        struct QNode *node = peek(send_message);
        assert(node != NULL);
        char *msg = node->buf;
<<<<<<< HEAD
<<<<<<< HEAD
        // printf("msg to be sent: %s\n", msg);
        // repeat until msg is sent completely
        // keep track of bytes sent and send remaining bytes
        int bytes_sent = 0;
        int msg_len = node->msg_len;
=======
=======
        printf("msg to be sent: %s\n", msg);
>>>>>>> ad7c317 (done part1)

        // repeat until msg is sent completely
        // keep track of bytes sent and send remaining bytes
        int bytes_sent = 0;
        int msg_len = node->msg_len - 2; // last 2 bytes are separators
>>>>>>> 1c6e543 (Revert "working")
        while(bytes_sent < msg_len){
            int bytes = send(global_socket, msg+bytes_sent, msg_len-bytes_sent, 0);
            if(bytes == -1){
                perror("Error sending message.\n");
                exit(1);
            }
            bytes_sent += bytes;
        }

        pop(send_message);
        printf("Message sent of length: %d\n", bytes_sent);
        pthread_mutex_unlock(&send_buf_mutex);
        pthread_cond_signal(&send_popped);
    }

    printf("Thread S exiting\n");
    pthread_exit(NULL);
}
void *thread_R(void *arg){
<<<<<<< HEAD
    int *temp = (int *)arg;
    int sockfd = *temp;
    printf("Thread R started with sockfd: %d\n", sockfd);
=======
>>>>>>> 1c6e543 (Revert "working")
    while(1){
        sleep(SLEEP_TIME);
        pthread_mutex_lock(&received_buf_mutex);

        // check if space available in received_message
        while((received_message != NULL && received_message->length == TABLE_SIZE) || global_socket == -1){
            pthread_cond_wait(&received_active, &received_buf_mutex);
        }

        char *msg = (char *)malloc(sizeof(char) * (MAX_MSG_SIZE+2));
        memset(msg, '\0', MAX_MSG_SIZE+2);
        for(int i = 0; i<rem_string_index; i++){
            msg[i] = remaining_string[i];
        }

        int bytes_received = rem_string_index;

        memset(remaining_string, '\0', MAX_MSG_SIZE+2);
        rem_string_index = 0;
    
        int r_received = 0;
        int end_recv = 0;
        int ending_index = 0;

        while(1){
<<<<<<< HEAD
<<<<<<< HEAD
            int bytes = recv(sockfd, msg+bytes_received, MAX_MSG_SIZE+2-bytes_received, 0);
=======
            int bytes = recv(global_socket, msg+bytes_received, MAX_MSG_SIZE-bytes_received, 0);
>>>>>>> 1c6e543 (Revert "working")
=======
            int bytes = recv(global_socket, msg+bytes_received, MAX_MSG_SIZE+2-bytes_received, 0);
            if(bytes == 0){
                printf("Connection closed by server\n");
                pthread_mutex_unlock(&received_buf_mutex);
                pthread_exit(NULL);
            }
>>>>>>> ad7c317 (done part1)
            if(bytes == -1){
                perror("Error receiving message.\n");
                exit(1);
            }
            bytes_received += bytes;
            if(msg[bytes_received-2] == '\r' && msg[bytes_received-1] == '\n'){
                break;
            }
        }

        // remove message boundary \r\n
<<<<<<< HEAD
<<<<<<< HEAD
        msg[ending_index+1] = '\0';
        msg[ending_index+2] = '\0';
        push(received_message, msg, ending_index, MAX_MSG_SIZE+2);
        free(msg);
        break;
        // pthread_mutex_unlock(&received_buf_mutex);
=======
        msg[bytes_received-2] = '\0';
        push(received_message, msg, bytes_received-2, MAX_MSG_SIZE+2);
        free(msg);
        pthread_mutex_unlock(&received_buf_mutex);
>>>>>>> 1c6e543 (Revert "working")
=======
        push(received_message, msg, ending_index, MAX_MSG_SIZE+2);
        printf("Message received: %s\n", msg);
        printf("Message received of length: %d\n", ending_index);
        free(msg);
        pthread_mutex_unlock(&received_buf_mutex);
        pthread_cond_signal(&recv_pushed);
>>>>>>> ad7c317 (done part1)
    }
    printf("Thread R exiting\n");
    pthread_exit(NULL);
}

int my_socket(int domain, int type, int protocol){
    if(type != SOCK_MyTCP){
        perror("Invalid socket type\n");
        exit(1);
    }

    send_message = createQueue();
    received_message = createQueue();
<<<<<<< HEAD
    send_sockfd = -1;
    recv_sockfd = -1;
    remaining_string = (char *)malloc(sizeof(char) * (MAX_MSG_SIZE+2));
    memset(remaining_string, '\0', MAX_MSG_SIZE+2);
    rem_string_index = 0;
=======
    global_socket = -1;
>>>>>>> 1c6e543 (Revert "working")

    send_active = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    received_active = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    recv_pushed = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    send_popped = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    send_buf_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    received_buf_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;


    pthread_create(&R, NULL, thread_R, NULL);
    pthread_create(&S, NULL, thread_S, NULL);

    return socket(domain, SOCK_STREAM, protocol); 
}

int my_bind(int socket, const struct sockaddr *address, socklen_t address_len){
    return bind(socket, address, address_len);
}

int my_listen(int sockfd, int backlog){
    return listen(sockfd, backlog);
}

int my_accept(int socket, struct sockaddr *address, socklen_t *address_len){
    global_socket = accept(socket, address, address_len);
    pthread_cond_signal(&received_active);
    pthread_cond_signal(&send_active);
    return global_socket;
}

int my_connect(int sockfd, const struct sockaddr *address, socklen_t address_len){
    global_socket = sockfd;
    return connect(sockfd, address, address_len);
}

ssize_t my_send(int sockfd, const void *buf, size_t len, int flags){
    char *buf_copy = (char *)buf;
    
    // if queue is full, throw error
    pthread_mutex_lock(&send_buf_mutex);
    while(send_message->length == TABLE_SIZE){
        pthread_cond_wait(&send_popped, &send_buf_mutex);
    }

    // else, insert into send_message
    char *msg = (char *)malloc(sizeof(char) * (MAX_MSG_SIZE+2));
    for(int i=0; i<len; i++){
        msg[i] = buf_copy[i];
    }
    // add message boundary \r\n
    msg[len] = '\r';
    msg[len+1] = '\n';

<<<<<<< HEAD
<<<<<<< HEAD
    push(send_message, msg, len+2, MAX_MSG_SIZE+2);
    // printf("Sent: %s", msg);
    struct QNode *node = peek(send_message);
    assert(node != NULL);
    printf("Sent: %s", node->buf);
=======
    push(send_message, msg, len, MAX_MSG_SIZE+2);
>>>>>>> 1c6e543 (Revert "working")
    free(msg);
    
    // signal that message is sent
=======
    push(send_message, msg, len+2, MAX_MSG_SIZE+2);
    pthread_mutex_unlock(&send_buf_mutex);
>>>>>>> ad7c317 (done part1)
    pthread_cond_signal(&send_active);
    free(msg);
    return len;
}

ssize_t my_recv(int sockfd, void *buf, size_t len, int flags){
    char *buf_copy = (char *)buf;
   
    // if queue is full, throw error

    pthread_mutex_lock(&received_buf_mutex);
    while(received_message->length == 0){
        pthread_cond_wait(&recv_pushed, &received_buf_mutex);
    }

    // else, pop from received_message
    struct QNode *node = peek(received_message);
    assert(node != NULL);
    char *msg = node->buf;
    for(int i=0; i<len; i++){
        buf_copy[i] = msg[i];
    }
    pop(received_message);
<<<<<<< HEAD
<<<<<<< HEAD

=======
    
>>>>>>> 1c6e543 (Revert "working")
=======
    pthread_mutex_unlock(&received_buf_mutex);

>>>>>>> ad7c317 (done part1)
    // signal that the message has been received
    pthread_cond_signal(&received_active);

    return len;
}

int my_close(int fd){
    sleep(5);
    // empty the queues
    while(send_message->length != 0){
        pop(send_message);
    }
    while(received_message->length != 0){
        pop(received_message);
    }
<<<<<<< HEAD
    free(send_message);
    free(received_message);
    free(remaining_string);
    
=======

    pthread_cancel(R);
    pthread_cancel(S);
    pthread_join(R, NULL);
    pthread_join(S, NULL);

>>>>>>> ad7c317 (done part1)
    // delete any mutexes here
    pthread_mutex_destroy(&send_buf_mutex);
    pthread_mutex_destroy(&received_buf_mutex);
    pthread_cond_destroy(&send_active);
    pthread_cond_destroy(&received_active);

    free(send_message);
    free(received_message);
    free(remaining_string);
    return close(fd);
}