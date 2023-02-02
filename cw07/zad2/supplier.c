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

sem_t *table_capacity;
sem_t *table_access;
sem_t *table_ready;

struct pizzeria *pizzeria;

void deliver(struct pizzeria *pizzeria){
    if (sem_wait(table_ready) == -1){
        error("sem_wait error");
    }
    if (sem_wait(table_access) == -1){
        error("sem_wait error");
    }
    if (sem_post(table_capacity) == -1){
        error("sem_post error");
    }

    int i = 0;
    while(pizzeria->table[i] == FREE){
        i = (i + 1) % TABLE_SIZE;
    }

    int n = pizzeria->table[i] - 1;
    pizzeria->table[i] = FREE;
    pizzeria->nop_table--;

    printf("(%d %lld)\tPobieram pizze: %d. Liczba pizz na stole: %d\n", getpid(), get_time(), n, pizzeria->nop_table);

    if (sem_post(table_access) == -1){
        error("sem_post error");
    }

    sleep(randint(4, 5));
    printf("(%d %lld)\tDostarczam pizze: %d\n", getpid(), get_time(), n);
    sleep(randint(4, 5));
}

void handle_exit(){
    if (sem_close(table_capacity) == -1){
        error("Couldn't close semaphore");
    }
    if (sem_close(table_access) == -1){
        error("Couldn't close semaphore");
    }
    if (sem_close(table_ready) == -1){
        error("Couldn't close semaphore");
    }

    if (munmap(pizzeria, sizeof(struct pizzeria)) == -1){
        error("Couldn't detach shared memory");
    }
}

int main(void){
    srand(getpid());
    if(atexit(handle_exit) == -1){
        error("Couldn't set the exit handler");
    }

    if ((table_capacity = sem_open(TABLE_CAPACITY, O_RDWR))== SEM_FAILED){
        error("Couldn't create semaphore");
    }
    if ((table_access = sem_open(TABLE_ACCESS, O_RDWR)) == SEM_FAILED){
        error("Couldn't create semaphore");
    }
    if ((table_ready = sem_open(TABLE_READY, O_RDWR)) == SEM_FAILED){
        error("Couldn't create semaphore");
    }
    int fd;
    if ((fd = shm_open(PATH, O_RDWR, 0)) == -1){
        error("Couldn't get shared memory");
    }


    if((pizzeria = (struct pizzeria *)mmap(NULL, sizeof (struct pizzeria), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == (void *)(-1)){
        error("Couldn't get the shared memory address");
    }



    while(1){
        deliver(pizzeria);
    }

    return 0;
}



