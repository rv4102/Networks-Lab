#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define _POSIX_SOURCE
#include <dirent.h>

#define BUFSIZE 500

int valid_username(char *username)
{
    // check username against list of usernames
    FILE *fp;
    char line[BUFSIZE];

    fp = fopen("users.txt", "r");
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        if (strcmp(line, username) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int main()
{
    char text[BUFSIZE];
    char cmd[5], args[200]="\0";
    strcpy(text, "dir /Users/rushilv/");

    int nargs = sscanf(text, "%s %s", cmd, args);
    strcpy(text, "\0");

    printf("%s\n", args);

    DIR *dir;
    struct dirent *de; // Pointer for directory entry

    // dir command accepts a path as an argument
    // if there is an argument, open that directory
    if (nargs>1)
        dir = opendir(args);
    // else open current directory
    else
        dir = opendir(".");

    // traverse this directory
    if (dir == NULL)
        strcpy(text, "####");
    else
    {
        while ((de = readdir(dir)) != NULL)
        {
            // printf("%s\n", de->d_name);
            strcat(text, de->d_name);
            strcat(text, "\n");
        }
    }
    closedir(dir);

    printf("%s\n", text);

    // char *inp1 = "cd bakchod", *inp2 = "cd ", cmd[50], args[50] = "\0";
    // sscanf(inp1, "%s %s", cmd, args);
    // printf("%s, %s\n", cmd, args);
    // // sscanf(inp2, "%s %s", cmd, args);
    // // printf("%s\n%s", cmd, args);
    // if (args[0] == '\0')
    // {
    //     printf("yesss\n");
    // }
    // // char s1[15]="hi\n", s2[15]="\nthere.\n";
    // // strcat(s1, s2);
    // // printf("%s", s1);

    // // char msg[BUFSIZE] = "rushi";
    // // printf("%d", valid_username(msg));

    return 0;
}