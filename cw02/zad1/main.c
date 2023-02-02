#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

void f_lib(char f1_name[], char f2_name[]){
    FILE *f1 = fopen(f1_name, "r");
    FILE *f2 = fopen(f2_name, "w");
    if (!f1 || !f2){
        printf("No access to files\n");
        exit(-1);
    }
    int count = 0;
    char c;
    while (fread(&c, 1, 1, f1) != 0){
        if (isspace(c)){
            count++;
            if (count > 1)
                continue;
        }else
            count = 0;
        fwrite(&c, 1, 1, f2);
    }
    fclose(f1);
    fclose(f2);
}
void f_sys(char f1_name[], char f2_name[]){
    int f1, f2;
    f1 = open(f1_name, O_RDONLY);
    f2 = open(f2_name, O_WRONLY);
    if (!f1 || !f2){
        printf("No access to files\n");
        exit(-1);
    }
    int count = 0;
    char c;
    while (read(f1, &c, 1) != 0){
        if (isspace(c)){
            count++;
            if (count > 1)
                continue;
        }else
            count = 0;
        write(f2, &c, 1);
    }
    close(f1);
    close(f2);
}

int main(int argc, char **arg){
    char f1_name[30], f2_name[30];
    if (argc != 3){
        printf("Enter name of the first file: ");
        scanf("%s", f1_name);
        printf("Enter name of the second file: ");
        scanf("%s", f2_name);
    }else{
        strcpy(f1_name, arg[1]);
        strcpy(f2_name, arg[2]);
    }
    FILE *t = fopen("pomiar_zad_1", "w");

    clock_t start = clock();
    f_lib(f1_name, f2_name);
    clock_t delta_lib = clock() - start;

    char buffer[100];
    int n = sprintf(buffer, "Time for library function: %fms\n", ((double) delta_lib)/CLOCKS_PER_SEC * 1000);
    fwrite(buffer, 1, n, t);
    fclose(t);

    start = clock();
    f_sys(f1_name, f2_name);
    clock_t delta_sys = clock() - start;

    t = fopen("pomiar_zad_1", "a+");
    n = sprintf(buffer, "Time for system function: %fms\n", ((double) delta_sys)/CLOCKS_PER_SEC * 1000);
    fwrite(buffer, 1, n, t);
    n = sprintf(buffer, "Time difference: %fms\n", ((double) abs(delta_lib-delta_sys))/CLOCKS_PER_SEC * 1000);
    fwrite(buffer, 1, n, t);
    fclose(t);
    return 0;
}
