//
// Created by eylon on 5/30/22.
//
#include <pthread.h>
#include <assert.h>

// answer to question in section 4
// you cant use this guard in order to protect strtok and make it thread safe
// because if a thread A enters this function with a string and delimiter he locks the function with guard
// and when he exits from the function he releases the guard.
// after that another thread B enters this function with a different string and different delimiter and updates the global
// variable, of course he locks the function but when he exits the function he releases the guard
// then thread A will call the strktok with NULL and his delimiter, but thread B already updated the global variable
// and thread A expects that the global variable to be the one he entered.
// so in conclusion you cant make strtok thread safe using this guard, because this guard prevents other threads to enter
// the function when another thread is in, but it doesnt prevent them from changing the global variable after the
// thread is done.

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
    return NULL;
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