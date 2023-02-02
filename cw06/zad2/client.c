#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <stdbool.h>

#include "common.h"


mqd_t server_qid = -1;
mqd_t queue_id = -1;
int id = -1;


void stop(){
    message msg;
    msg.type = STOP;
    msg.id = id;

    if(mq_send(server_qid, (char*)&msg, MSGMAX, 0) == -1){
        error("Coundn't send the message");
    }
    exit(0);
}


void init_client(){
    message msg;
    msg.type = INIT;
    msg.pid = getpid();

    if(mq_send(server_qid, (char*)&msg, MSGMAX, 0) == -1){
        error("Coundn't send the message");
    }
    if(mq_receive(queue_id, (char*)&msg, MSGMAX, NULL) == -1){
        error("Coundn't recive the message");
    }

    if(msg.type == STOP){
        error("Maximum number of clients exceeded");
    }
    id = msg.id;
    printf("ID: %d\n", id);
}


void list(){
    message msg;
    msg.type = LIST;
    msg.id = id;

    if(mq_send(server_qid, (char*)&msg, MSGMAX, 0) == -1){
        error("Coundn't send the message");
    }
    if(mq_receive(queue_id, (char*)&msg, MSGMAX, NULL) == -1){
        error("Coundn't recive the message");
    }
    printf("%s\n", msg.text);
}


void toall(char* news){
    message msg;
    msg.type = TOALL;
    msg.id = id;
    strcpy(msg.text, news);

    if(mq_send(server_qid, (char*)&msg, MSGMAX, 0) == -1){
        error("Coundn't send the message");
    }
}


void toone(char* news){
    message msg;
    msg.type = TOONE;
    msg.id = id;
    strcpy(msg.text, news);

    if(mq_send(server_qid, (char*)&msg, MSGMAX, 0) == -1){
        error("Coundn't send the message");
    }
}


void sigint_handler(int signum){
    printf("\nRecived SIGINT\n");
    stop();
}


void close_queue(){
    if (queue_id > -1){
        if(mq_close(queue_id) == -1){
            error("Couldn't close the queue");
        }
        char cpath[20];
        sprintf(cpath, "/%d", getpid());
        if(mq_unlink(cpath) == -1){
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
        error("Coundn't set the exit handler");
    }

    if(signal(SIGINT, sigint_handler) == SIG_ERR){
        error("Coundn't set the sigint handler");
    }

    struct mq_attr attr;
    struct mq_attr state;
    attr.mq_msgsize = MSGMAX;
    attr.mq_maxmsg = CLMAX;


    if ((server_qid = mq_open(server_path, O_RDWR , 0666, &attr)) == -1){
        error("Couldn't connect to servers queue");
    }

    char cpath[20];
    sprintf(cpath, "/%d", getpid());

    if ((queue_id = mq_open(cpath, O_RDONLY | O_CREAT | O_EXCL , 0666, &attr)) == -1){
        error("Couldn't create client's queue");
    }

    message feedback;

    init_client();

    char* buff;
    size_t len = 0;
    while(1){
        if(mq_getattr(queue_id, &state) == -1){
            error("Coundn't get the state of the queue");
        }

        if (state.mq_curmsgs > 0){
            if(mq_receive(queue_id, (char*)&feedback, MSGMAX, NULL) < 0){
                error("Coundn't recive the message");
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
