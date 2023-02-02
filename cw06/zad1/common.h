#ifndef COMMON_H
#define COMMON_H
#define MSG_TEXTMAX 1024
#define CLMAX 10
#define PROJECTID 42

#define STOP 1
#define LIST 2
#define INIT 3
#define TOALL 4
#define TOONE 5
#define PRIO -100

typedef struct message{
    long type;
    int id;
    int qid;
    char text[MSG_TEXTMAX];
} message;

int error(char* msg){
    printf("Error, %s\n", msg);
    exit(-1);
}
const size_t MSGMAX = sizeof(message) - sizeof(long);



#endif
