//
// Created by eylon on 5/30/22.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

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

queue_point createQ(){
    queue_point new_queue = (queue_point)(malloc(sizeof(Queue)));
    if(new_queue){
        new_queue->head = NULL;
        new_queue->tail = NULL;
        new_queue->capacity = 0;
    }
    return new_queue;
}

void enQ(queue_point curr_queue, void* data){
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond);
    queue_node_point new_elem = (queue_node_point)(malloc(sizeof(Queue_node)));
    if(new_elem){
        new_elem->data = (char*)(malloc(strlen((char*)data)+1));
        if(!new_elem->data){
            free(new_elem);
        }else{
            new_elem->next = NULL;
            memcpy(new_elem->data, data, 4096*sizeof(char));
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
    pthread_mutex_unlock(&mutex);
}

void deQ(queue_point curr_queue){
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond);
    if(curr_queue->capacity != 0){
        queue_node_point temp = curr_queue->head;
        if(curr_queue->capacity == 1){
            curr_queue->head = NULL;
            curr_queue->tail = NULL;
        }else{
            curr_queue->head = curr_queue->head->next;
        }
        free(temp->data);
        free(temp);
        --curr_queue->capacity;
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
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