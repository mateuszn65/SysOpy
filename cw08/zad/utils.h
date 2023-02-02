#ifndef UTILS_H
#define UTILS_H

#define MAX_LINE_LENGTH 70
struct Image{
    int width;
    int height;
    int M;
    int *values;
};


struct Context{
    int thread_id;
    int k;
};

enum options{
    NUMBERS,
    BLOCK
};


int error(char* msg){
    printf("Error, %s\n", msg);
    exit(-1);
};


long get_time(struct timeval start, struct timeval stop){
    return (stop.tv_sec - start.tv_sec)*1000000 + stop.tv_usec - start.tv_usec;
}


#endif
