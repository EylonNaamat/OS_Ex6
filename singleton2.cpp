//
// Created by eylon on 5/30/22.
//
#include <iostream>
#include <stdio.h>
#include <assert.h>

template <typename T> class Singleton{
private:
    T data;
    static Singleton<T>* instance;
    Singleton<T>(T data){
        this->data = data;
    }
    ~Singleton<T>(){}
public:
    static Singleton<T>* Instance(T data);
    static void Destroy();
};

template <typename T>
Singleton<T>* Singleton<T>::instance = 0;

template<typename T>
Singleton<T>* Singleton<T>::Instance(T data) {
    if(instance == nullptr){
        instance = new Singleton<T>(data);
    }
    return instance;
}

template <typename T>
void Singleton<T>::Destroy(){
    if(instance){
        instance = 0;
    }
}

int main(){
    FILE* file;
    Singleton<FILE*>* s1 = Singleton<FILE*>::Instance(file);
    Singleton<FILE*>* s2 = Singleton<FILE*>::Instance(file);
    std::cout << s1 << '\n';
    std::cout << s2 << '\n';
    assert(s1 == s2);
    Singleton<FILE*>::Destroy();
    Singleton<FILE*>* s3 = Singleton<FILE*>::Instance(file);
    std::cout << s3 << '\n';
    assert(s1 != s3);
    assert(s2 != s3);
}