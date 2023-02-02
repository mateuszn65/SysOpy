#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void displayMails(int sender){
    FILE* mail_input;
    if (sender){
        mail_input = popen("mail | tail -n +2 | sort -k 3 -", "w");
    }
    else{
        mail_input = popen("mail | tail -n +2", "w");
    }

    fputs("exit", mail_input);
    pclose(mail_input);
}

void sendMail(char* email, char* title, char* message)
{
    FILE* mail_input;
    char command[256];
    sprintf(command, "mail -s \"%s\" %s", title, email);
    mail_input = popen(command, "w");

    fputs(message, mail_input);
    pclose(mail_input);
}

int main(int argc, char** argv){
    if (argc != 2 && argc != 4){
        printf("Niepoprawna liczba argumentow\n");
        return 1;
    }
    if (argc == 2){
        if (strcmp(argv[1], "nadawca") == 0){
            displayMails(1);
        }
        else if (strcmp(argv[1], "data") == 0){
            displayMails(0);
        }else{
            printf("Niepoprawna liczba argumentow\n");
            return 1;
        }
    }else{
        char* email = argv[1];
        char* title = argv[2];
        char* message = argv[3];
        sendMail(email, title, message);
    }

    return 0;
}
