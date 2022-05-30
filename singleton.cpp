//
// Created by eylon on 5/30/22.
//
#include <assert.h>
#include <stdio.h>
#include <iostream>

template <typename T> class Singleton{
private:
    static T* instance;
    Singleton(){}
    ~Singleton(){}
public:
    static T* Instance();
    static void destroy();
};

template <typename T>
T* Singleton<T>::instance = 0;

template <typename T>
T* Singleton<T>::Instance() {
    if(!instance){
        instance = new T();
    }
    return instance;
}

template <typename T>
void Singleton<T>::destroy(){
    if(instance){
        instance = 0;
    }
}


int main(){
    FILE** sing1 = Singleton<FILE*>::Instance();
    FILE** sing2 = Singleton<FILE*>::Instance();
    assert(sing1 == sing2);
    std::cout << sing1 << '\n';
    std::cout << sing2 << '\n';

    Singleton<FILE*>::destroy();

    FILE** sing3 = Singleton<FILE*>::Instance();
    assert(sing3 != sing1);
    assert(sing3 != sing2);
    std::cout << sing3 << '\n';


    return 0;
}