#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <mqueue.h>

#include "common.h"


mqd_t queue_id = -1;
int clients[CLMAX] = {0};
int no_clients = 0;
FILE* file;
time_t rtime;

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
    int client_qid;
    if(client_id == -1){
        feedback.type = STOP;
    }else{
        char cpath[20];
        sprintf(cpath, "/%d", msg->pid);

        if ((client_qid = mq_open(cpath, O_WRONLY)) == -1){
            error("Couldn't open client's queue");
        }

        clients[client_id] = client_qid;
        no_clients++;
        feedback.type = INIT;
    }

    feedback.id = client_id;
    msg->id = client_id;

    if(mq_send(client_qid, (char*)&feedback, MSGMAX, feedback.type) == -1){
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



    if(mq_send(clients[msg->id], (char*)&feedback, MSGMAX, feedback.type) == -1){
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
    sprintf(new_msg.text, "From %d to all\nSent at: %s\n%s", msg->id, ctime(&rtime), text);

    for(int i = 0; i < CLMAX; i++){
        if(clients[i] == 0){
            continue;
        }
        if(mq_send(clients[i], (char*)&new_msg, MSGMAX, new_msg.type) == -1){
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
    sprintf(new_msg.text, "From %d to %d\nSent at: %s\n%s", msg->id, sendto, ctime(&rtime), news);

    if (0 <= sendto && sendto < CLMAX && clients[sendto] != 0){
        if(mq_send(clients[sendto], (char*)&new_msg, MSGMAX, new_msg.type) == -1){
            error("Couldn't send the message");
        }
    }
}


void handle_stop(message* msg){
    if (0 <= msg->id && msg->id  < CLMAX && clients[msg->id] != 0){
        if(mq_close(clients[msg->id]) == -1){
            error("Couldn't close client's queue");
        }
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
                if(mq_send(clients[i], (char*)&stop_msg, MSGMAX, stop_msg.type) == -1){
                    error("Couldn't send the message");
                }
            }
        }

        while (no_clients > 0){
            message stop_confirmed;
            if(mq_receive(queue_id, (char*)&stop_confirmed, MSGMAX, &stop_confirmed.type) < 0){
                error("Couldn't recive the message");
            }
            handle_stop(&stop_confirmed);
        }

        if(mq_close(queue_id) == -1){
            error("Couldn't close the queue");
        }
        if(mq_unlink(server_path) == -1){
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

    struct mq_attr attr;
    attr.mq_maxmsg = CLMAX;
    attr.mq_msgsize = MSGMAX;

    if ((queue_id = mq_open(server_path, O_RDONLY | O_CREAT | O_EXCL , 0666, &attr)) == -1){
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
        if(mq_receive(queue_id, (char*)&msg, MSGMAX, &msg.type) < 0){
            error("Couldn't recive the message");
        }
        time(&rtime);
        char* buffer;
        char buff[4096];
        switch(msg.type){
            case STOP:
                printf("Handling stop\n");
                buffer = "\nRequest type: STOP\n";
                sprintf(buff, "Recived request at: %sFrom client with ID: %d\nWith no message\n", ctime(&rtime), msg.id);
                handle_stop(&msg);
                break;
            case LIST:
                printf("Handling list\n");
                buffer= "\nRequest type: LIST\n";
                sprintf(buff, "Recived request at: %sFrom client with ID: %d\nWith no message\n", ctime(&rtime), msg.id);
                handle_list(&msg);
                break;
            case INIT:
                printf("Handling init\n");
                buffer = "\nRequest type: INIT\n";
                handle_init(&msg);
                sprintf(buff, "Recived request at: %sFrom client with ID: %d\nWith no message\n", ctime(&rtime), msg.id);
                break;
            case TOALL:
                printf("Handling toall\n");
                buffer = "\nRequest type: TOALL\n";
                sprintf(buff, "Recived request at: %sFrom client with ID: %d\nWith message: %s\n", ctime(&rtime), msg.id, msg.text);
                handle_toall(&msg);
                break;
            case TOONE:
                printf("Handling toone\n");
                buffer = "\nRequest type: TOONE\n";
                sprintf(buff, "Recived request at: %sFrom client with ID: %d\nWith message: %s\n", ctime(&rtime), msg.id, msg.text);
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



