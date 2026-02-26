#ifndef CRYPTOR_HPP
#define CRYPTOR_HPP
#include <string>

class Cryptor{
    private:
        char encryption_key;//XOR transforms contents

    public:
        Cryptor(char key) : encryption_key(key) {}

        std::string transform(std::string contents);/*because contents is passed by value (a copy), 
                                                    the copy is being modified and returned. 
                                                    The original string is untouched.*/
};

#endif