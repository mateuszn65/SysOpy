#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#include "common.h"

struct msqid_ds queue;
int queue_id = -1;
int clients[CLMAX] = {0};
int no_clients = 0;
FILE* file;


int get_new_client_id(){
    for (int i = 0; i < CLMAX; i++){
        if (clients[i] == 0){
            return i;
        }
    }
    return -1;
}


void handle_init(message* msg){
    int client_id = get_new_client_id();
    message feedback;

    if(client_id == -1){
        feedback.type = STOP;
    }else{
        clients[client_id] = msg->qid;
        no_clients++;
        feedback.type = INIT;
    }

    feedback.id = client_id;
    msg->id = client_id;
    if(msgsnd(msg->qid, &feedback, MSGMAX, 0) == -1){
        error("Couldn't send the feedback");
    }
}


void handle_list(message* msg){
    message feedback;
    feedback.type = LIST;
    strcpy(feedback.text, "");
    sprintf(feedback.text + strlen(feedback.text), "List of clients\n");
    sprintf(feedback.text + strlen(feedback.text), "ID\tQID\n");

    for(int i = 0; i < CLMAX; i++){
        if(clients[i] == 0){
            continue;
        }
        sprintf(feedback.text + strlen(feedback.text), "%d\t%d\n", i, clients[i]);
    }

    if(msgsnd(msg->qid, &feedback, MSGMAX, 0) == -1){
        error("Couldn't send the feedback");
    }
}


void handle_toall(message* msg){
    message new_msg;
    new_msg.type = TOALL;
    char* rest;
    char* text;
    char delims[] = {'\0'};
    text = strtok_r(msg->text, delims, &rest);
    sprintf(new_msg.text, "From %d to all\nSent at: %s\n%s", msg->id, ctime(&queue.msg_rtime), text);

    for(int i = 0; i < CLMAX; i++){
        if(clients[i] == 0){
            continue;
        }
        if(msgsnd(clients[i], &new_msg, MSGMAX, 0) == -1){
            error("Couldn't send the message");
        }
    }
}


void handle_toone(message* msg){
    char delims[] = {' ', '\n'};
    char* news;
    int sendto = atoi(strtok_r(msg->text, delims, &news));
    message new_msg;
    new_msg.type = TOONE;
    sprintf(new_msg.text, "From %d to %d\nSent at: %s\n%s", msg->id, sendto, ctime(&queue.msg_rtime), news);

    if (0 <= sendto && sendto < CLMAX && clients[sendto] != 0){
        if(msgsnd(clients[sendto], &new_msg, MSGMAX, 0) == -1){
            error("Couldn't send the message");
        }
    }
}


void handle_stop(message* msg){
    if (0 <= msg->id && msg->id  < CLMAX && clients[msg->id] == msg->qid){
        clients[msg->id] = 0;
        no_clients--;
    }
}


void sigint_handler(int signum){
    printf("Recived SIGINT\n");
    exit(0);
}


void close_queue(){
    if (queue_id > -1){
        message stop_msg;
        stop_msg.type = STOP;
        for(int i = 0; i < CLMAX; i++){
            if (clients[i] != 0){
                if(msgsnd(clients[i], &stop_msg, MSGMAX, 0) == -1){
                    error("Couldn't send the message");
                }
            }
        }

        while (no_clients > 0){
            message stop_confirmed;
            if(msgrcv(queue_id, &stop_confirmed, MSGMAX, STOP, 0) == -1){
                error("Couldn't recive the message");
            }
            handle_stop(&stop_confirmed);
        }

        if(msgctl(queue_id, IPC_RMID, NULL) == -1){
            error("Couldn't delete the queue");
        }
        printf("Deleted servers queue\n");
    }
    fclose(file);
}


int main(void){
    if(atexit(close_queue) == -1){
        error("Couldn't set the exit handler");
    }

    if(signal(SIGINT, sigint_handler) == SIG_ERR){
        error("Couldn't set the sigint handler");
    }

    char*home;
    if ((home = getenv("HOME")) == NULL){
        error("Couldn't find $HOME varialble");
    }

    key_t key;
    if((key = ftok(home, PROJECTID)) == -1){
        error("Couldn't create the key");
    }

    if ((queue_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666)) == -1){
        error("Couldn't create the queue");
    }else{
        printf("Server started\n");
    }

    file = fopen("requests.txt", "w");
    if (!file){
        error("Couldn't create request.txt file");
    }

    message msg;
    while(1){
        if(msgrcv(queue_id, &msg, MSGMAX, PRIO, 0) == -1){
            error("Couldn't recive the message");
        }
        if(msgctl(queue_id, IPC_STAT, &queue) == -1){
            error("Couldn't get the state of the queue");
        }

        char* buffer;
        char buff[4096];
        switch(msg.type){
            case STOP:
                printf("Handling stop\n");
                buffer = "\nRequest type: STOP\n";
                sprintf(buff, "Recived request at: %sFrom client with ID: %d\nWith no message\n", ctime(&queue.msg_rtime), msg.id);
                handle_stop(&msg);
                break;
            case LIST:
                printf("Handling list\n");
                buffer= "\nRequest type: LIST\n";
                sprintf(buff, "Recived request at: %sFrom client with ID: %d\nWith no message\n", ctime(&queue.msg_rtime), msg.id);
                handle_list(&msg);
                break;
            case INIT:
                printf("Handling init\n");
                buffer = "\nRequest type: INIT\n";
                handle_init(&msg);
                sprintf(buff, "Recived request at: %sFrom client with ID: %d\nWith no message\n", ctime(&queue.msg_rtime), msg.id);
                break;
            case TOALL:
                printf("Handling toall\n");
                buffer = "\nRequest type: TOALL\n";
                sprintf(buff, "Recived request at: %sFrom client with ID: %d\nWith message: %s\n", ctime(&queue.msg_rtime), msg.id, msg.text);
                handle_toall(&msg);
                break;
            case TOONE:
                printf("Handling toone\n");
                buffer = "\nRequest type: TOONE\n";
                sprintf(buff, "Recived request at: %sFrom client with ID: %d\nWith message: %s\n", ctime(&queue.msg_rtime), msg.id, msg.text);
                handle_toone(&msg);
                break;
            default:
                printf("Unknown message type\n");
        }

        fwrite(buffer, 1, strlen(buffer), file);
        fwrite(buff, 1, strlen(buff), file);

    }


    return 0;
}



