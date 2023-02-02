#ifndef UTILS_H
#define UTILS_H

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
#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_CLIENTS 10
#define MAX_MSG_LEN 256
#define MAX_NICK_LEN 20
#define BOARD_SIZE 9
#define CROSS 'x'
#define CIRCLE 'o'

int error(char* msg){
    printf("Error, %s\n", msg);
    exit(-1);
};

int randint(int from, int to){
    return rand()% (to - from) + to;
}

void setnonblocking(int fd){
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

void setblocking(int fd){
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_NONBLOCK);
}

enum connection_t{
    LOCAL,
    NET
};

enum game_winner{
    NOT_FINISHED,
    X,
    O,
    TIE
};

struct Game{
    int player1_id;
    int player2_id;
    char board[BOARD_SIZE];
    int turn;
};

struct Client{
    char nick[MAX_NICK_LEN];
    int game_id;
    struct Game *game;
    int fd;
    struct sockaddr address;
};



enum msg_t{
    CONNECT,
    DISCONNECT,
    START1,
    START2,
    END,
    MOVE,
    OPPONENT_MOVE,
    OPPONENT_LEFT,
    INCORRECT_MOVE,
    NICK_TAKEN,
    SERVER_FULL,
    WAITING_FOR_OPPONENT
};

struct Msg{
    enum msg_t type;
    char nick[MAX_NICK_LEN];
    int id;
    char board[BOARD_SIZE];
    int move_id;
    int winner;
};

#endif
