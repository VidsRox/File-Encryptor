#ifndef SHAREDQUEUE_HPP
#define SHAREDQUEUE_HPP
#include "Task.hpp"
#include <semaphore.h>
#include <pthread.h>
#include <sys/mman.h>
#include "Cryptor.hpp"
#include <vector>
#include <unistd.h>
#include "FileReader.hpp"
#include "FileWriter.hpp"
#include <sys/wait.h>

struct SharedTask{
    char file_path[256]; //didn't use std::string as it internally heap-allocates its character data:
                        // the pointer points to private heap memory, inaccessible to other processes.
    Action action;
};

struct SharedQueue{
    SharedTask s_t[100];
    int head;
    int tail;
    int size;
    int capacity;
    pthread_mutex_t mtx;/*std::mutex cannot be placed in shared memory 
                        and shared across processes. pthread_mutex_t 
                        can - when initialized with PTHREAD_PROCESS_SHARED 
                        attribute it works across processes.*/
    sem_t items;
    sem_t spaces;
    bool stop;
};

SharedQueue* create_shared_queue();

//main adding a task
void enqueue(SharedQueue* q, SharedTask task);

//worker taking task
SharedTask dequeue(SharedQueue* q);

class MultiProcessManager{
    private:
        SharedQueue* q;
        Cryptor cryptor;
        int processes;
        std::vector<pid_t> pid;
    
    public:
        //constructor    
        MultiProcessManager(char key, int p): cryptor(key), processes(p) {
                q = create_shared_queue();/*shared memory needs to be created before forking, 
                                        so that both parent and child see the same region*/
        }

        void Submit(SharedTask t);

        void process();

        void Start();

        void Shutdown();

};

#endif