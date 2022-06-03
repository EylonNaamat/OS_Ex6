//
// Created by eylon on 6/2/22.
//
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <assert.h>


#define PORT_NUM 9027

int my_sock1;
int my_sock2;

void* sender(void*){
    char buf[4096] = "eylon";
    unsigned int len = 4096;

    int my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock == -1) {
        printf("socket creation failed...\n");
        exit(1);
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT_NUM);

    if (connect(my_sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(1);
    }

    sleep(2);
    if (send(my_sock, buf, 4096, 0) == -1) {
        printf("sending to the server failed...\n");
        exit(1);
    }

    sleep(2);
    strcpy(buf, "michael");
    if (send(my_sock, buf, 4096, 0) == -1) {
        printf("sending to the server failed...\n");
        exit(1);
    }

    sleep(2);
    strcpy(buf, "naamat");
    if (send(my_sock, buf, 4096, 0) == -1) {
        printf("sending to the server failed...\n");
        exit(1);
    }

    sleep(2);
    strcpy(buf, "matveev");
    if (send(my_sock, buf, 4096, 0) == -1) {
        printf("sending to the server failed...\n");
        exit(1);
    }
    sleep(2);
    close(my_sock);
}

void* receiver(void*){
    char buf[4096];
    int check = 0;
    int my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock == -1) {
        printf("socket creation failed...\n");
        exit(1);
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT_NUM);

    if (connect(my_sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(1);
    }
    while(1) {
        if (recv(my_sock, buf, 4096, 0) == -1) {
            printf("receiving from server failed...\n");
            exit(1);
        }
        printf("%s", buf);
        fflush(stdout);

        check = (!strcmp(buf, "eylon")) || (!strcmp(buf, "michael")) || (!strcmp(buf, "naamat")) ||
                    (!strcmp(buf, "matveev"));
        assert(check);
        if (!strcmp(buf, "matveev")) {
            break;
        }
    }
    close(my_sock);
}


int main(){
    pthread_t c1;
    pthread_t c2;

    pthread_create(&c1, NULL, sender, NULL);
    pthread_create(&c2, NULL, receiver, NULL);

    pthread_join(c1, NULL);
    pthread_join(c2, NULL);
}