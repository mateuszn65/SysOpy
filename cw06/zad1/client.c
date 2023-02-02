#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdbool.h>

#include "common.h"


int server_qid = -1;
int queue_id = -1;
int id = -1;


void stop(){
    message msg;
    msg.type = STOP;
    msg.id = id;
    msg.qid = queue_id;

    if(msgsnd(server_qid, &msg, MSGMAX, 0) == -1){
        error("Couldn't send the message");
    }
    exit(0);
}


void init_client(){
    message msg;
    msg.type = INIT;
    msg.qid = queue_id;

    if(msgsnd(server_qid, &msg, MSGMAX, 0) == -1){
        error("Couldn't send the message");
    }
    if(msgrcv(queue_id, &msg, MSGMAX, PRIO, 0) == -1){
        error("Couldn't recive the message");
    }

    if(msg.type == STOP){
        error("Maximum number of clients exceeded");
    }
    id = msg.id;
    printf("QID: %d\n", queue_id);
    printf("ID: %d\n", id);
}


void list(){
    message msg;
    msg.type = LIST;
    msg.qid = queue_id;

    if(msgsnd(server_qid, &msg, MSGMAX, 0) == -1){
        error("Couldn't send the message");
    }
    if(msgrcv(queue_id, &msg, MSGMAX, LIST, 0) == -1){
        error("Couldn't recive the message");
    }
    printf("%s\n", msg.text);
}


void toall(char* news){
    message msg;
    msg.type = TOALL;
    msg.id = id;
    msg.qid = queue_id;
    strcpy(msg.text, news);

    if(msgsnd(server_qid, &msg, MSGMAX, 0) == -1){
        error("Couldn't send the message");
    }
}


void toone(char* news){
    message msg;
    msg.type = TOONE;
    msg.qid = queue_id;
    msg.id = id;
    strcpy(msg.text, news);

    if(msgsnd(server_qid, &msg, MSGMAX, 0) == -1){
        error("Couldn't send the message");
    }
}


void sigint_handler(int signum){
    printf("\nRecived SIGINT\n");
    stop();
}


void close_queue(){
    if (queue_id > -1){
        if(msgctl(queue_id, IPC_RMID, NULL) == -1){
            error("Couldn't delete the queue");
        }
        printf("Deleted clients queue\n");
    }

}


bool canWrite(){
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return (FD_ISSET(0, &fds));
}


int main(void){
    if(atexit(close_queue) == -1){
        error("Couldn't set the exit handler");
    }

    if(signal(SIGINT, sigint_handler) == SIG_ERR){
        error("Couldn't set the sigint handler");
    }

    char* home;
    if ((home = getenv("HOME")) == NULL){
        error("Couldn't find $HOME varialble");
    }

    key_t publicKey;
    key_t privateKey;
    if((publicKey = ftok(home, PROJECTID)) == -1){
        error("Couldn't create the key");
    }

    struct msqid_ds queue;
    if ((server_qid = msgget(publicKey, 0666)) == -1){
        error("Couldn't connect to the queue");
    }else{
        printf("Server connected\n");
    }

    if((privateKey = ftok(home, getpid())) == -1){
        error("Couldn't create the key");
    }

    if ((queue_id = msgget(privateKey, IPC_CREAT | IPC_EXCL | 0666)) == -1){
        error("Couldn't create the queue");
    }else{
        printf("Client started\n");
    }


    message feedback;

    init_client();

    char* buff;
    size_t len = 0;
    while(1){
        if(msgctl(queue_id, IPC_STAT, &queue) == -1){
            error("Couldn't get the state of the queue");
        }

        if (queue.msg_qnum > 0){
            if(msgrcv(queue_id, &feedback, MSGMAX, PRIO, 0) == -1){
                error("Couldn't recive the message");
            }
            if(feedback.type == STOP){
                stop();
            }
            printf("%s\n", feedback.text);
        }

        if (canWrite()){
            getline(&buff, &len, stdin);

            char delims[] = {' ', '\n'};
            char* news;
            char* request = strtok_r(buff, delims, &news);
            if(!request){
                printf("Try again\n");
                continue;
            }

            if(!strcmp(request, "LIST")){
                list();
            }else if (!strcmp(request, "STOP")){
                stop();
            }else if (!strcmp(request, "TOALL")){
                toall(news);
            }else if (!strcmp(request, "TOONE")){
                toone(news);
            }else{
                printf("Unknown request\n");
                continue;
            }
        }else{
            usleep(100);
        }

    }

    return 0;
}
