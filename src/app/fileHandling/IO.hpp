#ifndef IO_HPP
#define IO_HPP

#include <string>
#include <fstream>


class IO{
    public:
        IO(const std:: string &file_path);
        ~IO();
        std::fstream getFileStream();

    private:
    std::fstream fileStream;
};

#endif