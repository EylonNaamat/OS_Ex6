//
// Created by eylon on 5/30/22.
//

#include "reactor.hpp"
#define PORT "9027"

//global
pthread_t t1;
pthread_t t2;
pthread_t t3;

typedef struct _install_arg{
    Reactor* reactor;
    void* (*func)(void* argument);
    int fd;
}install_arg, *install_arg_point;


void* InstallHandler(void* argu) {
    install_arg_point arg = (install_arg_point)(argu);
    if(arg->reactor->count == arg->reactor->size){
        arg->reactor->size *= 2;
        arg->reactor->fds = (struct pollfd*)(realloc(arg->reactor->fds, (sizeof(struct pollfd) * arg->reactor->size)));
        if(arg->reactor->fds == NULL){
//            return false;
        }
        arg->reactor->funcs = (fd_funcs*)(realloc(arg->reactor->funcs, (sizeof(fd_funcs) * arg->reactor->size)));
        if(arg->reactor->funcs == NULL){
//            return false;
        }
    }
    arg->reactor->fds[arg->reactor->count].fd = arg->fd;
    arg->reactor->fds[arg->reactor->count].events = POLLIN;
    arg->reactor->funcs[arg->reactor->count] = (arg->func);
    arg->reactor->count++;
//    return true;
}

void install_helper(Reactor *reactor, void* (*func)(void* argument), int fd){
    install_arg_point argu = (install_arg_point)(malloc(sizeof(install_arg)));
    argu->reactor = reactor;
    argu->func = func;
    argu->fd = fd;
    pthread_create(&t2, NULL, InstallHandler, (void*)(argu));
    pthread_join(t2, NULL);
}

void RemoveHandler(Reactor *reactor, int i) {
    reactor->fds[i] = reactor->fds[reactor->count-1];
    reactor->count--;
}

void* newreactor_help(void* newarg){
    Reactor* reactor = (Reactor*)(newarg);
    while(1){
        int poll_stat = poll(reactor->fds, reactor->count, -1);
        if(poll_stat == -1){
            printf("error\n");
        }
        for(int i = 0; i < reactor->count; ++i){
            if(reactor->fds[i].revents & POLLIN){
                arguments arg;
                arg.react = reactor;
                arg.fd = reactor->fds[i];
                arg.i = i;
                reactor->funcs[i]((void*)&arg);
            }
        }
    }

}


void *newReactor(Reactor* reactor) {
    pthread_create(&t1, NULL, newreactor_help, (void*)reactor);
    pthread_join(t1, NULL);
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

//global
int listener;

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

        RemoveHandler(arg->react, arg->i);

    } else {
        // We got some good data from a client

        for(int j = 0; j < arg->react->count; j++) {
            // Send to everyone!
            int dest_fd = arg->react->fds[j].fd;

            // Except the listener and ourselves
            if (dest_fd != sender_fd && dest_fd != listener) {
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
        install_helper(arg->react, &recv_send, newfd);
        printf("pollserver: new connection from %s on "
               "socket %d\n",
               inet_ntop(remoteaddr.ss_family,
                         get_in_addr((struct sockaddr*)&remoteaddr),
                         remoteIP, INET6_ADDRSTRLEN),
               newfd);
    }
}


int main(void)
{
    Reactor* reactor = new Reactor();


    // Set up and get a listening socket
    listener = get_listener_socket();

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }



    install_helper(reactor, listener_func, listener);
    newReactor(reactor);


    return 0;
}