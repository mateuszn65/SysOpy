#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <ftw.h>
#include <dirent.h>

int count[3] = {0};
char *types[4] = {"file", "dir", "slink", ""};


int type_id(int type){
    switch(type){
        case FTW_F: return 0;
        case FTW_D: return 1;
        case FTW_SL: return 2;
        default: return 3;
    };
}
int nftw_handler(const char *dir_path, const struct stat *stat_buff, int flag_type, struct FTW* pwtw){
    int t_id = type_id(flag_type);
    if ( t_id == 3)
        return 0;

    printf("\nPath: %s\n", dir_path);
    printf("Number of links: %d\n", stat_buff->st_nlink);
    printf("Type: %s\n", types[t_id]);
    printf("Size: %ld\n", stat_buff->st_size);
    char buff[256];
    struct tm *access_time = localtime((const time_t *) &stat_buff->st_atime);
    strftime(buff, 256, "Last accessed: %c\n", access_time);
    printf("%s", buff);
    struct tm *modify_time = localtime((const time_t *) &stat_buff->st_atime);
    strftime(buff, 256, "Last modified: %c\n", modify_time);
    printf("%s", buff);
    if (t_id < 3)
        count[t_id]++;
    return 0;

}

void my_nftw(char *dir_path){
    nftw(dir_path, nftw_handler, 100, FTW_PHYS);
    printf("\nSumary:\n");
    for(int i = 0; i < 3; i++){
        printf("%s: %d\n", types[i], count[i]);
    }
}



int main(int argc, char **arg){
    char *dir_path = arg[1];
    my_nftw(dir_path);
    return 0;
}
