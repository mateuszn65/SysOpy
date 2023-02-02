#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>

bool is_txt(char* path){
    if(strlen(path) < 5)
        return false;
    if(strcmp(path+strlen(path)-4, ".txt"))
        return false;
    return true;
}
bool contains_pattern(char* path, char* pattern){
    FILE *f = fopen(path, "r");
    if (!f)
        return false;
    char *buff;
    size_t len = 0;
    while (getline(&buff, &len, f) != -1){
        if(strstr(buff, pattern)){
            free(buff);
            fclose(f);
            return true;
        }
    }
    free(buff);
    fclose(f);
    return false;

}

void searchf(char* curr_dir, char* pattern, int max_depth, char* total_path){

    DIR* dir = opendir(total_path);
    if (!dir){
        printf("No access to directory\n");
        return;
    }
    struct dirent *file;
    char new_path[1000];

    while((file = readdir(dir))){
        if (strcmp(file->d_name, ".") && strcmp(file->d_name, "..")){
            sprintf(new_path, "%s/%s", total_path, file->d_name);
            if(file->d_type == 4){
                pid_t child = fork();
                if (child == 0){
                    char max_depth_str[256];
                    sprintf(max_depth_str, "%d", max_depth - 1);
                    execlp("./main", "./main", file->d_name, pattern, max_depth_str, new_path, NULL);
                }else{
                    while(wait(NULL) > 0){}
                }

            }else{
                if(is_txt(new_path)){
                    printf("%s pid: %d\n", new_path, (int)getpid());
                    if (contains_pattern(new_path, pattern)){
                        printf("Znaleziono pattern w %s\n", file->d_name);
                    }else{
                        printf("Nie znaleziono patternu w %s\n", file->d_name);
                    }

                }
            }
        }
    }
    closedir(dir);
}


int main(int argc, char **argv){
    if(argc < 4){
        printf("Za malo argumentow\n");
        return 0;
    }

    char* curr_dir = argv[1];
    char* pattern = argv[2];
    int max_depth = atoi(argv[3]);

    if(max_depth < 0)
        return 0;

    char* total_path = argv[1];
    if (argc > 4){
        total_path = argv[4];
    }
    //printf("curr_dir: %s\tpattern: %s\tmd: %d\ttotal_path: %s\n", curr_dir, pattern, max_depth, total_path);
    searchf(curr_dir, pattern, max_depth, total_path);
    return 0;
}
