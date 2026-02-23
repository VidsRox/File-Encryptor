#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <mutex>

class FileReader{//reads a file's contents
    private:
        std::string file_path;

    public:
        FileReader(std::string path) : file_path(path) {}//constructor
        
        std::string read() {
            std::ifstream file(file_path);//open the file first

            std::string contents((std::istreambuf_iterator<char>(file)), 
                                std::istreambuf_iterator<char>());

            return contents;
        }
};

class FileWriter{//write contents to a file
    private:
    std::string file_path;

    public:
        FileWriter(std::string path) : file_path(path) {}//constructor
        
        void write(std::string contents) {
            std::ofstream file(file_path);//open file

            file << contents;//write contents to it
        }
};

enum class Action {
    ENCRYPT,
    DECRYPT
};

struct Task{//describes a job (file path + action)
    std::string file_path;
    Action action;

    Task(std::string path, Action act): file_path(path), action(act) {}
};

class Cryptor{//XOR transforms contents
    private:
        char encryption_key;

    public:
        Cryptor(char key) : encryption_key(key) {}

        std::string transform(std::string contents) {/*because contents is passed by value (a copy), 
                                                    the copy is being modified and returned. 
                                                    The original string is untouched.*/
        for(int i = 0; i<contents.size(); i++){
            contents[i] = contents[i] ^ encryption_key;
        }

        return contents;
        }
};

char read_encryption_key(std::string env_file_path){
    
    std::string line = FileReader(env_file_path).read();

    int index = line.find('=');

    std::string value = line.substr(index+1);
    return value[0];
}

class ProcessManager{
    private:
        std::queue<Task> t; 
        Cryptor cryptor;

    public:
        ProcessManager(char key): cryptor(key) {}

        void submit(Task task){
            t.push(task);
        }

        void process() {
            while(!t.empty()) {
                Task task = t.front();//peek at front
                t.pop();//remove it
                
                std::string contents = FileReader(task.file_path).read();//get the contents

                std::string transformed_contents = cryptor.transform(contents);

                FileWriter(task.file_path).write(transformed_contents);
            }
        }
};

int main() {
   char key = read_encryption_key(".env");
   
   ProcessManager pm(key);

    Task t("test.txt", Action::ENCRYPT);

   pm.submit(t);
   pm.process();

}