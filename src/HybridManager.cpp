#include "HybridManager.hpp"

void HybridManager::submit(SharedTask t){
    enqueue(q, t);
}

void HybridManager::start(){
    for(int i = 0; i<processes; i++){
        pid_t p = fork();
        if(p==0){//child process
            
            std::vector<std::thread> threads;
            
            for(int j = 0; j<threads_per_process; j++){
                threads.push_back(std::thread([this](){
                    while(!q->stop){
                        SharedTask task = dequeue(q);
                        if(q->stop) break;
                        
                        //read, write, transform
                        std::string contents = FileReader(task.file_path).read();//get the contents

                        std::string transformed_contents = cryptor.transform(contents);

                        FileWriter(task.file_path).write(transformed_contents);
                    }
                }));
            }

            for(auto& t: threads) {
                t.join();
            }
            exit(0);//child exits after processing
        
        } else{
            pid.push_back(p);//parent stores child PID
        }
    }
}

void HybridManager::shutdown(){
    q->stop=true;
    int total_threads = processes * threads_per_process;
    for(int i = 0; i<total_threads; i++){
        sem_post(&q->items); //wake each blocked worker
    }
    for(pid_t p: pid){
        waitpid(p, NULL, 0); //wait for each child
    }
}