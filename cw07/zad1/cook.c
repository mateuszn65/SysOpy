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


void make_pizza(struct pizzeria *pizzeria){
    int oven_id;
    int n = randint(0, 9);
    sleep(randint(1,2));
    printf("(%d %lld)\tPrzygotowuje pizze: %d\n", getpid(), get_time(), n);

    void place_in_oven(){
        struct sembuf sops[2];
        sops[0].sem_num = OVEN_CAPACITY;
        sops[0].sem_op = -1;
        sops[0].sem_flg = 0;
        sops[1].sem_num = OVEN_ACCESS;
        sops[1].sem_op = -1;
        sops[1].sem_flg = 0;
        if (semop(sem_id, sops, 2) == -1){
            error("Semop failed");
        }

        while(pizzeria->oven[pizzeria->oven_tail_id] != FREE){
           pizzeria->oven_tail_id = (pizzeria->oven_tail_id + 1) % OVEN_SIZE;
        }
        oven_id = pizzeria->oven_tail_id;
        pizzeria->oven[oven_id] = PIZZA;
        pizzeria->nop_oven++;
        printf("(%d %lld)\tDodalem pizze: %d. Liczba pizz w piecu: %d\n", getpid(), get_time(), n, pizzeria->nop_oven);

        sops[1].sem_op = 1;

        if (semop(sem_id, &sops[1], 1) == -1){
            error("Semop failed");
        }

        sleep(randint(4,5));
    }

    void place_on_table(){
        struct sembuf sops[2];
        sops[0].sem_num = OVEN_CAPACITY;
        sops[0].sem_op = 1;
        sops[0].sem_flg = 0;
        sops[1].sem_num = OVEN_ACCESS;
        sops[1].sem_op = -1;
        sops[1].sem_flg = 0;
        if (semop(sem_id, sops, 2) == -1){
            error("Semop failed");
        }

        pizzeria->oven[oven_id] = FREE;
        pizzeria->nop_oven--;

        sops[1].sem_op = 1;
        if (semop(sem_id, &sops[1], 1) == -1){
            error("Semop failed");
        }

        sops[0].sem_num = TABLE_CAPACITY;
        sops[0].sem_op = -1;
        sops[1].sem_num = TABLE_ACCES;
        sops[1].sem_op = -1;

        if (semop(sem_id, sops, 2) == -1){
            error("Semop failed");
        }

        while(pizzeria->table[pizzeria->table_tail_id] != FREE){
           pizzeria->table_tail_id = (pizzeria->table_tail_id + 1) % TABLE_SIZE;
        }

        pizzeria->table[pizzeria->table_tail_id] = n + 1;
        pizzeria->nop_table++;

        printf("(%d %lld)\tWyjmuje pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n", getpid(), get_time(), n, pizzeria->nop_oven, pizzeria->nop_table);

        sops[0].sem_num = TABLE_READY;
        sops[0].sem_op = 1;
        sops[1].sem_op = 1;
        if (semop(sem_id, sops, 2) == -1){
            error("Semop failed");
        }
    }


    place_in_oven();

    place_on_table();
}




int main(int argc, char **argv){
    srand(getpid());
    char* home;
    if ((home = getenv("HOME")) == NULL){
        error("Couldn't find $HOME varialble");
    }

    key_t key;
    if((key = ftok(home, PROJECTID)) == -1){
        error("Couldn't get the key");
    }

    if ((sem_id = semget(key, 0, 0)) == -1){
        error("Couldn't get the semaphore");
    }

    if ((shm_id = shmget(key, 0, 0)) == -1){
        error("Couldn't get the shared memory");
    }

    struct pizzeria *pizzeria;
    if((pizzeria = shmat(shm_id, NULL, 0)) == (void *)(-1)){
        error("Couldn't get the shared memory address");
    }

    while(1){
        make_pizza(pizzeria);
    }

    return 0;
}
