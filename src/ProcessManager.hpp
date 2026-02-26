#ifndef PROCESSMANAGER_HPP
#define PROCESSMANAGER_HPP
#include <queue>
#include <mutex>
#include <condition_variable>
#include "Task.hpp"
#include "Cryptor.hpp"

class ProcessManager{
    private:
        std::queue<Task> t; 
        Cryptor cryptor;
        std::mutex mtx;
        std::condition_variable cv;
        bool stop = false;

    public:
        ProcessManager(char key): cryptor(key) {}

        void submit(Task task);

        void shutdown();

        void process();
};

#endif