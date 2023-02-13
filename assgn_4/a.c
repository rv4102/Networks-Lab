#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    size_t n=0;
    int fd = open("IMG_5968.heic", O_RDONLY);
    if(fd == -1){
        printf("Error opening file");
        exit(0);
    }
    int fd_new = open("out.heic", O_WRONLY | O_CREAT, 0666);
    char buffer[100];
    while((n = read(fd, buffer, 100)) > 0){
        printf("%s", buffer);
    }
    close(fd);
    close(fd_new);


    return 0;
}