#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
int main(int argc, char ** argv){
    time_t s,e;
    double dif;
    time (&s);
    if (argc != 3){
        printf("Niepoprawna liczba argumentow\n");
        return -1;
    }
    float dx = atof(argv[1]);
    int n = atoi(argv[2]);
    if (n < 0 || dx < 0){
        printf("Argumenty nie moga byc ujemne\n");
        return -1;
    }
    int a = 0;
    int b = 1;
    if (dx > b - a){
        printf("Niepoprawna dokladnosc obliczen\n");
        return -1;
    }
    float x = a+dx/2;
    while(x < b){
        for (int i = 0; i < n && x < b; i++){
            pid_t child = fork();
            if(child == 0){
                char argi[256], argx[256];
                sprintf(argi, "%d", i+1);
                sprintf(argx, "%f", x);
                execlp("./child", argi, argx, NULL);
            }
            x += dx;
        }
        while(wait(NULL) > 0){}
    }

    char f_name[256];
    char *buff;
    size_t len = 0;
    FILE *f;
    float res = 0;
    float fx;
    for (int i = 0; i < n; i++){
        sprintf(f_name, "w%d.txt", i+1);
        f = fopen(f_name, "r");
        while (getline(&buff, &len, f) != -1){
            fx = atof(buff);
            res += fx*dx;
        }
        fclose(f);
        remove(f_name);
    }
    free(buff);
    time (&e);
    dif = difftime(e, s);
    printf("result: %f\n", res);

    char buffer[100];
    int m = sprintf(buffer, "Czas dla dokladnodsci %f i %d procesow : %.2lfs\n", dx, n, dif);
    printf("%s", buffer);
    FILE *t = fopen("pomiar_zad_2.txt", "a");
    fwrite(buffer, 1, m, t);
    fclose(t);
    return 0;
}
