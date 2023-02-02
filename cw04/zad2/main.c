#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

void siginfo_handler(int signum, siginfo_t *info, void* ucontext){
    printf("\nSIGINFO handler\n");
    printf("Numer sygnalu: %d\n", info->si_signo);
    printf("Kod sygnalu: %d\n", info->si_code);
    printf("Sygnal wyslany przez proces o pid: %d\n", info->si_pid);
    printf("Id uzytkownika wysylajacego sygnal %d\n", info->si_uid);
    printf("Status sygnalu: %d\n", info->si_status);
}

void resethand_handler(int signum){
    printf("\nWywolam sie tylko raz, po czym przywroci domyslne dzialanie sygnalu\n");
}

void restart_handler(int signum){
    printf("\nRestart\n");
}
void restart_if_interapted(){
    printf("\nHello there! and Goodbye in 5s\n");
    sleep(5);
    printf("Goodbye\n");
}

int main(int argc, char ** argv){

    struct sigaction act;
    act.sa_sigaction = siginfo_handler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);

    if(sigaction(SIGUSR1, &act, NULL) == -1){
        perror("Nie udalo sie ustawic obslugi sygnalu\n");
        exit(1);
    }
    if(sigaction(SIGUSR2, &act, NULL) == -1){
        perror("Nie udalo sie ustawic obslugi sygnalu\n");
        exit(1);
    }
    if(sigaction(SIGCHLD, &act, NULL) == -1){
        perror("Nie udalo sie ustawic obslugi sygnalu\n");
        exit(1);
    }
    raise(SIGUSR1);
    raise(SIGUSR2);

    pid_t pid = fork();
    if (pid == 0){
        exit(5);
    }
    while(wait(NULL) > 0){}



    struct sigaction act2;
    act2.sa_handler = restart_handler;
    act2.sa_flags = SA_RESTART;
    sigemptyset(&act2.sa_mask);

    if(sigaction(SIGINT, &act2, NULL) == -1){
        perror("Nie udalo sie ustawic obslugi sygnalu\n");
        exit(1);
    }
    restart_if_interapted();

    struct sigaction act3;
    act3.sa_handler = resethand_handler;
    act3.sa_flags = SA_RESETHAND;
    sigemptyset(&act3.sa_mask);

    if(sigaction(SIGUSR2, &act3, NULL) == -1){
        perror("Nie udalo sie ustawic obslugi sygnalu\n");
        exit(1);
    }
    raise(SIGUSR2);
    raise(SIGUSR2);
    printf("Mnie tu nie ma\n");
    return 0;
}
