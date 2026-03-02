#include "SharedQueue.hpp"
#include "Cryptor.hpp"
#include <thread>
#include "FileReader.hpp"
#include "FileWriter.hpp"

class HybridManager{
    private:
        SharedQueue* q;
        Cryptor cryptor;
        int processes;
        int threads_per_process;
        std::vector<pid_t> pid;

    public:
        HybridManager(char key, int p, int t_p): cryptor(key), processes(p), threads_per_process(t_p){
            q = create_shared_queue();
        }

        void submit(SharedTask t);//adds task to shared queue

        void start();//forks processes, each spawns threads
        
        void shutdown();//signals stop, wakes workers, waits for children
        
};