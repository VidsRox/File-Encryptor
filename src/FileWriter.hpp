#ifndef FILEWRITER_HPP
#define FILEWRITER_HPP
#include <string>

class FileWriter{
    private:
        std::string file_path;

    public:
        FileWriter(std::string path) : file_path(path) {}//constructor

        void write(std::string contents);
};


#endif