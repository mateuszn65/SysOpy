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

void deliver(struct pizzeria *pizzeria){
    struct sembuf sops[3];
    sops[0].sem_num = TABLE_CAPACITY;
    sops[0].sem_op = 1;
    sops[0].sem_flg = 0;
    sops[1].sem_num = TABLE_ACCES;
    sops[1].sem_op = -1;
    sops[1].sem_flg = 0;
    sops[2].sem_num = TABLE_READY;
    sops[2].sem_op = -1;
    sops[2].sem_flg = 0;
    if (semop(sem_id, sops, 3) == -1){
        error("Semop failed");
    }

    int i = 0;
    while(pizzeria->table[i] == FREE){
        i = (i + 1) % TABLE_SIZE;
    }

    int n = pizzeria->table[i] - 1;
    pizzeria->table[i] = FREE;
    pizzeria->nop_table--;

    printf("(%d %lld)\tPobieram pizze: %d. Liczba pizz na stole: %d\n", getpid(), get_time(), n, pizzeria->nop_table);
    sops[1].sem_op = 1;
    if (semop(sem_id, &sops[1], 1) == -1){
        error("Semop failed");
    }

    sleep(randint(4, 5));
    printf("(%d %lld)\tDostarczam pizze: %d\n", getpid(), get_time(), n);
    sleep(randint(4, 5));
}


int main(void){
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
        deliver(pizzeria);
    }

    return 0;
}



