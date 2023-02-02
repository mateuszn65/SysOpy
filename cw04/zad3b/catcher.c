#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

int sender_pid = 0;
int sig_count = 0;
int SIG1 = 0;
int SIG2 = 0;
int mode_int = 0;
int last = 0;

void sig1_handler(int signum, siginfo_t *info, void* ucontext){
    sender_pid = info->si_pid;
    if(mode_int){
        sigqueue(sender_pid, SIG1, (union sigval){.sival_int = sig_count});
    }else{
        kill(sender_pid, SIG1);
    }
    sig_count++;
}
void sig2_handler(int signum, siginfo_t *info, void* ucontext){
    sender_pid = info->si_pid;
    if(mode_int){
        sigqueue(sender_pid, SIG2, (union sigval){.sival_int = 0});
    }else{
        kill(sender_pid, SIG2);
    }
    last = 1;
}


int main(int argc, char ** argv){
    if (argc != 2){
        printf("Nieprawidlowa liczba argumentow\n");
        exit(1);
    }
    char* modes[] = {"KILL", "SIGQUEUE", "SIGRT"};
    char* mode = argv[1];
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
    act1.sa_sigaction= sig1_handler;
    act1.sa_flags = SA_SIGINFO;
    sigemptyset(&act1.sa_mask);
    sigaction(SIG1, &act1, NULL);

    struct sigaction act2;
    act2.sa_sigaction= sig2_handler;
    act2.sa_flags = SA_SIGINFO;
    sigemptyset(&act2.sa_mask);
    sigaction(SIG2, &act2, NULL);


    printf("PID: %d\n", getpid());

    while (!last){
    }
    printf("Otrzymano %d signals\n", sig_count);

    return 0;
}
