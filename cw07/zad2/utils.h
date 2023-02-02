#ifndef UTILS_H
#define UTILS_H

#define TABLE_SIZE 5
#define OVEN_SIZE 5

#define PATH "/pizzeria"
#define OVEN_CAPACITY "/oven_capacity"
#define TABLE_CAPACITY "/table_capacity"
#define OVEN_ACCESS "/oven_access"
#define TABLE_ACCESS "/table_access"
#define TABLE_READY "/table_ready"

enum state{
    FREE,
    PIZZA
};


int error(char* msg){
    printf("Error, %s\n", msg);
    exit(-1);
};

int randint(int from, int to){
    return rand() % (to-from+1) + from;
};

long long get_time(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000LL + tv.tv_usec/1000;
}

struct pizzeria{
    int oven[OVEN_SIZE];
    int table[TABLE_SIZE];
    int oven_tail_id;
    int table_tail_id;
    int nop_oven;
    int nop_table;
};

#endif
