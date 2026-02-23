#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <mutex>
#include <thread>
#include <filesystem>
#include <condition_variable>
#include <chrono>

using namespace std;
namespace fs = std::filesystem; // alias to avoid typing std::filesystem every time

class FileReader{//reads a file's contents
    private:
        string file_path;

    public:
        FileReader(string path) : file_path(path) {}//constructor
        
        string read() {
            ifstream file(file_path);//open the file first

            string contents((std::istreambuf_iterator<char>(file)), 
                                std::istreambuf_iterator<char>());

            return contents;
        }
};

class FileWriter{//write contents to a file
    private:
    string file_path;

    public:
        FileWriter(string path) : file_path(path) {}//constructor
        
        void write(string contents) {
            ofstream file(file_path);//open file

            file << contents;//write contents to it
        }
};

enum class Action {
    ENCRYPT,
    DECRYPT
};

struct Task{//describes a job (file path + action)
    string file_path;
    Action action;

    Task(string path, Action act): file_path(path), action(act) {}
};

class Cryptor{//XOR transforms contents
    private:
        char encryption_key;

    public:
        Cryptor(char key) : encryption_key(key) {}

        string transform(string contents) {/*because contents is passed by value (a copy), 
                                                    the copy is being modified and returned. 
                                                    The original string is untouched.*/
        for(int i = 0; i<contents.size(); i++){
            contents[i] = contents[i] ^ encryption_key;
        }

        return contents;
        }
};

char read_encryption_key(std::string env_file_path){
    
    string line = FileReader(env_file_path).read();

    int index = line.find('=');

    string value = line.substr(index+1);
    return value[0];
}

class ProcessManager{
    private:
        std::queue<Task> t; 
        Cryptor cryptor;
        mutex mtx;
        condition_variable cv;
        bool stop = false;

    public:
        ProcessManager(char key): cryptor(key) {}

        void submit(Task task){
            unique_lock<mutex> lock(mtx);
            t.push(task);
            cv.notify_one();
        }

        void shutdown(){
            unique_lock<mutex> lock(mtx);
            stop = true;
            cv.notify_all();
        }

        void process() {
            while(true) {
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [this]{ return !t.empty() || stop; });

                if(t.empty() && stop){
                    break;
                }
                
                Task task = t.front();//peek at front
                t.pop();//remove it
                lock.unlock();
                
                string contents = FileReader(task.file_path).read();//get the contents

                string transformed_contents = cryptor.transform(contents);

                FileWriter(task.file_path).write(transformed_contents);
            }
        }
};

int main(int argc, char* argv[]) {

    if(argc < 4) {
        cout << "Usage: ./encrypt <directory> <ENCRYPT|DECRYPT> <num_threads>" << endl;
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();

   char key = read_encryption_key(".env");
   
   ProcessManager pm(key);

   Action action = (string(argv[2]) == "ENCRYPT") ? Action::ENCRYPT : Action::DECRYPT;

   int num_threads = stoi(argv[3]);
   vector<thread> threads;

   for(int i = 0; i<num_threads; i++){
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
}