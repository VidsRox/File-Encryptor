#include "ProcessManager.hpp"
#include "FileReader.hpp"
#include "FileWriter.hpp"

void ProcessManager::submit(Task task){
        std::unique_lock<std::mutex> lock(mtx);
        t.push(task);
        cv.notify_one();
}

void ProcessManager::shutdown(){
    std::unique_lock<std::mutex> lock(mtx);
    stop = true;
    cv.notify_all();
}

void ProcessManager::process(){
    while(true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !t.empty() || stop; });

        if(t.empty() && stop){
            break;
        }
        
        Task task = t.front();//peek at front
        t.pop();//remove it
        lock.unlock();
        
        std::string contents = FileReader(task.file_path).read();//get the contents

        std::string transformed_contents = cryptor.transform(contents);

        FileWriter(task.file_path).write(transformed_contents);
    }
}