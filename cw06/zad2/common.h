#ifndef COMMON_H
#define COMMON_H
#define MSG_TEXTMAX 1024
#define CLMAX 10
const char server_path[] = "/server";

#define STOP 5
#define LIST 4
#define INIT 3
#define TOALL 2
#define TOONE 1

typedef struct message{
    size_t type;
    int id;
    pid_t pid;
    char text[MSG_TEXTMAX];
} message;

int error(char* msg){
    printf("Error, %s\n", msg);
    exit(-1);
}
const size_t MSGMAX = sizeof(message);



#endif
