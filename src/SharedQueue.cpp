#include "SharedQueue.hpp"

SharedQueue* create_shared_queue(){
    
    SharedQueue* queue = (SharedQueue*)mmap(
            NULL, sizeof(SharedQueue), 
            PROT_READ | PROT_WRITE, 
            MAP_SHARED | MAP_ANONYMOUS, 
            -1, 0
            );

    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    queue->capacity = 100;
    queue->stop = false;

    pthread_mutexattr_t attr; //mutex with process-shared attributes
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    
    pthread_mutex_init(&queue->mtx, &attr);

    //semaphores-
    sem_init(&queue->items, 1, 0);//shared=1, inital value=0;
    sem_init(&queue->spaces, 1, 100);

    return queue;
}

void enqueue(SharedQueue* q, SharedTask task){
    sem_wait(&q->spaces);//wait for free slot
    pthread_mutex_lock(&q->mtx);

    q->s_t[q->tail] = task;
    q->tail = (q->tail + 1) % q->capacity;
    q->size++;
    pthread_mutex_unlock(&q->mtx);

    sem_post(&q->items);//signal task availability 
}

SharedTask dequeue(SharedQueue* q){
    sem_wait(&q->items);//wait for a task
    pthread_mutex_lock(&q->mtx);

    SharedTask task = q->s_t[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->size--; 

    pthread_mutex_unlock(&q->mtx);
    sem_post(&q->spaces);//signal free slot
    return task;
}

void MultiProcessManager::Submit(SharedTask t){
        enqueue(q, t);
    }

void MultiProcessManager::process() {
    while(!q->stop){
        SharedTask task = dequeue(q);
        if(q->stop) break;

        std::string contents = FileReader(task.file_path).read();//get the contents

        std::string transformed_contents = cryptor.transform(contents);

        FileWriter(task.file_path).write(transformed_contents);
    }
}

void MultiProcessManager::Start(){
    for(int i = 0; i<processes; i++){
        pid_t p = fork();
        if(p==0){//child process
            process();
            exit(0);//child exits after processing
        } else{
            pid.push_back(p);//parent stores child PID
        }
    }
}

void MultiProcessManager::Shutdown(){
    q->stop=true;
    for(int i = 0; i<processes; i++){
        sem_post(&q->items); //wake each blocked worker
    }
    for(pid_t p: pid){
        waitpid(p, NULL, 0); //wait for each child
    }
}



