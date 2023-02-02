#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

int type_id(int type){
    switch(type){
        case 8: return 0;
        case 4: return 1;
        case 2: return 2;
        case 6: return 3;
        case 1: return 4;
        case 10: return 5;
        case 12: return 6;
        default: return 7;
    };
}
void my_stat_rec(char *dir_path, int count[], char **types){
    DIR* dir = opendir(dir_path);
    if (!dir){
        printf("No access to directory\n");
        return;
    }
    struct dirent *file;
    char new_path[256];
    char abs_path[256];
    while((file = readdir(dir))){
        strcpy(new_path, dir_path);
        strcat(new_path, "/");
        strcat(new_path, file->d_name);
        realpath(new_path, abs_path);
        struct stat stat_buff;

        if(lstat(new_path, &stat_buff) < 0){
            printf("No access to %s\n", new_path);
            exit(-1);
        }
        if (strcmp(file->d_name, ".") && strcmp(file->d_name, "..")){
            printf("\nPath: %s\n", abs_path);
            printf("Number of links: %d\n", stat_buff.st_nlink);
            int t_id = type_id(file->d_type);
            printf("Type: %s\n", types[t_id]);
            printf("Size: %ld\n", stat_buff.st_size);
            char buff[256];
            struct tm *access_time = localtime((const time_t *) &stat_buff.st_atime);
            strftime(buff, 256, "Last accessed: %c\n", access_time);
            printf("%s", buff);
            struct tm *modify_time = localtime((const time_t *) &stat_buff.st_atime);
            strftime(buff, 256, "Last modified: %c\n", modify_time);
            printf("%s", buff);
            if (t_id < 7)
                count[t_id]++;

            if(type_id(file->d_type) == 1)
                my_stat_rec(new_path, count, types);
        }

    }
    closedir(dir);
}

void my_stat(char *dir_path){
    int count[7] = {0};
    char *types[8] = {"file", "dir", "char dev", "block dev", "fifo", "slink", "sock", ""};
    my_stat_rec(dir_path, count, types);
    printf("\nSumary:\n");
    for(int i = 0; i < 7; i++){
        printf("%s: %d\n", types[i], count[i]);
    }
}

int main(int argc, char **arg){
    char *dir_path = arg[1];
    my_stat(dir_path);
    return 0;
}
