#include "envReader.hpp"
#include "FileReader.hpp"

char read_encryption_key(std::string env_file_path){
    
    std::string line = FileReader(env_file_path).read();

    int index = line.find('=');

    std::string value = line.substr(index+1);
    return value[0];
}