//
// Created by eylon on 6/3/22.
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
#include <assert.h>


// global variables
pthread_mutex_t mutex;
pthread_cond_t cond;

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
                curr_queue->tail = curr_queue->tail->next;
            }
            ++curr_queue->capacity;
        }
    }
    pthread_cond_signal(&cond);
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
        free(temp->data);
        free(temp);
        --q->capacity;
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* top(queue_point curr_queue){
    if(curr_queue->capacity != 0){
        printf("%s\n", ((char*)curr_queue->head->data));
        return curr_queue->head->data;
    }else{
        char* data = "empty";
        printf("empty\n");
        return data;
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

int main(){
    queue_point q = (queue_point)createQ();
    char str[4096] = "eylon";



    enq_arg_point arg = (enq_arg_point)(malloc(sizeof(enq_arg)));
    arg->data = str;
    arg->q = q;

    char* data = (char*)top(q);
    assert(!strcmp(data, "empty"));

    enQ(arg);

    char* back = (char*)top(q);
    assert(!strcmp(back, "eylon"));

    strcpy(str, "michael");

    arg->data = str;
    arg->q = q;
    enQ(arg);

    strcpy(str, (char*)top(q));
    assert(!strcmp(str, "eylon"));

    deQ(q);
    strcpy(str, (char*)top(q));
    assert(!strcmp(str, "michael"));

    deQ(q);
    strcpy(str, (char*)top(q));
    assert(!strcmp(str, "empty"));

    destroyQ(q);


    // we will test waiting on cond
    // first deq with an empty queue
    // than enq a string
    //checking top, should be empty
    // and not "michael"
    queue_point q2 = (queue_point)createQ();
    char str2[4096] = "michael";

    enq_arg_point arg2 = (enq_arg_point)(malloc(sizeof(enq_arg)));
    arg2->data = str2;
    arg2->q = q2;

    pthread_t t1;
    pthread_t t2;

    pthread_create(&t1, NULL, deQ, (void*)q);
    pthread_create(&t2, NULL, enQ, arg2);
    sleep(2);
    strcpy(str2, (char*)top(q2));
    assert(!strcmp(str, "empty"));

    destroyQ(q2);
}