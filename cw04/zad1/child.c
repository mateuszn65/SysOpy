#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>


void is_signal_pending(){
    sigset_t pendingset;
    sigpending(&pendingset);
    if (sigismember(&pendingset, SIGUSR1)){
        printf("Oczekujacy sygnal jest widoczny w programie wywolanym przez exec\n");
        return;
    }
    printf("Oczekujacy sygnal nie jest widoczny w programie wywolanym przez exec\n");
}

int main(int argc, char ** argv){
    is_signal_pending();
    raise(SIGUSR1);
    return 0;
}

