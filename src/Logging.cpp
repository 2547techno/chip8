#include "Logging.hpp"

#include <iostream>
#include <iomanip>

void log(string text)
{
    std::cout << text;
}

void log(unsigned char byte)
{
    std::cout << std::setw(2) << std::setfill('0') <<  std::hex << (int)byte;
}

void log(unsigned short twoByte)
{
    std::cout << std::setw(4) << std::setfill('0') << std::hex << (int)twoByte;
}