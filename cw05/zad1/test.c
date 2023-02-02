#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


int main (int argc, char *argv[]){
    struct stat buf;
    char file[] = "main.c";


    if(stat(file, &buf) == 0){
        printf("%d\n", buf.st_mode);
        if(S_ISREG(buf.st_mode)){
            printf("chuj2");
        }
        if(S_ISDIR (buf.st_mode))
            printf("chuj");
        printf("%d", buf.st_ino);
    }

    return 0;
}
