//
// Created by eylon on 5/30/22.
//
//#include <poll.h>
//#include <unistd.h>
//#include <string>
//#include <string.h>
//#include <pthread.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <netdb.h>
//#include <stdio.h>
#include "reactor.hpp"
#define PORT "9037"

//typedef int (*fd_funcs) (int fd, void* buf, size_t len, int flags);
//typedef void* (*fd_funcs) (void* argument);
//void* recv_send(void* argument);




//class Reactor{
//public:
//    struct pollfd* fds;
//    fd_funcs* funcs;
//    int count;
//    int size;
//    pthread_t thread_id;
//public:
//    Reactor(){
//        this->fds = (struct pollfd*)(malloc(sizeof(struct pollfd) * 10));
//        this->funcs = (fd_funcs *)(malloc(sizeof(fd_funcs) * 10));
//        this->count = 0;
//        this->size = 10;
//    }
//    ~Reactor(){}
//    void* newReactor();
//    bool InstallHandler(Reactor* reactor, void* (*func)(void*), int fd);
//    void RemoveHandler(Reactor* reactor, int i);
//};

bool Reactor::InstallHandler(Reactor *reactor, void* (*func)(void*), int fd) {
    if(reactor->count == reactor->size){
        reactor->size *= 2;
        reactor->fds = (struct pollfd*)(realloc(reactor->fds, (sizeof(struct pollfd) * reactor->size)));
        if(reactor->fds == NULL){
            return false;
        }
        reactor->funcs = (fd_funcs*)(realloc(reactor->funcs, (sizeof(fd_funcs) * reactor->size)));
        if(reactor->funcs == NULL){
            return false;
        }
    }
    reactor->fds[reactor->count].fd = fd;
    reactor->fds[reactor->count].events = POLLIN;
    reactor->funcs[reactor->count] = (fd_funcs)(func);
    reactor->count++;
    return true;
}

void Reactor::RemoveHandler(Reactor *reactor, int i) {
    reactor->fds[i] = reactor->fds[reactor->count-1];
    reactor->count--;
}


void *Reactor::newReactor() {
    while(1){
        int poll_stat = poll(this->fds, this->count, -1);
        if(poll_stat == -1){
            printf("error\n");
        }
        for(int i = 0; i < this->count; ++i){
            if(this->fds[i].revents & POLLIN){
                arguments arg;
                arg.react = this;
                arg.fd = this->fds[i];
                arg.i = i;
                pthread_create(&this->thread_id, NULL, this->funcs[i], &arg);
            }
        }
    }
}



//beej functions
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Return a listening socket
int get_listener_socket(void)
{
    int listener;     // Listening socket descriptor
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }

    // Listen
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}


//my funcs
void* recv_send(void* argument){
    arguments* arg = (arguments*)(argument);
    char buf[256];

    int nbytes = recv(arg->fd.fd, buf, sizeof buf, 0);

    int sender_fd = arg->fd.fd;

    if (nbytes <= 0) {
        // Got error or connection closed by client
        if (nbytes == 0) {
            // Connection closed
            printf("pollserver: socket %d hung up\n", sender_fd);
        } else {
            perror("recv");
        }

        close(arg->fd.fd); // Bye!

        arg->react->RemoveHandler(arg->react, arg->i);

    } else {
        // We got some good data from a client

        for(int j = 1; j < arg->react->count; j++) {
            // Send to everyone!
            int dest_fd = arg->react->fds[j].fd;

            // Except the listener and ourselves
            if (dest_fd != sender_fd) {
                if (send(dest_fd, buf, nbytes, 0) == -1) {
                    perror("send");
                }
            }
        }
    }
}

void* listener_func(void* argument){
    int newfd;
    struct sockaddr_storage remoteaddr;
    char remoteIP[INET6_ADDRSTRLEN];
    socklen_t addrlen;

    arguments* arg = (arguments*)(argument);


    addrlen = sizeof remoteaddr;
    newfd = accept(arg->fd.fd,
                   (struct sockaddr *)&remoteaddr,
                   &addrlen);

    if (newfd == -1) {
        perror("accept");
    } else {
        arg->react->InstallHandler(arg->react, &recv_send, newfd);
        printf("pollserver: new connection from %s on "
               "socket %d\n",
               inet_ntop(remoteaddr.ss_family,
                         get_in_addr((struct sockaddr*)&remoteaddr),
                         remoteIP, INET6_ADDRSTRLEN),
               newfd);
    }
}


int main(){
    Reactor reactor = new Reactor();

    reactor.InstallHandler();
}