#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#include "utils.h"

int sem_id;
int shm_id;
int N = 0;
int M = 0;
pid_t *cooks;
pid_t *suppliers;




void sigint_handler(int signum){
    printf("Recived SIGINT\n");
    for(int i = 0; i < N; i++){
        kill(cooks[i], SIGINT);
    }
    for(int i = 0; i < M; i++){
        kill(suppliers[i], SIGINT);
    }

    exit(0);
}


void handle_exit(){
    if (semctl(sem_id, 0, IPC_RMID) == -1){
        error("Couldn't remove semaphore");
    }
    if(shmctl(shm_id, IPC_RMID, NULL) == -1){
        error("Couldn't remove shared memory");
    }
    free(cooks);
    free(suppliers);
}


int main(int argc, char **argv){
    if(atexit(handle_exit) == -1){
        error("Couldn't set the exit handler");
    }

    if(signal(SIGINT, sigint_handler) == SIG_ERR){
        error("Couldn't set the sigint handler");
    }

    if(argc != 3){
        error("Incorrect number of arguments");
    }

    N = atoi(argv[1]);
    M = atoi(argv[2]);

    char*home;
    if ((home = getenv("HOME")) == NULL){
        error("Couldn't find $HOME varialble");
    }

    key_t key;
    if((key = ftok(home, PROJECTID)) == -1){
        error("Couldn't create the key");
    }

    if ((sem_id = semget(key, NSEMS, IPC_CREAT | IPC_EXCL | 0666)) == -1){
        error("Couldn't create the semaphore");
    }

    if ((shm_id = shmget(key, sizeof(struct pizzeria), IPC_CREAT | IPC_EXCL | 0666)) == -1){
        error("Couldn't create the shared memory");
    }

    if (semctl(sem_id, OVEN_CAPACITY, SETVAL, OVEN_SIZE) == -1){
        error("Couldn't set semaphore");
    }
    if (semctl(sem_id, TABLE_CAPACITY, SETVAL, TABLE_SIZE) == -1){
        error("Couldn't set semaphore");
    }
    if (semctl(sem_id, OVEN_ACCESS, SETVAL, 1) == -1){
        error("Couldn't set semaphore");
    }
    if (semctl(sem_id, TABLE_ACCES, SETVAL, 1) == -1){
        error("Couldn't set semaphore");
    }
    if (semctl(sem_id, TABLE_READY, SETVAL, 0) == -1){
        error("Couldn't set semaphore");
    }

    cooks = calloc(N, sizeof(pid_t));
    for (int i = 0; i < N; i++){
        pid_t child = fork();

        if (child == 0){
            execlp("./cook", "./cook", NULL);

        }
        cooks[i] = child;
    }
    suppliers = calloc(N, sizeof(pid_t));
    for (int i = 0; i < N; i++){
        pid_t child = fork();

        if (child == 0){
            execlp("./supplier", "./supplier", NULL);
        }
        suppliers[i] = child;
    }
    while (wait(NULL) > 0){}
    return 0;
}



