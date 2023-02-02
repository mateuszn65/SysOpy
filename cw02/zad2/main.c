#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

void f_lib(char c, char file_name[]){
    FILE *f = fopen(file_name, "r");
    if (!f){
        printf("No access to file\n");
        exit(-1);
    }
    int count_c = 0, count_l = 0;
    char buff[256];
    int occurs = 0;
    while (fread(buff, 1, 256, f) != 0){
        for(int  i = 0; i < 256; i++){
            if(buff[i]==c){
                count_c++;
                if(!occurs){
                    count_l++;
                    occurs = 1;
                }
            }
            if(buff[i]=='\n')
                occurs = 0;
        }
    }
    fclose(f);
    printf("Number of lines: %d, number of occurences: %d\n", count_l, count_c);
}
void f_sys(char c, char file_name[]){
    int f;
    f = open(file_name, O_RDONLY);
    if (!f){
        printf("No access to file\n");
        exit(-1);
    }
    int count_c = 0, count_l = 0;
    char buff[256];
    int occurs = 0;
    while (read(f, buff, 256) != 0){
        for(int  i = 0; i < 256; i++){
            if(buff[i]==c){
                count_c++;
                if(!occurs){
                    count_l++;
                    occurs = 1;
                }
            }
            if(buff[i] == '\n')
                occurs = 0;
        }
    }
    close(f);
    printf("Number of lines: %d, number of occurances: %d\n", count_l, count_c);
}

int main(int argc, char **arg){
    char c, file_name[30];
    if (argc != 3){
        printf("Expected 2 arguments, got %d ", argc-1);
        exit(-1);
    }else{
        strcpy(&c, arg[1]);
        strcpy(file_name, arg[2]);
    }
    FILE *t = fopen("pomiar_zad_2", "w");

    clock_t start = clock();
    f_lib(c, file_name);
    clock_t delta_lib = clock() - start;

    char buffer[100];
    int n = sprintf(buffer, "Time for library function: %fms\n", ((double) delta_lib)/CLOCKS_PER_SEC * 1000);
    fwrite(buffer, 1, n, t);
    fclose(t);
    start = clock();
    f_sys(c, file_name);
    clock_t delta_sys = clock() - start;
    t = fopen("pomiar_zad_2", "a+");
    n = sprintf(buffer, "Time for system function: %fms\n", ((double) delta_sys)/CLOCKS_PER_SEC * 1000);
    fwrite(buffer, 1, n, t);
    n = sprintf(buffer, "Time difference: %fms\n", ((double) abs(delta_lib-delta_sys))/CLOCKS_PER_SEC * 1000);
    fwrite(buffer, 1, n, t);
    fclose(t);
    return 0;
}

