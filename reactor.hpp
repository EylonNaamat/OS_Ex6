//
// Created by eylon on 5/31/22.
//
#include <poll.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>

typedef void* (*fd_funcs) (void* argument);

class Reactor{
public:
    struct pollfd* fds;
    fd_funcs* funcs;
    int count;
    int size;
    pthread_t thread_id;
public:
    Reactor(){
        this->fds = (struct pollfd*)(malloc(sizeof(struct pollfd) * 10));
        this->funcs = (fd_funcs *)(malloc(sizeof(fd_funcs) * 10));
        this->count = 0;
        this->size = 10;
    }
    ~Reactor(){}
    void* newReactor();
    bool InstallHandler(Reactor* reactor, void* (*func)(void*), int fd);
    void RemoveHandler(Reactor* reactor, int i);
};

typedef struct _arguments{
    Reactor* react;
    struct pollfd fd;
    int i;

}arguments;


void* recv_send(void* argument);
void* listener_func(void* argument);

void *get_in_addr(struct sockaddr *sa);
int get_listener_socket(void);


