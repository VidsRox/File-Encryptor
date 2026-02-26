#include "Cryptor.hpp"

std::string Cryptor::transform(std::string contents){
    for(int i = 0; i<contents.size(); i++){
            contents[i] = contents[i] ^ encryption_key;
        }

        return contents;
}
