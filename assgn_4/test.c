#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void split_command(char *msg, size_t len, char **split){
    // printf("%zu\n", len);
    int i;
    size_t cmd_ptr=0, idx=0;
    for(i=0; i<len; i++){
        if(msg[i] == '\n'){
            split[cmd_ptr][idx++] = '\0';
        }
        else if(msg[i] != ' '){
            split[cmd_ptr][idx++] = msg[i];
        }
        else{
            split[cmd_ptr][idx++] = '\0';
            cmd_ptr++;
            idx = 0;
        }
    }
}

int main(){
    char msg[100] = "GET http://ag.com/index.html\n", **cmd_list;
    cmd_list = (char **)malloc(sizeof(char *) * 3);
    for(int i=0; i<3; i++){
        cmd_list[i] = (char *)malloc(sizeof(char) * 100);
    }
    split_command(msg, strlen(msg), cmd_list);

    for(int i=0; i<3; i++){
        printf("%s\n", cmd_list[i]);
    }

    char url[50] = "hello:8080";
    size_t ptr=strlen(url)-1;
    
    while( isdigit(url[ptr]) ) ptr--;
    
    ptr++;
    printf("%zu\n", ptr);

    char port_str[10];
    strcpy(port_str, url+ptr);

    printf("%s\n", port_str);
    int port;
    sscanf(port_str, "%d", &port);
    printf("%d\n", port);

    return 0;
}