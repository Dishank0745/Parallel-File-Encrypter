#include "Cryption.hpp"
#include "../processes/Task.hpp"
#include "../fileHandling/ReadEnv.cpp"


int executeCryption(const std::string &taskData){
    Task task = Task::fromString(taskData);

     std::cout << "DEBUG: task.action = " << (int)task.action << std::endl;
    std::cout << "DEBUG: Action::ENCRYPT = " << (int)Action::ENCRYPT << std::endl;
    std::cout << "DEBUG: Action::DECRYPT = " << (int)Action::DECRYPT << std::endl;
    
    ReadEnv env;
    std::string envKey =env.getenv();
    int key = std::stoi(envKey);
    std::cout<<"HELLO"<<std::endl;
    if(task.action == Action::ENCRYPT){
        char ch;
        while(task.f_stream.get(ch)){
            ch=(ch+key)%256;
            task.f_stream.seekp(-1, std::ios::cur);
            task.f_stream.put(ch);
        }
        task.f_stream.close();
    }
    else{
        char ch;
        while(task.f_stream.get(ch)){
            ch=(ch-key+256)%256; 
            task.f_stream.seekp(-1, std::ios::cur);
            task.f_stream.put(ch);
        }
        task.f_stream.close();
    }
    return 0;
}