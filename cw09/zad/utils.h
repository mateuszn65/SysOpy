#ifndef UTILS_H
#define UTILS_H

#define ELVES_NO 10
#define REINDEERS_NO 9
#define ELVES_REQ 3
#define REINDEERS_REQ 9
#define MAX_DELIVERIES 3


int error(char* msg){
    printf("Error, %s\n", msg);
    exit(-1);
};

int randint(int from, int to){
    return rand()% (to - from) + to;
}


#endif
