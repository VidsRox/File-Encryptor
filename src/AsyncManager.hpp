#include <string>
#include "Task.hpp"
#include <fcntl.h>      // open()
#include <sys/stat.h>   // stat()
#include <liburing.h>   // io_uring
#include "Cryptor.hpp"
#include <vector>
#include "FileReader.hpp"
#include "FileWriter.hpp"
struct IOContext{
    int fd;//file descriptor
    char* buffer;
    std::string path;
    Action action;
    bool is_read;// true = read just completed, false = write just completed
    int size;
};

class AsyncManager{
    private:
        struct io_uring ring;
        int queue_depth;
        int in_flight;
        Cryptor cryptor;
        std::vector<std::string> file_path;
        Action action;

    public:
        AsyncManager(char key, int q_d, Action a) : cryptor(key), queue_depth(q_d), action(a), in_flight(0) {}
        
        //collect all paths first
        void add_file(std::string path);
        
        
        void process();
};