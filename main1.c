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

#define PORT "3498"
#define BACKLOG 10

// queue node
typedef struct Queue_node{
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

typedef struct _enq_arg{
    queue_point q;
    void* data;
}enq_arg, *enq_arg_point;



void* enQ(void* argument){
    pthread_mutex_lock(&mutex);
    enq_arg_point arg = enq_arg_point(argument);
    queue_point curr_queue = arg->q;
    char* data = (char*)(arg->data);
    queue_node_point new_elem = (queue_node_point)(malloc(sizeof(Queue_node)));
    if(new_elem){
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
                curr_queue->tail = new_elem;
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
    queue_point q = (queue_point)curr_queue;
    pthread_mutex_lock(&mutex);
    if(q->capacity == 0) {
        printf("entering cond\n");
        pthread_cond_wait(&cond, &mutex);
        printf("out of cond\n");
    }
    if(q->capacity != 0){
        queue_node_point temp = q->head;
        if(q->capacity == 1){
            q->head = NULL;
            q->tail = NULL;
        }else{
            q->head = q->head->next;
        }
        free(temp->data);
        free(temp);
        --q->capacity;
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void top(queue_point curr_queue){
    if(curr_queue->capacity != 0){
        printf("%s\n", ((char*)curr_queue->head->data));
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
    void (*func)(struct _active_object* ao);
    void (*after_func)(struct _active_object* ao);
}active_object, *active_object_point;


active_object_point newAO(queue_point q, void (*func)(struct _active_object* ao), void (*after_func)(struct _active_object* ao)){
    active_object_point ao = (active_object_point)(malloc(sizeof(active_object)));
    ao->q = q;
    ao->func = func;
    ao->after_func = after_func;
    return ao;
}

void destroyAO(active_object_point ao){
    clear(ao->q);
}

void caesar_cipher(active_object_point ao){
    for(int i = 0; ao->buf[i] != '\0'; ++i){
        if(ao->buf[i] >= 'a' && ao->buf[i] <= 'z'){
            ao->buf[i] += 1;
            if(ao->buf[i] > 'z'){
                ao->buf[i] = ao->buf[i] - 'z' + 'a' - 1;
            }
        }else if(ao->buf[i] >= 'A' && ao->buf[i] <= 'Z'){
            ao->buf[i] += 1;
            if(ao->buf[i] > 'Z'){
                ao->buf[i] = ao->buf[i] - 'Z' + 'A' - 1;
            }
        }
    }
}

void small_to_big(active_object_point ao){
    for(int i = 0; ao->buf[i] != '\0'; ++i){
        if(ao->buf[i] >= 'a' && ao->buf[i] <= 'z'){
            ao->buf[i] = toupper(ao->buf[i]);
        }else if(ao->buf[i] >= 'A' && ao->buf[i] <= 'Z'){
            ao->buf[i] = tolower(ao->buf[i]);
        }
    }
}

void enq_other_queue(active_object_point src, active_object_point dest){
    char data [4096];
    strcpy(data, (char*)src->q->head->data);
    enq_arg_point arg = (enq_arg_point)(malloc(sizeof(enq_arg)));
    arg->data = data;
    arg->q = dest->q;
    enQ(arg);
}




int main(){
    queue_point q = (queue_point)createQ();
    char* str = "eylon";
//    enQ(q, str);
//    top(q);
//    deQ(q);
//    top(q);
//    str = "naamat";
//    enQ(q, str);
//    top(q);

//    pthread_t t1;
//    pthread_t t2;
//    pthread_t t3;
////    pthread_create(&t1, NULL, deQ, (void*)q);
//
//    enq_arg_point arg;
//    arg->data = str;
//    arg->q = q;
//
//    pthread_create(&t1, NULL, deQ, (void*)q);
//    sleep(2);
////    top(q);
//
//    pthread_create(&t2, NULL, enQ, arg);
//    sleep(2);
//    top(q);
//
//    pthread_create(&t3, NULL, enQ, arg);
//    sleep(2);
//    top(q);

//    char buff[4096];
//    strcpy(buff, "hello");
//    strcpy(buff, "world");
//    printf("%s\n", buff);
//
//    active_object_point ao = (active_object_point)(malloc(sizeof(active_object)));
//    strcpy(ao->buf, "azAZ");
//    caesar_cipher(ao);
//    printf("%s\n", ao->buf);
//    small_to_big(ao);
//    printf("%s\n", ao->buf);




    active_object_point ao1 = newAO(q, caesar_cipher, small_to_big);
    enq_arg_point arg1 = enq_arg_point(malloc(sizeof(enq_arg)));

    queue_point q2 = (queue_point)createQ();
    active_object_point ao2 = newAO(q2, caesar_cipher, small_to_big);
//    enq_arg_point arg2 = enq_arg_point(malloc(sizeof(enq_arg)));

    arg1->q = ao1->q;
    arg1->data = str;
    enQ(arg1);
    top(ao1->q);
    enq_other_queue(ao1, ao2);
    top(ao2->q);

//    strcpy(ao1->buf, "azAZ");
//    printf("%s\n", ao1->buf);
//    ao1->func(ao1);
//    printf("%s\n", ao1->buf);
//    ao1->after_func(ao1);
//    printf("%s\n", ao1->buf);


//    sleep(2);
//    top(q);
    return 0;
}