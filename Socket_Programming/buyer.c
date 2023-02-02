#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>
#define BUFF_SIZE 255
#define TIME_OUT 60
int signalAlarm = 0;

int connectServer(int port) {
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // checking for errors
        printf("Error in connecting to server\n");
    }

    return fd;
}

void alarmHandler(){
    signalAlarm = 1;
}

int main(int argc, char const *argv[]) {
    int recvFd;
    int new_socket, max_sd, sock, broadcast = 1, opt = 1;
    char buffer[1024] = {0};
    fd_set master_set, working_set;
    struct sockaddr_in bc_address;

    if (argc < 2) 
    {
        printf("ERROR, no port provided\n");
        exit(1);
    }
    
    int port;
    port = atoi(argv[1]);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("172.29.31.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));

    signal(SIGALRM, alarmHandler);
    siginterrupt(SIGALRM, 1);

    FD_ZERO(&master_set);
    max_sd = sock;
    FD_SET(sock, &master_set);
    FD_SET(0, &master_set);

    char name[BUFF_SIZE];
    write(1, "Enter your name 'Buyer': ", 25);
    read(0, name, BUFF_SIZE);
    name[strlen(name) - 1] = '\0';
    bzero(buffer, 1024);
    int isNegotiating = 0;
    int canOffer = 0;
    int advFd = 0;
    sigaction(SIGALRM, &(struct sigaction){.sa_handler = alarmHandler, .sa_flags = SA_RESTART}, NULL);

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);
        char advPortNum[BUFF_SIZE];
        char advOfferedPrice[BUFF_SIZE];
        
       if(signalAlarm){
           printf("Negotiation time passed\n");
           alarm(0);
           char message[1024];
           memset(message, 0, 1024);
           sprintf(message, "Negotiation time for answering %s passed\n", name);
           send(advFd, message, strlen(message), 0);
           isNegotiating = 0;
           canOffer = 1;
           close(advFd);
           FD_CLR(advFd, &master_set);
           signalAlarm = 0;
       }
       
       if(FD_ISSET(0, &working_set)){
           memset(buffer, 0, 1024);
           read(0, buffer, 1024);
            if(isNegotiating){
                printf("You are Negotiating on another advertisement!\n");
            }
            else if(buffer[0] == 'O'){
                alarm(TIME_OUT);
                signalAlarm = 0;
                isNegotiating = 1;
                int advPort;
                int idx = 0;
                int j;
                for(j = 2; buffer[j] != ' '; j++){
                    if(buffer[j] >= '0' && buffer[j] <= '9'){
                        advPortNum[idx] = buffer[j];
                        idx++;
                    }
                }
                idx = 0;
                for(j++; buffer[j] != '\n'; j++){
                    if(buffer[j] >= '0' && buffer[j] <= '9'){
                        advOfferedPrice[idx] = buffer[j];
                        idx++;
                    }
                }
                advFd = connectServer(atoi(advPortNum));
                FD_SET(advFd, &master_set);
                if(advFd > max_sd)
                    max_sd = advFd;
                char sendMesaage[BUFF_SIZE];
                memset(sendMesaage, 0, BUFF_SIZE);
                strcat(sendMesaage, name);
                strcat(sendMesaage, " : ");
                strcat(sendMesaage, advOfferedPrice);
                send(advFd, sendMesaage, strlen(sendMesaage), 0);
            }

        }
        if(FD_ISSET(advFd, &working_set)){
            memset(buffer, 0, 1024);
            recv(advFd, buffer, 1024, 0);
            printf("%s\n", buffer);
            char temp[BUFF_SIZE];
            strcpy(temp, "Seller recived your suggestion.\n");
            temp[strlen(temp) - 1] = '\0';
            buffer[strlen(buffer) - 1] = '\0';
            if(strcmp(temp, buffer) == 0){
                isNegotiating = 1;
            }
            else{
                isNegotiating = 0;
            }
            int isAccOrRej = 0;
            memset(temp, 0, BUFF_SIZE);
            strcpy(temp, "Your Suggested Price accepted.\n");
            temp[strlen(temp) - 1] = '\0';
            if(strcmp(temp, buffer) == 0){
                alarm(0);
                isAccOrRej = 1;
                signalAlarm = 0;
                FD_CLR(advFd, &master_set);
            }
            memset(temp, 0, BUFF_SIZE);
            strcpy(temp, "Your Suggested Price rejected.\n");
            temp[strlen(temp) - 1] = '\0';
            if(strcmp(temp, buffer) == 0){
                alarm(0);
                isAccOrRej = 1;
                signalAlarm = 0;
                FD_CLR(advFd, &master_set);
            }
            if(!isAccOrRej){
                alarm(TIME_OUT);
            }
            memset(temp, 0, BUFF_SIZE);
            strcpy(temp, "Another buyer is negotiating on this advertisement\n");
            temp[strlen(temp) - 1] = '\0';
            if(strcmp(temp, buffer) == 0){
                alarm(0);
            }
        }

        if(FD_ISSET(sock, &working_set)){
            memset(buffer, 0, 1024);
            recv(sock, buffer, 1024, 0);
            char portAdv[BUFF_SIZE];
            char advName[BUFF_SIZE];
            int idx = 0, idx2 = 0;
            if(buffer[0] == 'A'){
                for(int j = 2; buffer[j] != '\n'; j++){
                    if(buffer[j] >= '0' && buffer[j] <= '9'){
                        portAdv[idx] = buffer[j];
                        idx++;
                    }
                    else if(buffer[j] != '\0'){
                        advName[idx2] = buffer[j];
                        idx2++;
                    }
                }
                advName[strlen(advName) - 1] = '\0';
                char tempBuff[1024];
                memset(tempBuff, 0, 1024);
                strcat(tempBuff, advName);
                strcat(tempBuff, " ");
                strcat(tempBuff, portAdv);
                printf("%s\n", tempBuff);
            }
            else if(isNegotiating == 0){
                printf("%s\n", buffer);
            }
        }
     }
    return 0;
}