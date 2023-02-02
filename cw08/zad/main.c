#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include "utils.h"


struct Image image;
int threads_no;
FILE* time_file;

void load_image(char* filename);
void save_image(char* filename);
void negate(int option);
void* negate_numbers(void* arg);
void* negate_blocks(void* arg);
void write_input_data(int option);

int main(int argc, char **argv){

    if(argc != 5){
        error("Incorrect number of arguments");
    }

    threads_no = atoi(argv[1]);
    int option;
    char* in_filename = argv[3];
    char* out_filename = argv[4];
    if (strcmp(argv[2], "numbers") == 0)
        option = NUMBERS;
    else if (strcmp(argv[2], "block") == 0)
        option = BLOCK;
    else
        error("Incorrect option");

    time_file = fopen("times.txt", "a");
    if (time_file == NULL)
        error("Couldn't open times file");

    load_image(in_filename);
    write_input_data(option);
    negate(option);
    save_image(out_filename);
    if (fclose(time_file) == -1)
        error("Couldn't close times file");
    return 0;
}


void load_image(char* filename){
    FILE* file = fopen(filename, "r");
    if (file == NULL)
        error("Couldn't open input file");
    char buffer[MAX_LINE_LENGTH];
    if (fgets(buffer, MAX_LINE_LENGTH, file) == NULL)
        error("Couldn't read to buffer");
    if (fgets(buffer, MAX_LINE_LENGTH, file) == NULL)
        error("Couldn't read to buffer");

    while (buffer[0] == '#'){
        if (fgets(buffer, MAX_LINE_LENGTH, file) == NULL)
            error("Couldn't read to buffer");
    }

    sscanf(buffer, "%d  %d", &image.width, &image.height);
     if (fgets(buffer, MAX_LINE_LENGTH, file) == NULL)
        error("Couldn't read to buffer");
    sscanf(buffer, "%d", &image.M);

    image.values = calloc(image.width * image.height, sizeof(int));
    int i = 0;
    char* tok_buffer;
    while (fgets(buffer, MAX_LINE_LENGTH, file) != NULL){
        tok_buffer = strtok(buffer, " \t\n");
        while (tok_buffer != NULL){

            image.values[i++] = atoi(tok_buffer);
            tok_buffer = strtok(NULL, " \t\n");
        }
    }

    if (fclose(file) == -1)
        error("Couldn't close input file");

}


void save_image(char* filename){
    FILE* file = fopen(filename, "w");
    if (file == NULL)
        error("Couldn't open output file");

    fprintf(file, "P2\n%d  %d\n%d\n", image.width, image.height, image.M);

    for (int i = 0; i < image.width * image.height; i++){
        fprintf(file, "%d  ", image.values[i]);
        if ((i+1) % (MAX_LINE_LENGTH / 5) == 0)
            fprintf(file, "\n");
    }

    if (fclose(file) == -1)
        error("Couldn't close output file");
}


void negate(int option){
    struct timeval total_start, total_stop;
    gettimeofday(&total_start, NULL);

    pthread_t* threads = calloc(threads_no, sizeof(pthread_t));
    int* threads_indexes = calloc(threads_no, sizeof(int));
    for(int i = 0; i < threads_no; i++){
        threads_indexes[i] = i;
        if (option == NUMBERS){
            if (pthread_create(&threads[i], NULL, negate_numbers, (void *) &threads_indexes[i]) != 0)
                error("Couldn't create thread");
        }else{
            if (pthread_create(&threads[i], NULL, negate_blocks, (void *) &threads_indexes[i]) != 0)
                error("Couldn't create thread");
        }
    }
    for(int i = 0; i < threads_no; i++){
        long* thread_time;
        pthread_join(threads[i], (void*)&thread_time);
        printf("\tthread: %d, thread time: %ld µs\n", i, *thread_time);
        fprintf(time_file, "\tthread: %d, thread time: %ld µs\n", i, *thread_time);
    }
    gettimeofday(&total_stop, NULL);
    long total_time = get_time(total_start, total_stop);
    printf("Total time: %ld µs\n", total_time);
    fprintf(time_file, "Total time: %ld µs\n", total_time);
    free(threads);
    free(threads_indexes);

}


void* negate_numbers(void* arg){
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    int index = *((int *) arg);
    int left_bound = index * ceil((image.M + 1) / threads_no) / 2;
    int right_bound = (index + 1) * ceil((image.M + 1) / threads_no) / 2;
    for (int i = 0; i < image.width * image.height; i++){
        int val = image.values[i] > image.M / 2 ? image.M - image.values[i] : image.values[i];
        if (left_bound <= val && val < right_bound)
            image.values[i] = image.M - image.values[i];
    }

    gettimeofday(&stop, NULL);
    long thread_time = get_time(start, stop);
    pthread_exit((void*)&thread_time);
}


void* negate_blocks(void* arg){
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    int index = *((int *) arg);
    int left_bound = index * ceil(image.width / threads_no);
    int right_bound = (index + 1) * ceil(image.width / threads_no);

    for(int i = left_bound; i < right_bound && i < image.width; i++){
        for(int j = i; j < image.width * image.height; j += image.width)
            image.values[j] = image.M - image.values[j];
    }

    gettimeofday(&stop, NULL);
    long thread_time = get_time(start, stop);
    pthread_exit((void*)&thread_time);
}


void write_input_data(int option){
    printf("\n\n______________________________________\n");
    printf("Number of threads: %d\n", threads_no);
    if (option == NUMBERS)
        printf("Option: NUMBERS\n");
    else
        printf("Option: BLOCK\n");
    printf("Size: %dx%d\n", image.width, image.height);

    fprintf(time_file, "\n\n______________________________________\n");
    fprintf(time_file, "Number of threads: %d\n", threads_no);
    if (option == NUMBERS)
        fprintf(time_file, "Option: NUMBERS\n");
    else
        fprintf(time_file, "Option: BLOCK\n");
    fprintf(time_file, "Size: %dx%d\n", image.width, image.height);
}



