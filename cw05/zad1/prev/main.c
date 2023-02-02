#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#define MAX_CMDS 3
char ** splitCommand(char * line){
    int i = 0;
    char ** args = NULL;
    char splitby[2] = {' ', '\n'};
    line = strtok(line, splitby);
    while (line){
        i++;
        args = realloc(args, i * sizeof(char*));
        args[i - 1] = line;
        line = strtok(NULL, splitby);
    }
    args = realloc(args, (i + 1) * sizeof(char*));
    args[i] = NULL;

    return args;
}

void executeLine(char* line){
    int cmd_i = 0;
    char* cmds[MAX_CMDS];
    line = strtok(line, "|");
    while(line){
        cmds[cmd_i++] = line;
        line = strtok(NULL, "|");
    }

    int pipes[2], prev_pipes[2];
    for(int i = 0; i < cmd_i; i++){
        char** args = splitCommand(cmds[i]);
        pipe(pipes);
        pid_t pid = fork();
        if (pid == 0){
            close(prev_pipes[1]);
            close(pipes[0]);
            if (i!=0){
                dup2(prev_pipes[0], STDIN_FILENO);
            }
            if(i!=cmd_i-1){
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
    size_t len;
    pid_t ppid = getpid();
    int j = 0;
    while(getline(&line, &len, fp) != -1){
        if (ppid != getpid()){
            break;
        }
        pid_t pid = fork();
        if (pid == 0){

            executeLine(line);
        }else{j++;
            while(wait(NULL) > 0){}
        }

    }
    //printf("%d \n",j);
    fclose(fp);
    free(line);
    return 0;
}
