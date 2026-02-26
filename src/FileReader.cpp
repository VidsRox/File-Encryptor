#include "FileReader.hpp"
#include <fstream>

std::string FileReader::read() {
    std::ifstream file(file_path);//open the file first

            std::string contents((std::istreambuf_iterator<char>(file)), 
                                std::istreambuf_iterator<char>());

            return contents;
        }
