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


sem_t *oven_capacity;
sem_t *table_capacity;
sem_t *oven_access;
sem_t *table_access;
sem_t *table_ready;

struct pizzeria *pizzeria;

void make_pizza(struct pizzeria *pizzeria){
    int oven_id;
    int n = randint(0, 9);
    sleep(randint(1,2));
    printf("(%d %lld)\tPrzygotowuje pizze: %d\n", getpid(), get_time(), n);

    void place_in_oven(){
        if (sem_wait(oven_capacity) == -1){
            error("sem_wait error");
        }
        if (sem_wait(oven_access) == -1){
            error("sem_wait error");
        }

        while(pizzeria->oven[pizzeria->oven_tail_id] != FREE){
           pizzeria->oven_tail_id = (pizzeria->oven_tail_id + 1) % OVEN_SIZE;
        }
        oven_id = pizzeria->oven_tail_id;
        pizzeria->oven[oven_id] = PIZZA;
        pizzeria->nop_oven++;
        printf("(%d %lld)\tDodalem pizze: %d. Liczba pizz w piecu: %d\n", getpid(), get_time(), n, pizzeria->nop_oven);

        if (sem_post(oven_access) == -1){
            error("sem_post error");
        }
        sleep(randint(4,5));

    }

    void place_on_table(){
        if (sem_wait(oven_access) == -1){
            error("sem_wait error");
        }
        if (sem_post(oven_capacity) == -1){
            error("sem_post error");
        }
        pizzeria->oven[oven_id] = FREE;
        pizzeria->nop_oven--;

        if (sem_post(oven_access) == -1){
            error("sem_post error");
        }


        if (sem_wait(table_capacity) == -1){
            error("sem_wait error");
        }
        if (sem_wait(table_access) == -1){
            error("sem_wait error");
        }

        while(pizzeria->table[pizzeria->table_tail_id] != FREE){
           pizzeria->table_tail_id = (pizzeria->table_tail_id + 1) % TABLE_SIZE;
        }

        pizzeria->table[pizzeria->table_tail_id] = n + 1;
        pizzeria->nop_table++;

        printf("(%d %lld)\tWyjmuje pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n", getpid(), get_time(), n, pizzeria->nop_oven, pizzeria->nop_table);

        if (sem_post(table_access) == -1){
            error("sem_post error");
        }
        if (sem_post(table_ready) == -1){
            error("sem_post error");
        }
    }


    place_in_oven();

    place_on_table();
}



void handle_exit(){
    if (sem_close(oven_capacity) == -1){
        error("Couldn't close semaphore");
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

    if (munmap(pizzeria, sizeof(struct pizzeria)) == -1){
        error("Couldn't detach shared memory");
    }
}

int main(int argc, char **argv){
    srand(getpid());

    if(atexit(handle_exit) == -1){
        error("Couldn't set the exit handler");
    }
    if ((oven_capacity = sem_open(OVEN_CAPACITY, O_RDWR)) == SEM_FAILED){
        error("Couldn't create semaphore");
    }
    if ((table_capacity = sem_open(TABLE_CAPACITY, O_RDWR))== SEM_FAILED){
        error("Couldn't create semaphore");
    }
    if ((oven_access = sem_open(OVEN_ACCESS, O_RDWR)) == SEM_FAILED){
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
        make_pizza(pizzeria);
    }

    return 0;
}
