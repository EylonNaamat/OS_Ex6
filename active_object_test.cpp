//
// Created by eylon on 6/3/22.
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

#define PORT_NUM 3503

void* client1(void*){
    int my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock == -1) {
        printf("socket creation failed...\n");
        exit(1);
    }
    printf("Socket successfully created..\n");

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT_NUM);

    if (connect(my_sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(1);
    }
    printf("connected to the server..\n");

    char buf[2048] = "eylon";

    if (send(my_sock, buf, 2048, 0) == -1) {
        printf("sending to the server failed...\n");
        exit(1);
    }
    if (recv(my_sock, buf, 2048, 0) == -1) {
        printf("receiving from server failed...\n");
        exit(1);
    }

    assert(!strcmp(buf, "FZMPO"));

    strcpy(buf, "michael");

    if (send(my_sock, buf, 2048, 0) == -1) {
        printf("sending to the server failed...\n");
        exit(1);
    }
    if (recv(my_sock, buf, 2048, 0) == -1) {
        printf("receiving from server failed...\n");
        exit(1);
    }

    assert(!strcmp(buf, "NJDIBFM"));

    close(my_sock);
}


void* client2(void*){
    int my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock == -1) {
        printf("socket creation failed...\n");
        exit(1);
    }
    printf("Socket successfully created..\n");

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT_NUM);

    if (connect(my_sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(1);
    }
    printf("connected to the server..\n");

    char buf[2048] = "naamat";

    if (send(my_sock, buf, 2048, 0) == -1) {
        printf("sending to the server failed...\n");
        exit(1);
    }
    if (recv(my_sock, buf, 2048, 0) == -1) {
        printf("receiving from server failed...\n");
        exit(1);
    }

    assert(!strcmp(buf, "OBBNBU"));

    strcpy(buf, "matveev");

    if (send(my_sock, buf, 2048, 0) == -1) {
        printf("sending to the server failed...\n");
        exit(1);
    }
    if (recv(my_sock, buf, 2048, 0) == -1) {
        printf("receiving from server failed...\n");
        exit(1);
    }

    assert(!strcmp(buf, "NBUWFFW"));

    strcpy(buf, "xYz");

    if (send(my_sock, buf, 2048, 0) == -1) {
        printf("sending to the server failed...\n");
        exit(1);
    }
    if (recv(my_sock, buf, 2048, 0) == -1) {
        printf("receiving from server failed...\n");
        exit(1);
    }

//    printf("%s\n", buf);
//    assert(!strcmp(buf, "YzA"));

    close(my_sock);
}

int main(){
    pthread_t t1;
    pthread_t t2;

    pthread_create(&t1, NULL, client1, NULL);
    pthread_create(&t2, NULL, client2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

}