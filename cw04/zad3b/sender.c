#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
int replies_count = 0;
int SIG1 = 0;
int SIG2 = 0;
int last = 0;

void sig1_handler(int signum, siginfo_t *info, void* ucontext){
    replies_count++;
}
void sig1_qhandler(int signum, siginfo_t *info, void* ucontext){
    printf("Numer odebranego sygnalu %d\n", info->si_value.sival_int);
    replies_count++;
}
void sig2_handler(int signum, siginfo_t *info, void* ucontext){
    last = 1;
}
int main(int argc, char ** argv){
    if (argc != 4){
        printf("Nieprawidlowa liczba argumentow\n");
        exit(1);
    }
    pid_t catcher_pid = atoi(argv[1]);
    int signals_to_sent = atoi(argv[2]);
    char* modes[] = {"KILL", "SIGQUEUE", "SIGRT"};
    char* mode = argv[3];
    int mode_int = 0;
    if (strcmp(mode, modes[0])&&strcmp(mode, modes[1])&&strcmp(mode, modes[2])){
        printf("Nieprawidlowe argumenty\n");
        exit(1);
    }
    SIG1 = SIGUSR1;
    SIG2 = SIGUSR2;
    if (!strcmp(mode, modes[2])){
        SIG1 = SIGRTMIN;
        SIG2 = SIGRTMAX;
    }
    if (!strcmp(mode, modes[1])){
        mode_int = 1;
    }

    sigset_t block_mask;
    sigfillset(&block_mask);
    sigdelset(&block_mask, SIG1);
    sigdelset(&block_mask, SIG2);
    sigprocmask(SIG_SETMASK, &block_mask, NULL);

    struct sigaction act1;
    if (mode_int){
        act1.sa_sigaction= sig1_qhandler;
    }else{
        act1.sa_sigaction= sig1_handler;
    }
    act1.sa_flags = SA_SIGINFO;
    sigemptyset(&act1.sa_mask);
    sigaction(SIG1, &act1, NULL);

    struct sigaction act2;
    act2.sa_sigaction= sig2_handler;
    act1.sa_flags = SA_SIGINFO;
    sigemptyset(&act2.sa_mask);
    sigaction(SIG2, &act2, NULL);

    for (int i = 0; i < signals_to_sent; i++){
        if (mode_int){
            sigqueue(catcher_pid, SIG1, (union sigval){.sival_int = 0});
        }else{
            kill(catcher_pid, SIG1);
        }
        while (replies_count - 1 < i){
        }
    }
    if (mode_int){
        sigqueue(catcher_pid, SIG2, (union sigval){.sival_int = 0});
    }else{
        kill(catcher_pid, SIG2);
    }
    while (!last){
    }
    printf("Wyslano %d sygnalow, odebrano %d syganlow\n", signals_to_sent, replies_count);

    return 0;
}
