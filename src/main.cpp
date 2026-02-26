#include <iostream>
#include <cstring>
#include "envReader.hpp"
#include "FileWriter.hpp"
#include "Task.hpp"
#include "Cryptor.hpp"
#include "envReader.hpp"
#include "ProcessManager.hpp"
#include "SharedQueue.hpp"
#include <thread>
#include <filesystem>
#include <chrono>

using namespace std;
namespace fs = std::filesystem; // alias to avoid typing std::filesystem every time


int main(int argc, char* argv[]) {

    if(argc < 5) {
    cout << "Usage: ./encrypt <directory> <ENCRYPT|DECRYPT> <num> <threaded|multiprocess>" << endl;
    return 1;
    }

    
    auto start = std::chrono::high_resolution_clock::now();

    char key = read_encryption_key(".env");

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
    }

}
