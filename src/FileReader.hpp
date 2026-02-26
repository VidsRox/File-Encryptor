#ifndef FILEREADER_HPP
#define FILEREADER_HPP
#include <string>


class FileReader{
    private:
        std::string file_path;

    public:
        FileReader(std::string path) : file_path(path) {} //constructor

        std::string read();
};

#endif