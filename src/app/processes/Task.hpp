#ifndef TASK_HPP
#define TASK_HPP
#include <string>
#include <iostream>
#include "../fileHandling/IO.hpp"
#include <sstream>

enum class Action {
    ENCRYPT=1,
    DECRYPT=0
};

struct Task{
    std:: string filePath;
    std:: fstream f_stream;
    Action action;

    Task(std::fstream &&file_stream,Action act, std::string filePath): f_stream(std::move(file_stream)), action(act), filePath(filePath) {}
    
    std:: string toString() const {
        std::ostringstream oss;
        oss << filePath << ",";
        if (action == Action::ENCRYPT) {
            oss << "Encrypt";
        } else {
            oss << "Decrypt";
        }
        return oss.str();
    }

    static Task fromString(const std::string &taskData);
};





#endif