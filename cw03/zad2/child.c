#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
int main(int argc, char ** argv){
    int N = atoi(argv[0]);
    float x = atof(argv[1]);
    float w = 4/(x*x+1);
    char f_name[256];
    sprintf(f_name, "w%d.txt", N);
    FILE *f = fopen(f_name, "a");
    char buff[256];
    int n = sprintf(buff, "%f\n", w);
    fwrite(buff, 1, n, f);
    fclose(f);
    return 0;
}
