#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

void handler(int signum){
    printf("Przechwycono sygnal: %d\n", signum);
}

void is_signal_pending(){
    sigset_t pendingset;
    sigpending(&pendingset);
    if (sigismember(&pendingset, SIGUSR1)){
        printf("Oczekujacy sygnal jest widoczny\n");
        return;
    }
    printf("Oczekujacy sygnal nie jest widoczny\n");
}

int main(int argc, char ** argv){
    if (argc != 2){
        printf("Niepoprawna liczba argumentow\n");
        return 1;
    }
    char* modes[] = {"ignore", "handler", "mask", "pending"};
    char* mode = argv[1];

    if (!strcmp(mode, modes[0])){
        signal(SIGUSR1, SIG_IGN);
    }else if(!strcmp(mode, modes[1])){
        signal(SIGUSR1, handler);
    }else if(!strcmp(mode, modes[2]) || !strcmp(mode, modes[3])){
        sigset_t blockmask;
        sigemptyset(&blockmask);
        sigaddset(&blockmask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &blockmask, NULL);
    }else{
        printf("Niepoprawny argument\n");
        return 1;
    }

    printf("Proces macierzysty\n");
    raise(SIGUSR1);
    is_signal_pending();
    pid_t pid = fork();
    if (pid == -1){
        perror("Nie powolano procesu potomnego\n");
        exit(1);
    }
    if(pid == 0){
        printf("Proces potomka\n");
        if (!strcmp(mode, modes[3])){
            is_signal_pending();
        }else{
            raise(SIGUSR1);
        }
        if (strcmp(mode, modes[1])){
            if (execlp("./child", "./child", NULL) == -1){
                printf("Nie udalo sie wywowac podprogramu child\n");
                exit(1);
            }
        }
    }
    while(wait(NULL) > 0){}
    printf("\n");
    return 0;
}
