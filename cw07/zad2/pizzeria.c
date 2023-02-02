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
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include "utils.h"

int fd;
int addr;
sem_t *oven_capacity;
sem_t *table_capacity;
sem_t *oven_access;
sem_t *table_access;
sem_t *table_ready;

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
    if (sem_close(oven_capacity) == -1){
        error("Couldn't close semaphore pizza");
    }
    if (sem_close(table_capacity) == -1){
        error("Couldn't close semaphore");
    }
    if (sem_close(oven_access) == -1){
        error("Couldn't close semaphore");
    }
    if (sem_close(table_access) == -1){
        error("Couldn't close semaphore");
    }
    if (sem_close(table_ready) == -1){
        error("Couldn't close semaphore");
    }

    if(sem_unlink(OVEN_CAPACITY) == -1){
        error("Couldn't unlink semaphore");
    }
    if(sem_unlink(TABLE_CAPACITY) == -1){
        error("Couldn't unlink semaphore");
    }
    if(sem_unlink(OVEN_ACCESS) == -1){
        error("Couldn't unlink semaphore");
    }
    if(sem_unlink(TABLE_ACCESS) == -1){
        error("Couldn't unlink semaphore");
    }
    if(sem_unlink(TABLE_READY) == -1){
        error("Couldn't unlink semaphore");
    }
    if(shm_unlink(PATH) == -1){
        error("Couldn't unlink shared memory");
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



    if ((oven_capacity = sem_open(OVEN_CAPACITY, O_CREAT | O_EXCL , 0666, OVEN_SIZE)) == SEM_FAILED){
        error("Couldn't create semaphore1");
    }
    if ((table_capacity = sem_open(TABLE_CAPACITY, O_CREAT | O_EXCL, 0666, TABLE_SIZE)) == SEM_FAILED){
        error("Couldn't create semaphore2");
    }
    if ((oven_access = sem_open(OVEN_ACCESS, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED){
        error("Couldn't create semaphore");
    }
    if ((table_access = sem_open(TABLE_ACCESS, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED){
        error("Couldn't create semaphore");
    }
    if ((table_ready = sem_open(TABLE_READY, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){
        error("Couldn't create semaphore");
    }

    if ((fd = shm_open(PATH, O_CREAT | O_EXCL | O_RDWR, 0666)) == -1){
        error("Couldn't create shared memory");
    }
    if ((addr = ftruncate(fd, sizeof(struct pizzeria))) == -1){
        error("Couldn't allocate shared memory");
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



