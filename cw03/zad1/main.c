#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
int main(int argc, char ** argv){
    if (argc != 2){
        printf("Niepoprawna liczba argumentow");
        return -1;
    }
    int n = atoi(argv[1]);
    if (n < 0)
        return 0;
    printf("Pid procesu macierzystego: %d\n", (int)getpid());
    for (int i = 0; i < n; i++){
        pid_t child = fork();
        if(child == 0){
            printf("Napis pochodzi z %d procesu o pid: %d stworzonym przez proces o pid: %d\n", i+1, (int)getpid(), (int)getppid());
            return 0;
        }
        while(wait(NULL) > 0){}
    }

    return 0;
}
