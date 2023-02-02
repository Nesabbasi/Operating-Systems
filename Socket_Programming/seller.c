#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#define BUFF_SIZE 255
#define ADNUMBERS 20

struct Advertise{
    char ad[BUFF_SIZE];
    int port;
    char status[BUFF_SIZE];
    int sock;
    int new_socket;
    int price;
};

struct Buyers{
    char name[BUFF_SIZE];
    char adName[BUFF_SIZE];
    int price;
};

void printStatus(struct Advertise adv[ADNUMBERS], struct sockaddr_in bc_address, int counterAd, int sock){
    char newBuff[1024];
    for(int i = 0; i < counterAd; i++){
        sprintf(newBuff, "status for advertisement %d '%s': %s\n", i+1, adv[i].ad, adv[i].status);
        sendto(sock, newBuff, strlen(newBuff), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
    }
}

void printAds2Status(struct Advertise adv[ADNUMBERS], struct sockaddr_in bc_address, int counterAd, int sock){
    char newBuff[1024];
    if(counterAd != 0 ) {
        memset(newBuff, 0, 1024);
        sprintf(newBuff, "Update advertisement status:\n");
        sendto(sock, newBuff, strlen(newBuff), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
    }
    for(int i = 0; i < counterAd; i++){
        memset(newBuff, 0, 1024);
        if(strcmp(adv[i].status, "Expired") != 0){
            sprintf(newBuff, "%s: %s\n", adv[i].ad, adv[i].status);
            sendto(sock, newBuff, strlen(newBuff), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
        }
    }
}

int checkCanAddAdvertise(struct Advertise adv[ADNUMBERS], int counterAd){
    for(int i = 0; i < counterAd; i++){
        if(strcmp(adv[i].status, "Negotiating") == 0){
            return 0;
        }
    }
    return 1;
}

void updateBuyers(struct Buyers *buyerr[ADNUMBERS], int *counterBuyer, char *advName){
    for(int i = 0; i < *counterBuyer; i++){
        if(strcmp((buyerr[i]->adName), advName) == 0){
            for(int j = i; j < *counterBuyer; j++){
                buyerr[j] = buyerr[j + 1];
            }
            counterBuyer--;
        }
    }
}

int setupServer(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    listen(server_fd, 4);
    return server_fd;
}

int acceptClient(int sock) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(sock, (struct sockaddr *)&client_address, (socklen_t*) &address_len);
    return client_fd;
}

char buyerName[BUFF_SIZE];
char suggestedPrice[BUFF_SIZE];

int main(int argc, char const *argv[]) {
    int sock, new_socket, max_sd, broadcast = 1, opt = 1;
    char buffer[1024] = {0};
    fd_set master_set, working_set;
    struct sockaddr_in bc_address;
    char bufferWrite[1025];
    
    if (argc < 2) {
        memset(bufferWrite, 0, 1024);
        sprintf(bufferWrite, "ERROR, no port provided\n");
        write(1, bufferWrite, strlen(bufferWrite));
        exit(1);
    }
    
    int port;
    port = atoi(argv[1]);
    
    struct Advertise adv[ADNUMBERS];
    struct Buyers buyerr[ADNUMBERS];

    int counterAd = 0;
    int counterBuyer = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("172.29.31.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));

    FD_ZERO(&master_set);
    max_sd = 0;
    FD_SET(0, &master_set);

    char name[BUFF_SIZE];
    write(1, "Enter your name 'Seller': ", 26);
    read(0, name, BUFF_SIZE);
    bzero(buffer, 1024);

    char fileName[300];
    memset(fileName, 0, 300);
    sprintf(fileName, "%s.txt", name);
    int fileFd = open(fileName, O_CREAT | O_RDWR | O_APPEND, 0644);

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);
        int idx = 0, idx2 = 0;
        char portNum[BUFF_SIZE];
        char advName[BUFF_SIZE];
        memset(portNum, 0, BUFF_SIZE);
        memset(advName, 0, BUFF_SIZE);
        if(FD_ISSET(0, &working_set)){
            memset(buffer, 0, 1024);
            read(0, buffer, 1024);
            int isUPressed = 0;
            if(buffer[0] == 'A'){
                if (checkCanAddAdvertise(adv, counterAd)){
                    for(int j = 2; buffer[j] != '\n'; j++){
                        if(buffer[j] >= '0' && buffer[j] <= '9'){
                            portNum[idx] = buffer[j];
                            idx++;
                        }
                        else if(buffer[j] != '\0'){
                            advName[idx2] = buffer[j];
                            idx2++;
                        }
                    }
                    memset(adv[counterAd].ad, 0, BUFF_SIZE);
                    advName[strlen(advName) - 1] = '\0';
                    strcpy(adv[counterAd].ad, advName);
                    adv[counterAd].port = atoi(portNum);
                    strcpy(adv[counterAd].status, "Waiting");
                    memset(bufferWrite, 0, 1024);
                    sprintf(bufferWrite, "status for advertisement %d: %s\n", counterAd + 1, adv[counterAd].status);
                    write(1, bufferWrite, strlen(bufferWrite));
                    memset(bufferWrite, 0, 1024);
                    sprintf(bufferWrite, "Advertisment number %d add with name: %s and port: %d\n",counterAd + 1, adv[counterAd].ad, adv[counterAd].port);
                    write(1, bufferWrite, strlen(bufferWrite));
                    int server_fd;
                    server_fd = setupServer(adv[counterAd].port);
                    adv[counterAd].sock = server_fd;
                    counterAd++;
                    FD_SET(server_fd, &master_set);
                    if (server_fd > max_sd)
                            max_sd = server_fd;
                    int a = sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
                }
                else{
                    memset(bufferWrite, 0, 1024);
                    sprintf(bufferWrite, "Can't add new Advertisement!\n");
                    write(1, bufferWrite, strlen(bufferWrite));
                }
            }
            else if(buffer[0] == 'U'){
                printAds2Status(adv, bc_address, counterAd, sock);
                isUPressed = 1;
            }
            else{
                char *token = strtok(buffer, " ");
                int offerMessage = 0;
                char accOrRej[BUFF_SIZE];
                char nameBuyer[BUFF_SIZE];
                while( token != NULL ) {
                    if(offerMessage == 0){
                        strcpy(accOrRej, token);
                        offerMessage = 1;
                    }
                    if(offerMessage == 1){
                        strcpy(nameBuyer, token);
                        offerMessage = 2;
                    }
                    if(offerMessage == 2){
                        strcpy(advName, token);
                    }
                    
                    token = strtok(NULL, " ");
                }
                advName[strlen(advName) - 1] = '\0';
                char newBuff[1024];
                int isInAdvertise = 0;
                for(int i = 0; i < counterAd; i++){
                    if(strcmp(advName, adv[i].ad) == 0){
                        isInAdvertise = 1;
                        if(strcmp("accept", accOrRej) == 0){
                            char writeTextFile[300];
                            memset(writeTextFile, 0, strlen(writeTextFile));
                            sprintf(writeTextFile, "%s : %d\n", adv[i].ad, adv[i].price);
                            write(fileFd, writeTextFile, strlen(writeTextFile));
                            memset(adv[i].status, 0, BUFF_SIZE);
                            strcpy(adv[i].status, "Expired");
                            memset(newBuff, 0, 1024);
                            sprintf(newBuff, "Your Suggested Price accepted.\n");
                            send(adv[i].new_socket, newBuff, strlen(newBuff),0);
                            memset(newBuff, 0, 1024);
                            printStatus(adv, bc_address, counterAd, sock);
                        }
                        else if(strcmp("reject", accOrRej) == 0){
                            memset(adv[i].status, 0, BUFF_SIZE);
                            strcpy(adv[i].status, "Waiting");
                            sprintf(newBuff, "Your Suggested Price rejected.\n");
                            send(adv[i].new_socket, newBuff, strlen(newBuff),0);
                            printStatus(adv, bc_address, counterAd, sock);
                        }
                        if(strcmp("accept", accOrRej) == 0 || strcmp("reject", accOrRej) == 0){
                            for(int k = 0; k < counterBuyer; k++){
                                if(strcmp(buyerr[k].adName, advName) == 0){
                                    for(int j = k; j < counterBuyer; j++){
                                        buyerr[j] = buyerr[j + 1];
                                    }
                                    counterBuyer--;
                                }
                            }
                            if(counterBuyer != 0) write(1, "Advertisement Update:\n", 23);
                            for(int k =0; k < counterBuyer; k++){
                                memset(bufferWrite, 0, 1024);
                                sprintf(bufferWrite, "%s : %s\n", buyerr[k].name, buyerr[k].adName);
                                write(1, bufferWrite, strlen(bufferWrite));
                            }
                        }
                    }
                }
                if(isInAdvertise == 0){
                    write(1, "Error! No such advertisement!\n", 31);
                }
            }
            if(!isUPressed)
                printAds2Status(adv, bc_address, counterAd, sock);

        }
        for (int i = 1; i <= max_sd; i++) {
            if(FD_ISSET(i, &working_set)){
                for(int j = 0; j < counterAd; j++){
                    if(i == adv[j].sock) {  // new clinet
                        char newBuff[1024];
                        new_socket = acceptClient(adv[j].sock);
                        FD_SET(new_socket, &master_set);
                        if (new_socket > max_sd)
                            max_sd = new_socket;
                        if(strcmp(adv[j].status, "Negotiating") == 0){
                            sprintf(newBuff, "Another buyer is negotiating on this advertisement\n");
                            send(new_socket, newBuff, strlen(newBuff),0);
                        }
                        else{
                            adv[j].new_socket = new_socket;
                            memset(buffer, 0, 1024);
                            recv(new_socket, buffer, 1024, 0);
                            char *token = strtok(buffer, " : ");
                            int isFirst = 1;
                            memset(buyerName, 0, BUFF_SIZE);
                            memset(suggestedPrice, 0, BUFF_SIZE);
                            while( token != NULL ) {
                                if(isFirst == 1){
                                    strcpy(buyerName, token);
                                    isFirst = 0;
                                }
                                if(isFirst == 0){
                                    strcpy(suggestedPrice, token);
                                }
                                
                                token = strtok(NULL, " : ");
                            }
                            memset(adv[j].status, 0, BUFF_SIZE);
                            strcpy(adv[j].status, "Negotiating");
                            memset(bufferWrite, 0, 1024);
                            sprintf(bufferWrite, "%s Suggested price for advertisement %d '%s': %s\n", buyerName, j+1, adv[j].ad, suggestedPrice);
                            write(1, bufferWrite, strlen(bufferWrite));
                            memset(buyerr[counterBuyer].name, 0, BUFF_SIZE);
                            strcpy(buyerr[counterBuyer].name, buyerName);
                            memset(buyerr[counterBuyer].adName, 0, BUFF_SIZE);
                            strcpy(buyerr[counterBuyer].adName, adv[j].ad);
                            buyerr[counterBuyer].price = atoi(suggestedPrice);
                            adv[j].price = atoi(suggestedPrice);
                            counterBuyer++;
                            sprintf(newBuff, "Seller recived your suggestion.\n");
                            send(new_socket, newBuff, strlen(newBuff),0);
                            for(int k = 0; k < counterBuyer; k++){
                                if(strcmp(buyerr[k].adName, advName) == 0){
                                    for(int j = k; j < counterBuyer; j++){
                                        buyerr[j] = buyerr[j + 1];
                                    }
                                    counterBuyer--;
                                }
                            }
                            if(counterBuyer != 0) write(1, "Advertisement Update:\n", 23);
                            for(int k =0; k < counterBuyer; k++){
                                memset(bufferWrite, 0, 1024);
                                sprintf(bufferWrite, "%s : %s\n", buyerr[k].name, buyerr[k].adName);
                                write(1, bufferWrite, strlen(bufferWrite));
                            }
                        }
                    }
                    else if(adv[j].new_socket == i) { // client sending msg
                        memset(buffer, 0, 1024);
                        int bytes_received;
                        bytes_received = recv(i , buffer, 1024, 0);        
                        if (bytes_received == 0) { // EOF
                            memset(bufferWrite, 0, 1024);
                            sprintf(bufferWrite, "client fd = %d closed\n", i);
                            write(1, bufferWrite, strlen(bufferWrite));
                            close(i);
                            FD_CLR(i, &master_set);
                        }
                        else{
                            memset(bufferWrite, 0, 1024);
                            sprintf(bufferWrite, "%s\n", buffer);
                            write(1, bufferWrite, strlen(bufferWrite));
                            char *token = strtok(buffer, " ");
                            int counterName =0 ;
                            memset(buyerName, 0, BUFF_SIZE);
                            while(token != NULL) {
                                if(counterName == 4){
                                    strcpy(buyerName, token);
                                    counterName++;
                                }
                                else{
                                    counterName++;
                                }
                                token = strtok(NULL, " ");
                            }
                            for(int k = 0; k < counterBuyer; k++){
                                if(strcmp(buyerr[k].name, buyerName) == 0){
                                    for(int j = k; j < counterBuyer; j++){
                                        buyerr[j] = buyerr[j + 1];
                                    }
                                    counterBuyer--;
                                }
                            }
                            if(counterBuyer != 0) write(1, "Advertisement Update:\n", 23);
                            for(int k =0; k < counterBuyer; k++){
                                memset(bufferWrite, 0, 1024);
                                sprintf(bufferWrite, "%s : %s\n", buyerr[k].name, buyerr[k].adName);
                                write(1, bufferWrite, strlen(bufferWrite));
                            }
                            memset(adv[j].status, '\0', strlen(adv[j].status));
                            strcat(adv[j].status, "Waiting");
                            printAds2Status(adv, bc_address, counterAd, sock);
                            memset(buffer, 0, 1024);
                            FD_CLR(adv[j].new_socket, &master_set);
                            adv[j].new_socket = -1;
                            
                        }
                    }
                }
            }
        }
    }

    return 0;
}