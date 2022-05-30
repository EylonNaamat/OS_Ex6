//
// Created by eylon on 5/30/22.
//
#include <pthread.h>
#include <assert.h>

class Guard{
private:
    pthread_mutex_t* lock;
public:
    Guard(pthread_mutex_t* lock){
        this->lock = lock;
        pthread_mutex_lock(this->lock);
    }
    ~Guard(){
        pthread_mutex_unlock(this->lock);
    }
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int counter;

void* inc(void* arg){
    Guard guard(&mutex);
    for(int i = 0; i < 100; ++i){
        ++counter;
    }
}

int main(){
    pthread_t thread1;
    pthread_t thread2;

    pthread_create(&thread1, NULL, inc, NULL);
    pthread_create(&thread2, NULL, inc, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    assert(counter == 200);

    return 0;
}