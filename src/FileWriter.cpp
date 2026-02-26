#include "FileWriter.hpp"
#include <fstream>

void FileWriter::write(std::string contents){
    std::ofstream file(file_path);//open file

    file << contents;//write contents to it
}
