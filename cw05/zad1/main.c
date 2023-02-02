#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#define MAX_CMDS 5
#define MAX_ARGS 5
#define MAX_ELEMENTS 20
#define MAX_LINE_SIZE 1000
#define id 8

void getElement(char* command, char* element, char** elementsDefinitions, int numEl){
    for (int i = 0; i < numEl; i++){
        if(elementsDefinitions[i][id] == element[id]){
            memcpy(command, elementsDefinitions[i] + id + 4, strlen(elementsDefinitions[i] + id + 4) - 1);
            return;
        }

    }
}

int countPipeSigns(char* line){
    int res = 0;
    for (int i = 0; i < strlen(line); i++){
        if(line[i] == '|')
            res++;
    }
    return res;
}

void splitCommand(char *line, char** args, char* splitby){
    int i = 0;
    line = strtok(line, splitby);
    while (line){
        args[i++] = line;
        line = strtok(NULL, splitby);
    }
}

void executeLine(char* line){
    int no_elements = countPipeSigns(line) + 1;
    char** elements = calloc(no_elements, sizeof(char*));
    char splitby[] = {'|'};
    splitCommand(line, elements, splitby);
    int pipes[2], prev_pipes[2];
    for(int i = 0; i < no_elements; i++){
        char** args = calloc(no_elements, sizeof(char*));
        char splitby2[] = {' ', '\n'};
        splitCommand(elements[i], args, splitby2);

        pipe(pipes);
        pid_t pid = fork();
        if (pid == 0){
            close(prev_pipes[1]);
            close(pipes[0]);
            if (i!=0){
                dup2(prev_pipes[0], STDIN_FILENO);
            }
            if(i!=no_elements-1){
                dup2(pipes[1], STDOUT_FILENO);
            }

            execvp(args[0], args);
        }
        close(pipes[1]);
        close(prev_pipes[0]);
        prev_pipes[0] = pipes[0];
        prev_pipes[1] = pipes[1];
        free(args);
    }
    while(wait(NULL) > 0){}
    free(elements);
}

int main(int argc, char ** argv){
    if (argc != 2){
        printf("Niepoprawna liczba argumentow\n");
        return 1;
    }
    char* file_name = argv[1];
    FILE* fp = fopen(file_name, "r");
    if (!fp){
        printf("Nie mozna otworzyc pliku\n");
        return 1;
    }

    char* line = NULL;
    size_t len = 0;
    pid_t ppid = getpid();
    char** elementsDefinitions = calloc(MAX_ELEMENTS, sizeof(char*));
    int line_id = 0;
    while(getline(&line, &len, fp) != -1 && strcmp(line, "\n") != 0){
        elementsDefinitions[line_id] = calloc(len + 1, sizeof(char));
        strcpy(elementsDefinitions[line_id++], line);
    }
    while(getline(&line, &len, fp) != -1){
        if (ppid != getpid()){
            break;
        }
        printf("\n%s", line);
        int no_elements = countPipeSigns(line) + 1;
        char** elements = calloc(no_elements, sizeof(char*));
        char splitby[] = {'|', ' ', '\n'};
        splitCommand(line, elements, splitby);
        char* fullCommand = calloc(MAX_LINE_SIZE, sizeof(char*));;
        for(int i = 0; i < no_elements; i++){
            char* command = calloc(MAX_LINE_SIZE / MAX_CMDS, sizeof(char));
            getElement(command, elements[i], elementsDefinitions, line_id);
            strcat(fullCommand, command);
            if (i!=no_elements-1)
            strcat(fullCommand, " | ");
            free(command);
        }
        printf("%s\n", fullCommand);
        pid_t pid = fork();
        if (pid == 0){
            executeLine(fullCommand);
        }
        while(wait(NULL) > 0){}
        free(elements);
        free(fullCommand);
    }
    for(int i = 0; i < line_id; i++){
        free(elementsDefinitions[i]);
    }
    free(elementsDefinitions);
    free(line);
    fclose(fp);
    return 0;
}
