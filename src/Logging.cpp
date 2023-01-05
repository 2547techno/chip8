#include "Logging.hpp"

#include <iostream>

void log(string text)
{
    std::cout << text << std::endl;
}

void log(unsigned char byte)
{
    std::cout << std::hex << (int)byte << std::endl;
}