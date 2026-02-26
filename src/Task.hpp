#ifndef TASK_HPP
#define TASK_HPP
#include <string>

enum class Action {
    ENCRYPT,
    DECRYPT
};

struct Task{//describes a job (file path + action)
    std::string file_path;
    Action action;

    Task(std::string path, Action act): file_path(path), action(act) {}
};


#endif