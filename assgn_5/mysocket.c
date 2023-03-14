#include "mysocket.h"

void *thread_S(void *arg){
    while(1){
        sleep(SLEEP_TIME);
        pthread_mutex_lock(&send_buf_mutex);
        pthread_cond_wait(&send_active, &send_buf_mutex);
        // check if any pending messages to be sent
        if((send_message != NULL && send_message->length == 0 )|| global_socket == -1){
            continue;
        }

        struct QNode *node = peek(send_message);
        assert(node != NULL);
        char *msg = node->buf;

        // repeat until msg is sent completely
        // keep track of bytes sent and send remaining bytes
        int bytes_sent = 0;
        int msg_len = node->msg_len - 2; // last 2 bytes are separators
        while(bytes_sent < msg_len){
            int bytes = send(global_socket, msg+bytes_sent, strlen(msg)-bytes_sent, 0);
            if(bytes == -1){
                perror("Error sending message.\n");
                exit(1);
            }
            bytes_sent += bytes;
        }

        pop(send_message);
        pthread_mutex_unlock(&send_buf_mutex);
    }
    pthread_exit(NULL);
}
void *thread_R(void *arg){
    while(1){
        sleep(SLEEP_TIME);
        pthread_mutex_lock(&received_buf_mutex);
        pthread_cond_wait(&received_active, &received_buf_mutex);
        // check if space available in received_message
        if((received_message != NULL && received_message->length == TABLE_SIZE) || global_socket == -1){
            continue;
        }

        char *msg = (char *)malloc(sizeof(char) * (MAX_MSG_SIZE+2));
        int bytes_received = 0;
        while(1){
            int bytes = recv(global_socket, msg+bytes_received, MAX_MSG_SIZE-bytes_received, 0);
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
        msg[bytes_received-2] = '\0';
        push(received_message, msg, bytes_received-2, MAX_MSG_SIZE+2);
        free(msg);
        pthread_mutex_unlock(&received_buf_mutex);
    }
    pthread_exit(NULL);
}

int my_socket(int domain, int type, int protocol){
    if(type != SOCK_MyTCP){
        perror("Invalid socket type.\n");
        exit(1);
    }

    send_message = createQueue();
    received_message = createQueue();
    global_socket = -1;

    send_active = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    received_active = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    send_buf_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    received_buf_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

    pthread_t R, S;
    pthread_create(&R, NULL, thread_R, NULL);
    pthread_create(&S, NULL, thread_S, NULL);
    pthread_join(R, NULL);
    pthread_join(S, NULL);

    global_socket = socket(domain, SOCK_STREAM, protocol);

    return global_socket;
}

int my_bind(int socket, const struct sockaddr *address, socklen_t address_len){
    return bind(socket, address, address_len);
}

int my_listen(int sockfd, int backlog){
    return listen(sockfd, backlog);
}

int my_accept(int socket, struct sockaddr *address, socklen_t *address_len){
    return accept(socket, address, address_len);
}

int my_connect(int sockfd, const struct sockaddr *address, socklen_t address_len){
    return connect(sockfd, address, address_len);
}

ssize_t my_send(int sockfd, const void *buf, size_t len, int flags){
    char *buf_copy = (char *)buf;
    
    // if queue is full, throw error
    if(send_message->length == TABLE_SIZE){
        perror("Unable to send. Buffer full.\n");
        exit(1);
    }

    // else, insert into send_message
    char *msg = (char *)malloc(sizeof(char) * (MAX_MSG_SIZE+2));
    for(int i=0; i<len; i++){
        msg[i] = buf_copy[i];
    }
    // add message boundary \r\n
    msg[len] = '\r';
    msg[len+1] = '\n';

    push(send_message, msg, len, MAX_MSG_SIZE+2);
    free(msg);
    
    // signal that message is sent
    pthread_cond_signal(&send_active);
    return len;
}

ssize_t my_recv(int sockfd, void *buf, size_t len, int flags){
    char *buf_copy = (char *)buf;
   
    // if queue is full, throw error
    if(received_message->length == 0){
        perror("Unable to receive. Buffer empty.\n");
        exit(1);
    }

    // else, pop from received_message
    struct QNode *node = peek(received_message);
    assert(node != NULL);
    char *msg = node->buf;
    for(int i=0; i<len; i++){
        buf_copy[i] = msg[i];
    }
    pop(received_message);
    
    // signal that the message has been received
    pthread_cond_signal(&received_active);
    return len;
}

int my_close(int fd){
    // empty the queues
    while(send_message->length != 0){
        pop(send_message);
    }
    while(received_message->length != 0){
        pop(received_message);
    }
    // free(send_message);
    // free(received_message);
    
    // delete any mutexes here
    return close(fd);
}