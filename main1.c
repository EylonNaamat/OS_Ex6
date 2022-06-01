//
// Created by eylon on 5/30/22.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <ctype.h>

#define PORT "3499"
#define BACKLOG 10

// queue node
typedef struct Queue_node{
    int fd;
    void* data;
    struct Queue_node* next;

}Queue_node, *queue_node_point;

// queue
typedef struct Queue{
    queue_node_point head;
    queue_node_point tail;
    int capacity;
} Queue, *queue_point;

// global variables
pthread_mutex_t mutex;
pthread_cond_t cond;
int new_fd[10];

void* createQ(){
    queue_point new_queue = (queue_point)(malloc(sizeof(Queue)));
    if(new_queue){
        new_queue->head = NULL;
        new_queue->tail = NULL;
        new_queue->capacity = 0;
    }
    return new_queue;
}

//struct of arguments to enqueue
typedef struct _enq_arg{
    queue_point q;
    void* data;
    int fd;
}enq_arg, *enq_arg_point;


void* enQ(void* argument){
    pthread_mutex_lock(&mutex);
    enq_arg_point arg = enq_arg_point(argument);
    queue_point curr_queue = arg->q;
    char* data = (char*)(arg->data);
    int fd = arg->fd;
    queue_node_point new_elem = (queue_node_point)(malloc(sizeof(Queue_node)));
    if(new_elem){
        new_elem->fd = fd;
        new_elem->data = (char*)(malloc(strlen(data)+1));
        if(!new_elem->data){
            free(new_elem);
        }else{
            new_elem->next = NULL;
            memcpy(new_elem->data, data, strlen(data)+1);
            if(curr_queue->capacity == 0){
                curr_queue->head = new_elem;
                curr_queue->tail = new_elem;
            }else{
                curr_queue->tail->next = new_elem;
                curr_queue->tail = curr_queue->tail->next;
            }
            ++curr_queue->capacity;
        }
    }
    pthread_cond_signal(&cond);
    printf("release from cond\n");
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* deQ(void* curr_queue){
    pthread_mutex_lock(&mutex);
    queue_point q = (queue_point)curr_queue;
    queue_node_point temp = NULL;
    if(q->capacity == 0) {
        printf("entering cond\n");
        pthread_cond_wait(&cond, &mutex);
        printf("out of cond\n");
    }
    if(q->capacity != 0){
        temp = q->head;
        if(q->capacity == 1){
            q->head = NULL;
            q->tail = NULL;
        }else{
            q->head = q->head->next;
        }
        --q->capacity;
    }
    pthread_mutex_unlock(&mutex);
    return temp;
}

void top(queue_point curr_queue){
    if(curr_queue->capacity != 0){
        printf("%s\n", ((char*)curr_queue->head->data));
//        printf("%d\n", curr_queue->head->fd);
    }else{
        printf("empty\n");
    }
}


void clear(queue_point curr_queue){
    while(curr_queue->capacity){
        deQ(curr_queue);
    }
}

void destroyQ(queue_point curr_queue){
    clear(curr_queue);
    free(curr_queue);
}


// active object

typedef struct _active_object{
    queue_point q;
    void (*func)(queue_node_point node);
    void (*after_func)(queue_node_point node);
}active_object, *active_object_point;


void* newAO(queue_point q, void (*func)(queue_node_point node), void (*after_func)(queue_node_point node)){

        queue_node_point node = (queue_node_point)deQ(q);
        func(node);
        after_func(node);
        printf("%s\n", (char*)(node->data));

}

void destroyAO(active_object_point ao){
    clear(ao->q);
}

// all the queues
queue_point q1;
queue_point q2;
queue_point q3;

void caesar_cipher(queue_node_point node){
    char* data = (char*)(node->data);
    for(int i = 0; data[i] != '\0'; ++i){
        if(data[i] >= 'a' && data[i] <= 'z'){
            data[i] += 1;
            if(data[i] > 'z'){
                data[i] = data[i] - 'z' + 'a' - 1;
            }
        }else if(data[i] >= 'A' && data[i] <= 'Z'){
            data[i] += 1;
            if(data[i] > 'Z'){
                data[i] = data[i] - 'Z' + 'A' - 1;
            }
        }
    }
}

void small_to_big(queue_node_point node){
    char* data = (char*)(node->data);
    for(int i = 0; data[i] != '\0'; ++i){
        if(data[i] >= 'a' && data[i] <= 'z'){
            data[i] = toupper(data[i]);
        }else if(data[i] >= 'A' && data[i] <= 'Z'){
            data[i] = tolower(data[i]);
        }
    }
}

void enq_2(queue_node_point head){
    enq_arg_point arg = (enq_arg_point)(malloc(sizeof(enq_arg)));
    arg->data = head->data;
    arg->q = q2;
    arg->fd = head->fd;
    enQ(arg);
}

void enq_3(queue_node_point head){
    enq_arg_point arg = (enq_arg_point)(malloc(sizeof(enq_arg)));
    arg->data = head->data;
    arg->q = q3;
    arg->fd = head->fd;
    enQ(arg);
}


void send_to_client(queue_node_point node){
    char* buf = (char*)node->data;
    int fd = node->fd;
    send(fd, buf, strlen(buf), 0);
}
void nothing(queue_node_point head){
    return;
}



//server funcs
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


void* sender(void* arg)
{
    int * new_fd = (int*)arg;
    char buf[2048];

    while(recv(*new_fd, buf, 2048, 0) != -1) {
        size_t ln = strlen(buf) - 1;
        if (buf[ln] == '\n') {
            buf[ln] = '\0';
        }
        enq_arg_point arg = enq_arg_point(malloc(sizeof(enq_arg)));;
        arg->data = buf;
        arg->q = q1;
        arg->fd = *new_fd;
        enQ(arg);
        newAO(q1, caesar_cipher, enq_2);
        newAO(q2, small_to_big, enq_3);
        newAO(q3, send_to_client, nothing);
    }
    close((*new_fd));

}


int main(void)
{
    int sockfd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    int i = 0;
    pthread_t thread_id[10];



    q1 = (queue_point)createQ();
    q2 = (queue_point)createQ();
    q3 = (queue_point)createQ();

    pthread_mutex_init(&mutex, NULL);

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd[i] = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd[i] == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        if(pthread_create(&thread_id[i], NULL, sender, (&new_fd[i])) != 0){
            printf("thread creation failed\n");
        }
        i++;
        if(i >= 5){
            for(int j=0;j<=i;j++)
            {
                pthread_join(thread_id[j], NULL);
            }
            i = 0;
        }
    }
    return 0;
}

