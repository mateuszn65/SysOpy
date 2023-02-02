#include "utils.h"



int local_fd;
int net_fd;
int epoll_fd;
struct epoll_event ev, events[MAX_CLIENTS + 2];
int clients_connected = 0;
struct Client *clients[MAX_CLIENTS] = {NULL};
int client_waiting_id = -1;
char *socket_path;
struct Game games[MAX_CLIENTS/2];


void init_local();
void init_net(char *port);
void init_epoll();
void wait_for_clients();
void find_and_dc_client(char username[MAX_NICK_LEN]);
int check_username(struct Msg* msg);
int create_client(int client_fd, struct Msg* msg, struct sockaddr address);
void handle_move(int client_fd, struct Msg* msg);
void handle_connect(int client_fd, struct Msg* msg, struct sockaddr* address);
void start_game(int player1_id, int player2_id);
int is_over(char board[BOARD_SIZE], char symbol);
void disconnect_player(int id);
void handle_sigint();
void handle_exit();


int main(int argc, char **argv){
    if (argc != 3)
        error("Incorrect number of arguments");

    if(atexit(handle_exit) == -1)
        error("Couldn't set the exit handler");


    if(signal(SIGINT, handle_sigint) == SIG_ERR)
        error("Couldn't set the sigint handler");
    char *port = argv[1];
    socket_path = argv[2];

    init_local();
    init_net(port);
    init_epoll();
    printf("server ready\n");
    wait_for_clients();

    return 0;
}


void init_local(){
    if ((local_fd = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
        error("Couldn't create socket");
    int opt = 1;
    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, socket_path);

    if(setsockopt(local_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        error("setsocket error");

    if(bind(local_fd, (struct sockaddr *) &address, sizeof(address)) == -1)
        error("Couldn't bind lolac socket");

}


void init_net(char *port){
    if ((net_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        error("Couldn't create socket");
    int opt = 1;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(atoi(port));
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(setsockopt(net_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        error("setsocket error");

    if(bind(net_fd, (struct sockaddr *) &address, sizeof(address)) == -1)
        error("Couldn't bind lolac socket");
}


void init_epoll(){
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        error("epoll create");


    ev.events  = EPOLLIN;
    ev.data.fd = local_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, local_fd, &ev) == -1)
        error("epoll_ctl");

    ev.data.fd = net_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, net_fd, &ev) == -1)
        error("epoll_ctl");
}


void find_and_dc_client(char username[MAX_NICK_LEN]){
    for(int j = 0; j < MAX_CLIENTS; j++){
        if(clients[j] != NULL && strcmp(clients[j]->nick, username) == 0){
            if(clients[j]->game != NULL ){
                int player1_id = clients[j]->game->player1_id;
                int player2_id = clients[j]->game->player2_id;
                free(clients[j]->game);
                clients[player1_id]->game = NULL;
                clients[player2_id]->game = NULL;
                if(player1_id == j){
                    sendto(clients[player2_id]->fd, &(struct Msg){.type = OPPONENT_LEFT}, sizeof(struct Msg), 0, &clients[player2_id]->address, sizeof(struct sockaddr));

                }else{
                    sendto(clients[player1_id]->fd, &(struct Msg){.type = OPPONENT_LEFT}, sizeof(struct Msg), 0, &clients[player1_id]->address, sizeof(struct sockaddr));
                }

            }
            disconnect_player(j);
            return;
        }
    }
}


void wait_for_clients(){
    //printf("local: %d, net: %d\n", local_fd, net_fd);
    struct Msg msg;
    struct sockaddr client_addr;
    size_t len = sizeof(client_addr);

    while(1){
        int nfds = epoll_wait(epoll_fd, events, 2 + MAX_CLIENTS, -1);
        if (nfds == -1)
            error("epoll_wait");

        for(int i = 0; i < nfds; i++){
            int client_fd = events[i].data.fd;
            recvfrom(events[i].data.fd, &msg, sizeof(struct Msg), 0, &client_addr, &len);
            //printf("event fd: %d\n", events[i].data.fd);

            if (msg.type == DISCONNECT){
                find_and_dc_client(msg.nick);

            }else if (msg.type == CONNECT){
                handle_connect(client_fd, &msg, &client_addr);

            }else if (msg.type == MOVE){
                handle_move(client_fd ,&msg);
            }


        }
    }
}


int check_username(struct Msg* msg){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i] != NULL && strcmp(clients[i]->nick, msg->nick) == 0)
            return -1;
    }
    return 0;
}

int create_client(int client_fd, struct Msg* msg, struct sockaddr address){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i] == NULL){
            struct Client *new_client = calloc(1, sizeof(struct Client));
            strcpy(new_client->nick, msg->nick);
            new_client->fd = client_fd;
            new_client->game = NULL;
            new_client->address = address;
            clients[i] = new_client;
            clients_connected++;
            return i;
        }
    }
    return -1;
}

void start_game(int player1_id, int player2_id){

    if(randint(0, 1)){
        int tmp = player1_id;
        player1_id = player2_id;
        player2_id = tmp;
    }

    char board[BOARD_SIZE] = {"         "};
    struct Game *game = malloc(sizeof(struct Game));
    strcpy(game->board, board);
    game->player1_id = player1_id;
    game->player2_id = player2_id;
    game->turn = 0;

    clients[player1_id]->game = game;
    clients[player2_id]->game = game;

    struct Msg msg;
    strcpy(msg.board, board);
    msg.type = START1;
    msg.id = player1_id;
    sendto(clients[player1_id]->fd, &msg, sizeof(struct Msg), 0, &clients[player1_id]->address, sizeof(struct sockaddr));
    msg.type = START2;
    msg.id = player2_id;
    sendto(clients[player2_id]->fd, &msg, sizeof(struct Msg), 0, &clients[player2_id]->address, sizeof(struct sockaddr));

}


void handle_connect(int client_fd, struct Msg* msg, struct sockaddr* address){
    if(clients_connected == MAX_CLIENTS){
        sendto(client_fd, &(struct Msg){.type = SERVER_FULL}, sizeof(struct Msg), 0, address, sizeof(struct sockaddr));
        printf("Connection failed, server full\n");
        return;
    }

    if (check_username(msg) == -1){
        sendto(client_fd, &(struct Msg){.type = NICK_TAKEN}, sizeof(struct Msg), 0, address, sizeof(struct sockaddr));
        printf("Connection failed, username already exists\n");
        return;
    }

    int id;
    id = create_client(client_fd, msg, *address);
    printf("Client successfuly connected\n");

    if(clients_connected % 2 == 0){
        printf("Players %d and %d started a game\n", client_waiting_id, id);
        start_game(client_waiting_id, id);
    }else{
        printf("Player %d is waiting for opponent\n", id);
        client_waiting_id = id;
        sendto(client_fd, &(struct Msg){.type = WAITING_FOR_OPPONENT}, sizeof(struct Msg), 0, address, sizeof(struct sockaddr));
    }

}

void handle_move(int client_fd, struct Msg* msg){

    int players[2] = {clients[msg->id]->game->player1_id, clients[msg->id]->game->player2_id};
    int turn = clients[msg->id]->game->turn;

    if (clients[msg->id]->game->board[msg->move_id - 1] != ' '){
        msg->type = INCORRECT_MOVE;
        sendto(clients[players[turn % 2]]->fd, msg, sizeof(struct Msg), 0, &clients[players[turn % 2]]->address, sizeof(struct sockaddr));
        strcpy(msg->board, clients[msg->id]->game->board);
        return ;
    }

    char symbol = turn % 2 ? CROSS : CIRCLE;
    clients[msg->id]->game->board[msg->move_id - 1] = symbol;
    strcpy(msg->board, clients[msg->id]->game->board);
    if (is_over(clients[msg->id]->game->board, symbol)){
        printf("Players %d and %d finished a game\n", players[0], players[1]);
        msg->type = END;
        msg->winner = players[turn % 2];
        free(clients[msg->id]->game);
        clients[players[0]]->game = NULL;
        clients[players[1]]->game = NULL;
        sendto(clients[players[turn % 2]]->fd, msg, sizeof(struct Msg), 0, &clients[players[turn % 2]]->address, sizeof(struct sockaddr));
        sendto(clients[players[(turn + 1) % 2]]->fd, msg, sizeof(struct Msg), 0, &clients[players[(turn + 1) % 2]]->address, sizeof(struct sockaddr));
        return ;
    }

    msg->type = OPPONENT_MOVE;
    sendto(clients[players[turn % 2]]->fd, msg, sizeof(struct Msg), 0, &clients[players[turn % 2]]->address, sizeof(struct sockaddr));
    clients[msg->id]->game->turn++;

    if (turn + 1 == 9){
        printf("Players %d and %d finished a game\n", players[0], players[1]);
        free(clients[msg->id]->game);
        clients[players[0]]->game = NULL;
        clients[players[1]]->game = NULL;
        msg->type = END;
        msg->winner = -1;

        sendto(clients[players[0]]->fd, msg, sizeof(struct Msg), 0, &clients[players[0]]->address, sizeof(struct sockaddr));
        sendto(clients[players[1]]->fd, msg, sizeof(struct Msg), 0, &clients[players[1]]->address, sizeof(struct sockaddr));

        return ;
    }

    msg->type = MOVE;
    sendto(clients[players[(turn + 1) % 2]]->fd, msg, sizeof(struct Msg), 0, &clients[players[(turn + 1) % 2]]->address, sizeof(struct sockaddr));

}


int is_over(char board[BOARD_SIZE], char symbol){
    for(int i = 0;i < 3; i++){
        if (board[3*i] == symbol && board[3*i] == board[3*i + 1] && board[3*i + 1] == board[3*i + 2])
            return 1;
        if (board[i] == symbol && board[i] == board[i + 3] && board[i + 3] == board[i + 6])
            return 1;
    }

    if (board[0] == symbol && board[0] == board[4] && board[4] == board[8])
        return 1;
    if (board[2] == symbol && board[2] == board[4] && board[4] == board[6])
        return 1;
    return 0;
}

void disconnect_player(int id){
    free(clients[id]);
    clients[id] = NULL;
    clients_connected--;
    printf("Disconnected cliend %d\n", id);

}

void handle_sigint(){
    exit(0);
}

void handle_exit(){

    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i] != NULL){
            sendto(clients[i]->fd, &(struct Msg){.type = DISCONNECT}, sizeof(struct Msg), 0, &clients[i]->address, sizeof(struct sockaddr));
            disconnect_player(i);
        }

    }

    close(local_fd);
    close(net_fd);
    close(epoll_fd);
    unlink(socket_path);
}



