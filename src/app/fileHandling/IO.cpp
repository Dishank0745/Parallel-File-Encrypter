#include<iostream>
#include "IO.hpp"
#include <fstream>

IO::IO(const std:: string &file_path){
    fileStream.open(file_path,std::ios::in | std::ios::out | std::ios::binary);
    if (!fileStream.is_open()) {
        std::cerr << "Error opening file: " << file_path << std::endl;
    }
}

std::fstream IO::getFileStream(){
    return std::move(fileStream);
} 

IO::~IO(){
    if (fileStream.is_open()) {
        fileStream.close();
    }
}