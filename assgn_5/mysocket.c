#include "mysocket.h"

pthread_mutex_t send_buf_mutex, received_buf_mutex;
pthread_cond_t received_active, send_active;

struct Queue *send_message, *received_message;
int global_socket;

char *remaining_string;
int rem_string_index;


void *thread_S(void *arg){
    int *temp = (int *)arg;
    int sockfd = *temp;
    printf("Thread S started with sockfd: %d\n", sockfd);
    while(1){
        sleep(SLEEP_TIME);
        // pthread_mutex_lock(&send_buf_mutex);
        // check if any pending messages to be sent
        if((send_message != NULL && send_message->length == 0 )){
        // pthread_cond_wait(&send_active, &send_buf_mutex);
            // continue;
            break;
        }

        struct QNode *node = peek(send_message);
        assert(node != NULL);
        char *msg = node->buf;
        printf("msg to be sent: %s\n", msg);
        // repeat until msg is sent completely
        // keep track of bytes sent and send remaining bytes
        int bytes_sent = 0;
        int msg_len = node->msg_len; // last 2 bytes are separators
        while(bytes_sent < msg_len){
            int bytes = send(sockfd, msg+bytes_sent, msg_len-bytes_sent, 0);
            if(bytes == -1){
                perror("Error sending message.\n");
                exit(1);
            }
            bytes_sent += bytes;
        }

        pop(send_message);
        printf("sent: %d\n", bytes_sent);
        // pthread_mutex_unlock(&send_buf_mutex);
    }

    printf("Thread S exiting\n");
    // pthread_exit(NULL);
}
void *thread_R(void *arg){
    int *temp = (int *)arg;
    int sockfd = *temp;
    while(1){
        sleep(SLEEP_TIME);
        // pthread_mutex_lock(&received_buf_mutex);
        // check if space available in received_message
        if((received_message != NULL && received_message->length == TABLE_SIZE)){
        // pthread_cond_wait(&received_active, &received_buf_mutex);
            // continue;
            break;
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
            int bytes = recv(sockfd, msg+bytes_received, MAX_MSG_SIZE+2-bytes_received, 0);
            printf("msg received: %s\n", msg);
            if(bytes == -1){
                perror("Error receiving message.\n");
                exit(1);
            }
            for(int i = bytes_received; i<bytes_received+bytes; i++){
                if(msg[i] == '\r'){
                    r_received = 1;
                }
                else if(msg[i] == '\n' && r_received == 1){
                    r_received = 0;
                    end_recv = 1;
                    ending_index = i-1;
                    break;
                }
                else{
                    r_received = 0;
                }
            }
            for(int i = ending_index+2; i<bytes_received + bytes;i++){
                remaining_string[rem_string_index++] = msg[i];
            }
            bytes_received += bytes;
            if(end_recv == 1){
                break;
            }
        }

        // remove message boundary \r\n
        // msg[bytes_received-2] = '\0';
        push(received_message, msg, ending_index, MAX_MSG_SIZE+2);
        free(msg);
        printf("done\n");
        break;
        // pthread_mutex_unlock(&received_buf_mutex);
    }
    printf("Thread R exiting\n");
    // pthread_exit(NULL);
}

int my_socket(int domain, int type, int protocol){
    if(type != SOCK_MyTCP){
        perror("Invalid socket type.\n");
        exit(1);
    }

    send_message = createQueue();
    received_message = createQueue();
    global_socket = -1;
    remaining_string = (char *)malloc(sizeof(char) * (MAX_MSG_SIZE+2));
    memset(remaining_string, '\0', MAX_MSG_SIZE+2);
    rem_string_index = 0;

    // send_active = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    // received_active = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    // send_buf_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    // received_buf_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

    // pthread_t R, S;
    // pthread_create(&R, NULL, thread_R, NULL);
    // pthread_create(&S, NULL, thread_S, NULL);
    // pthread_join(R, NULL);
    // pthread_join(S, NULL);


    
    return socket(domain, SOCK_STREAM, protocol); 
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

    push(send_message, msg, len+2, MAX_MSG_SIZE+2);
    // printf("Sent: %s", msg);
    printf("length: %d\n", send_message->length);
    struct QNode *node = peek(send_message);
    assert(node != NULL);
    printf("Sent: %s", node->buf);
    printf("length: %d\n", send_message->length);
    free(msg);
    
    // signal that message is sent
    // pthread_cond_signal(&send_active);
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
    printf("Received: %s", buf_copy);
    // signal that the message has been received
    // pthread_cond_signal(&received_active);
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