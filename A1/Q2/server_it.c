#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

#define STACK_SIZE 200
#define RET_SIZE 30
#define BUF_SIZE 200

// define the stack here
int top_nums = -1, top_ops = -1;
double nums[STACK_SIZE];
char ops[STACK_SIZE], ret[RET_SIZE];

void push_nums(double x){
    if (top_nums == STACK_SIZE - 1){
        printf("Overflow!!\n");
    }
    else
    {
        top_nums = top_nums + 1;
        nums[top_nums] = x;
    }
}
void pop_nums(){
    if (top_nums == -1){
        printf("Underflow!!\n");
    }
    else{
        // printf("\nPopped element: %d", nums[top_nums]);
        top_nums = top_nums - 1;
    }
}
void push_ops(char c){
    if(top_ops == STACK_SIZE - 1){
        printf("Overflow!\n");
    }
    else{
        top_ops = top_ops+1;
        ops[top_ops] = c;
    }
}
void pop_ops(){
    if(top_ops == -1){
        printf("Underflow!\n");
    }
    else{
        top_ops = top_ops - 1;
    }
}
char peek_ops(){
    return ops[top_ops];
}
double peek_nums(){
    return nums[top_nums];
}
void reset_stacks(){
    top_nums = -1;
    top_ops = -1;
}

double performOp(){
    double a, b;
    char op;
    a = peek_nums(); pop_nums();
    b = peek_nums(); pop_nums();
    op = peek_ops(); pop_ops();

    switch(op){
        case '+':
            return a+b;
        case '-':
            return b-a;
        case '*':
            return a*b;
        case '/':
            if(a == 0){
                perror("Divide by 0 error.\n");
                exit(0);
            }
            return b/a;
    }
    return 0;
}

int precedence(char c){
    switch(c){
        case '+':
        case '-':
        case '*':
        case '/':
            return 1;
    }
    return -1;
}

double evaluate(char *expr, size_t len){
    size_t i, j, st;
    char c, num[RET_SIZE];
    double d, temp1, temp2, ans;
    //  evaluate the arithmetic expression stored in *expr
    for(i=0; expr[i] != '\0'; i++){
        c = expr[i];
        if(isdigit(c) || c=='.'){
            st = i;
            while(isdigit(c) || c=='.'){
                i++;
                if(i<len)
                    c = expr[i];
                else
                    break;
            }
            i--;
            // empty the num
            for(j=0; j<RET_SIZE; j++)
                num[j] = '\0';
            strncpy(num, expr+st, i-st+1);
            sscanf(num, "%lf", &d);

            // push to operand stack
            push_nums(d);
        }
        // if the character c is an operator, push it to the operator stack
        else if(c == '(')
            push_ops(c);
        else if(c == ')'){
            while(peek_ops() != '('){
                temp1 = performOp();
                push_nums(temp1);
            }
            pop_ops();
        }
        else if(c=='+' || c=='-' || c=='*' || c=='/'){
            while(!(top_ops == -1) && precedence(c)<=precedence(peek_ops())){
                temp2 = performOp();
                push_nums(temp2);
            }
            push_ops(c);
        }
    }
    while(!(top_ops == -1)){
        temp1 = performOp();
        push_nums(temp1);
    }

    ans = peek_nums();
    pop_nums();

    return ans;
}

int main() {
    int sockfd, newsockfd, len; /* Socket descriptors */
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;

    size_t sz = BUF_SIZE, ptr, i, end, more_inp=1;
	char packet[BUF_SIZE], *expr = NULL, result[RET_SIZE];
    expr = (char *)(malloc(sizeof(char) * sz));
    double ans;

    // open a socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Unable to create socket!\n");
        exit(0);
    }
    printf("Socket successfully created.\n");

    // specify the server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    // bind the address to socket
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("Unable to bind address to socket!\n");
        exit(0);
    }
    printf("Socket successfully binded to address.\n");

    // listen for clients trying to connect to this server
    listen(sockfd, 5); /* this specifies that upto 5 concurrent client 
                        requests will be queued up while the system 
                        is executing the "accept" call */
    printf("Server listening ...\n");
    while(1){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if(newsockfd < 0){
            perror("Unable to accept client's connect request.\n");
            exit(0);
        }
        printf("Server has connected to a client.\n");

        more_inp = 1;
        // while the user wants to send more input
        while(more_inp){
            ptr = 0;
            end = 0;
            // while there are more characters in the current input
            while(!end){
                if(recv(newsockfd, packet, BUF_SIZE, 0) == 0){
                    perror("Unable to receive data from client.\n");
                    exit(0);
                }

                // check if end
                if(atoi(packet) == -1){
                    // close the client now
                    more_inp = 0;
                    break;
                }

                // reallocate the array if needed
                if(ptr >= sz){
                    expr = realloc(expr, sizeof(char) * sz);
                    sz += BUF_SIZE;
                }

                // check if this is the last packet
                for(i=0; i<BUF_SIZE; i++){
                    if(packet[i]=='\0'){
                        end = 1;
                    }
                }

                // take this input and allocate it to an expression array such that spaces are removed
                if(!end){
                    for(i=0; i<BUF_SIZE; i++){
                        if(packet[i] != ' '){
                            expr[ptr++] = packet[i];
                        }
                    }
                }
                else{
                    for(i=0; i<strlen(packet)+1; i++){
                        if(packet[i] != ' '){
                            expr[ptr++] = packet[i];
                        }
                    }
                }
            }

            // check if there should be no more inputs (i.e. -1 was sent)
            if(!more_inp){
                break;
            }
            
            ans = evaluate(expr, strlen(expr)+1);
            // convert this double to string
            snprintf(result, RET_SIZE, "%lf", ans);

            // send the result to the client
            if(send(newsockfd, result, strlen(result)+1, 0) != strlen(result)+1){
                perror("Unable to send the result back to the client.\n");
                exit(0);
            }
            reset_stacks();
        }
        if(close(newsockfd) < 0){
            perror("Unable to close socket.\n");
            exit(0);
        }
        printf("Connection to client has been closed.\n");
    }
    return 0;
}