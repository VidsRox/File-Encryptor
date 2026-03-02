#include <iostream>
#include <cstring>
#include "envReader.hpp"
#include "FileWriter.hpp"
#include "Task.hpp"
#include "Cryptor.hpp"
#include "envReader.hpp"
#include "ProcessManager.hpp"
#include "SharedQueue.hpp"
#include "HybridManager.hpp"
#include <thread>
#include <filesystem>
#include <chrono>

using namespace std;
namespace fs = std::filesystem; // alias to avoid typing std::filesystem every time


int main(int argc, char* argv[]) {

    if(argc < 5) {
    cout << "Usage: ./encrypt <dir> <ENCRYPT|DECRYPT> <num> <threaded|multiprocess|hybrid>" << endl;
    cout << "       ./encrypt <dir> <ENCRYPT|DECRYPT> <processes> <threads_per_process> hybrid" << endl;
    return 1;
    }

    
    auto start = std::chrono::high_resolution_clock::now();

    char key = read_encryption_key(".env");

    if(string(argv[2]) != "ENCRYPT" && string(argv[2]) != "DECRYPT") {
    cout << "Error: action must be ENCRYPT or DECRYPT" << endl;
    return 1;
    }
    Action action = (string(argv[2]) == "ENCRYPT") ? Action::ENCRYPT : Action::DECRYPT;

    int num = stoi(argv[3]);//threads/processes
    vector<thread> threads;

    if(string(argv[4]) == "threaded"){
        ProcessManager pm(key);

        for(int i = 0; i<num; i++){
            threads.push_back(thread(&ProcessManager::process, &pm));
        }

        for (auto& entry : fs::directory_iterator(argv[1])){
                pm.submit(Task(entry.path().string(), action));
        }
        
        pm.shutdown();

        for(auto& t : threads) {
            t.join();
            }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        cout << "Time with " << argv[3] << " threads: " << duration.count() << "ms" << endl;
        
    } else if(string(argv[4]) == "multiprocess") {
        MultiProcessManager mpm(key, num);
        mpm.Start();

        SharedTask t;

        for (auto& entry : fs::directory_iterator(argv[1])){
            strncpy(t.file_path, entry.path().string().c_str(), 255);
            t.action = action;
            mpm.Submit(t);
        }

        mpm.Shutdown();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        cout << "Time with " << argv[3] << " processes: " << duration.count() << "ms" << endl;
    
    } else if (string(argv[5]) == "hybrid"){
        int num_processes = stoi(argv[3]);
        int num_threads = stoi(argv[4]);

        HybridManager hm(key, num_processes, num_threads);
        hm.start();

        SharedTask t;

        for (auto& entry : fs::directory_iterator(argv[1])){
            strncpy(t.file_path, entry.path().string().c_str(), 255);
            t.action = action;
            hm.submit(t);
        }

        hm.shutdown();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        cout << "Time with " << num_processes << " processes x " << num_threads << " threads: " << duration.count() << "ms" << endl;

    }

}
