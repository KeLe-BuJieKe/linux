#pragma once 

#include <iostream>
#include <string>
#include <ctime>

#define INFO    1
#define WARNING 2
#define ERROR   3
#define FATAL   4


#define LOG(level, message) Log(#level, message, __FILE__, __LINE__)


void Log(std::string level, std::string message, std::string filename, size_t line)
{
    std::cout << "[" << level << "][" << time(nullptr) << "][" << message << "][" << filename << "][" << line << "]" << std::endl;
}

