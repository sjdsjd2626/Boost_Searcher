#pragma once
#include<iostream>
#include<string>
#include<ctime>

#define NORMAL 1
#define WARNING 2
#define DEBUG 3
#define FATAL 4

#define LOG(LEVEL,MESSAGE) log(#LEVEL,MESSAGE,__FILE__,__LINE__)//带#表示要用字符串
void log(const std::string level,const std::string message,const std::string file,const int line)
{
    std::cout<<"["<<level<<"]"<<"["<<time(nullptr)<<"]"<<"["<<message<<"]"<<"["<<file<<" : "<<line<<"]"<<std::endl;
}
