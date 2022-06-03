//
// Created by eylon on 5/31/22.
//
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>


#define PORT_NUM 9028
int my_sock;

void* receiver(void*){
    char buf[4096];
    while(1){
        if (recv(my_sock, buf, 4096, 0) == -1) {
            printf("receiving from server failed...\n");
            exit(1);
        }
        printf("%s", buf);
        fflush(stdout);
    }
}

void* sender(void*){
    char buf[4096];
    unsigned int len = 4096;
    while(1){
//        printf("insert command\n");
        fgets(buf, 4096, stdin);
        size_t ln = strlen(buf) - 1;
        if (buf[ln] == '\n') {
            buf[ln] = '\0';
        }
        if (send(my_sock, buf, 4096, 0) == -1) {
            printf("sending to the server failed...\n");
            exit(1);
        }
    }
}

int main(){
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
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

    pthread_t t1;
    pthread_t t2;

    pthread_create(&t1, NULL, sender, NULL);
    pthread_create(&t2, NULL, receiver, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    close(my_sock);

    return 0;
}