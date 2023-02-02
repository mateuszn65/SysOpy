#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include "utils.h"

int elves_waiting = 0;
int reindeers_waiting = 0;
pthread_t santa_thread;
pthread_t reindeers[REINDEERS_NO];
pthread_t elves[ELVES_NO];
pthread_t elves_at_workshop[ELVES_REQ];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elves_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeers_cond = PTHREAD_COND_INITIALIZER;

void* santa(void* arg);
void* elf(void* arg);
void* reindeer(void* arg);
void init();
void end();

int main(int argc, char **argv){

    init();
    pthread_join(santa_thread, NULL);
    end();

    return 0;
}

void init(){
    if (pthread_create(&santa_thread, NULL, santa, (void *) NULL) != 0)
        error("Couldn't create thread");
    for(int i = 0; i < ELVES_NO; i++){
        if (pthread_create(&elves[i], NULL, elf, (void *) NULL) != 0)
            error("Couldn't create thread");
    }
    for(int i = 0; i < REINDEERS_NO; i++){
        if (pthread_create(&reindeers[i], NULL, reindeer, (void *) NULL) != 0)
            error("Couldn't create thread");
    }
}

void end(){
    for(int i = 0; i < ELVES_NO; i++){
        pthread_cancel(elves[i]);
    }
    for(int i = 0; i < REINDEERS_NO; i++){
        pthread_cancel(reindeers[i]);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&santa_cond);
    pthread_cond_destroy(&elves_cond);
    pthread_cond_destroy(&reindeers_cond);
}


void* santa(void* arg){
    int deliveries = 0;
    while (deliveries < MAX_DELIVERIES){
        pthread_mutex_lock(&mutex);

        while(reindeers_waiting < REINDEERS_REQ && elves_waiting < ELVES_REQ){
            printf("Mikołaj: zasypiam\n");
            pthread_cond_wait(&santa_cond, &mutex);
            printf("Mikołaj: budzę się\n");
        }

        if(reindeers_waiting == REINDEERS_REQ){
            pthread_mutex_unlock(&mutex);
            printf("Mikołaj: dostarczam zabawki\n");
            sleep(randint(2, 4));
            pthread_mutex_lock(&mutex);
            deliveries++;
            reindeers_waiting = 0;
            pthread_cond_broadcast(&reindeers_cond);

        }

        if(deliveries < MAX_DELIVERIES && elves_waiting == ELVES_REQ){
            pthread_mutex_unlock(&mutex);
            printf("Mikołaj: rozwiązuje problemy elfów");
            for(int i = 0; i < ELVES_REQ; i++){
                printf(" %lu", elves_at_workshop[i]);
                elves_at_workshop[i] = 0;
            }
            printf("\n");
            sleep(randint(1, 2));
            pthread_mutex_lock(&mutex);
            elves_waiting = 0;
            pthread_cond_broadcast(&elves_cond);

        }
        pthread_mutex_unlock(&mutex);
    }
    printf("Mikołaj: wszystkie dostawy wykonane\n");
    pthread_exit((void *) NULL);
}

void* elf(void* arg){
    pthread_t id = pthread_self();

    while(1){
        sleep(randint(2, 5));
        pthread_mutex_lock(&mutex);
        while(elves_waiting == ELVES_REQ){
            printf("Elf: czeka na powrót elfów, %lu\n", id);
            pthread_cond_wait(&elves_cond, &mutex);
        }

        if (elves_waiting < ELVES_REQ){
            int i = elves_waiting;
            elves_at_workshop[elves_waiting++] = id;
            printf("Elf: czeka %d elfów na Mikołaja, %lu\n", elves_waiting, id);
            if(elves_waiting == ELVES_REQ){
                printf("Elf: wybudzam Mikołaja, %lu\n", id);
                pthread_cond_signal(&santa_cond);

            }
            while(pthread_equal(elves_at_workshop[i], id)){
                pthread_cond_wait(&elves_cond, &mutex);
            }

        }
        printf("Elf: Mikołaj rozwiązał problem, %lu\n", id);
        pthread_mutex_unlock(&mutex);
    }
}


void* reindeer(void* arg){
    pthread_t id = pthread_self();

    while(1){
        sleep(randint(5, 10));
        pthread_mutex_lock(&mutex);
        reindeers_waiting++;
        printf("Renifer: czeka %d reniferów na Mikołaja, %lu\n", reindeers_waiting, id);
        if(reindeers_waiting == REINDEERS_REQ){
            printf("Renifer: wybudzam Mikołaja, %lu\n", id);
            pthread_cond_signal(&santa_cond);
        }
        pthread_cond_broadcast(&reindeers_cond);
        while(reindeers_waiting > 0){
            pthread_cond_wait(&reindeers_cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
}
