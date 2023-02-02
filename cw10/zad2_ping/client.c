#include "utils.h"
#define USAGE "expeted: <name> <local|net> <path to server socket | ip: port"

int socket_fd, binded_socket_fd;
char symbol;
int id = -1;
int opponent_id = -1;
char nick[MAX_NICK_LEN];

void connect_local(char *socket_path);
void connect_net(char *ip_port);
void draw(char board[BOARD_SIZE]);
void handle_move(struct Msg *msg);
void handle_sigint();
void handle_exit();


int main(int argc, char **argv){
    if (argc != 4){
        printf("%s\n", USAGE);
        error("Incorrect number of arguments");
    }

    if (strcmp(argv[2], "local") == 0){
        connect_local(argv[3]);
    }else if(strcmp(argv[2], "net") == 0){
        connect_net(argv[3]);
    }else{
        printf("%s\n", USAGE);
        error("Incorrect argument");
    }

    if(atexit(handle_exit) == -1)
        error("Couldn't set the exit handler");

    if(signal(SIGINT, handle_sigint) == SIG_ERR)
        error("Couldn't set the sigint handler");

    setnonblocking(STDIN_FILENO);

    struct Msg msg;
    msg.type = CONNECT;
    strcpy(msg.nick, argv[1]);
    strcpy(nick, argv[1]);
    sendto(socket_fd, &msg, sizeof(struct Msg), 0, (struct sockaddr *)NULL, sizeof(struct sockaddr));

    while(1){
        recvfrom(socket_fd, &msg, sizeof(struct Msg), 0, (struct sockaddr *)NULL, NULL);
        switch (msg.type){
            case DISCONNECT:
                printf("disconnect\n");
                exit(0);

            case START1:
                symbol = CIRCLE;
                id = msg.id;
                printf("Your game started\nYou have the first move\n");
                msg.type = MOVE;
                handle_move(&msg);
                break;

            case START2:
                symbol = CROSS;
                id = msg.id;
                draw(msg.board);
                printf("Your game started\nYour opponent has the first move\n");
                break;

            case NICK_TAKEN:
                error("Username already taken");

            case SERVER_FULL:
                error("Server is full");

            case OPPONENT_LEFT:
                printf("Opponent left\n");
                exit(0);

            case WAITING_FOR_OPPONENT:
                id = msg.id;
                printf("Waiting for opponent to join\n");
                break;

            case END:
                draw(msg.board);
                if (msg.winner == -1){
                    printf("YOU TIED!\n");
                }else if (id == msg.winner){
                    printf("YOU WON!\n");
                }else{
                    printf("YOU LOST!\n");
                }
                exit(0);

            case MOVE:
                handle_move(&msg);
                break;

            case INCORRECT_MOVE:
                printf("Invalid move\n");
                msg.type = MOVE;
                handle_move(&msg);
                break;

            case OPPONENT_MOVE:
                draw(msg.board);
                printf("Waiting for opponents move\n");
                break;
            case PING:
                msg.id = id;
                sendto(socket_fd, &msg, sizeof(struct Msg), 0, (struct sockaddr *)NULL, sizeof(struct sockaddr));

                break;

            default:
                error("Invalid msg type");
        }
    }

    return 0;
}


void connect_local(char *socket_path){
    if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
        error("Couldn't create socket");

    struct sockaddr_un address, binded_addr;
    address.sun_family = AF_UNIX;
    binded_addr.sun_family = AF_UNIX;
    strcpy(address.sun_path, socket_path);
    sprintf(binded_addr.sun_path, "socket_%d", getpid());

    if (bind(socket_fd, (struct sockaddr *) &binded_addr, sizeof(binded_addr)) == -1)
        error("Couldn't create socket");

    if (connect(socket_fd, (struct sockaddr*)&address, sizeof(address)) == -1)
        error("Couldn't connect");

}

void connect_net(char *ip_port){
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        error("Couldn't create socket");

    struct sockaddr_in address;
    address.sin_family = AF_INET;

    if (inet_pton(AF_INET, strtok(ip_port, ":"), &address.sin_addr) == -1)
        error("Invalid address");
    address.sin_port = htons(atoi(strtok(NULL, ":")));
    if (connect(socket_fd, (struct sockaddr*)&address, sizeof(address)) == -1)
        error("Couldn't connect");

}

void draw(char board[BOARD_SIZE]){
    printf("\n %c | %c | %c \n", board[0], board[1], board[2]);
    printf("___________\n");
    printf(" %c | %c | %c \n", board[3], board[4], board[5]);
    printf("___________\n");
    printf(" %c | %c | %c \n\n", board[6], board[7], board[8]);
}

void handle_move(struct Msg *msg){
    draw(msg->board);
    printf("You are: %c\nChoose position [1-9]: \n", symbol);
    int c;
    struct Msg tmp;
    setnonblocking(socket_fd);
    while(1){
        tmp.type = MOVE;
        recvfrom(socket_fd, &tmp, sizeof(struct Msg), 0, (struct sockaddr *)NULL, NULL);
        if(tmp.type == OPPONENT_LEFT){
            printf("Opponent left\n");
            exit(0);
        }else if(tmp.type == DISCONNECT){
            printf("disconnect\n");
            exit(0);
        }else if (tmp.type == PING){
            tmp.id = id;
            sendto(socket_fd, &tmp, sizeof(struct Msg), 0, (struct sockaddr *)NULL, sizeof(struct sockaddr));

        }
        c = getchar();
        if(49 <= c && c <= 57)
            break;
    }
    setblocking(socket_fd);
    msg->move_id = c - 48;
    msg->id = id;
    sendto(socket_fd, msg, sizeof(struct Msg), 0, (struct sockaddr *)NULL, sizeof(struct sockaddr));

}
void send_exit(){
    struct Msg msg;
    msg.type = DISCONNECT;
    msg.id = id;
    strcpy(msg.nick, nick);
    sendto(socket_fd, &msg, sizeof(struct Msg), 0, (struct sockaddr *)NULL, sizeof(struct sockaddr));
}

void handle_sigint(){
    exit(0);
}

void handle_exit(){
    //send_exit();
    close(socket_fd);
    char socket_binded[20];
    sprintf(socket_binded, "socket_%d", getpid());
    unlink(socket_binded);
}





