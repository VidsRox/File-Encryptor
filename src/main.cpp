#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <mutex>
#include <thread>
#include <filesystem>

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

    public:
        ProcessManager(char key): cryptor(key) {}

        void submit(Task task){
            t.push(task);
        }

        void process() {
            while(true) {
                unique_lock<mutex> lock(mtx);
                if(t.empty()){
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
   char key = read_encryption_key(".env");
   
   ProcessManager pm(key);

   Action action = (string(argv[2]) == "ENCRYPT") ? Action::ENCRYPT : Action::DECRYPT;

   for (auto& entry : fs::directory_iterator(argv[1])){
        pm.submit(Task(entry.path().string(), action));
   }
   
   thread t1(&ProcessManager::process, &pm);
   thread t2(&ProcessManager::process, &pm);

   t1.join();
   t2.join();

}