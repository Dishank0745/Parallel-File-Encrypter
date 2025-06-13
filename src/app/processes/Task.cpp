#include "Task.hpp"
#include <algorithm> // Required for std::transform
#include <cctype>    // Required for ::toupper

Task Task::fromString(const std::string &taskData) {
    std::istringstream iss(taskData);
    std::string filePath;
    std::string actionStr;

    if(std::getline(iss, filePath, ',') && std::getline(iss, actionStr)) {
        std::transform(actionStr.begin(), actionStr.end(), actionStr.begin(),
                       ::toupper); 

        Action action = (actionStr == "ENCRYPT") ? Action::ENCRYPT : Action::DECRYPT;
        
        IO io(filePath);
        std::fstream f_stream = std::move(io.getFileStream());
        if(f_stream.is_open()) {
            return Task(std::move(f_stream), action, filePath);
        } else {
            throw std::runtime_error("Failed to open file: " + filePath);
        }
    } else {
        throw std::runtime_error("Invalid task data format");
    }
}