#include "emu/Display.hpp"

#include <cstring>
#include <iostream>
#include <string>

using std::string;

void Display::drawBuffer(unsigned char gfx[64 * 32])
{
    std::cout << "==================================================================================================================================" << std::endl;

    string line = "";
    for(int y = 0; y < 32; y++)
    {
        line = "|";
        for (int x = 0; x < 64; x++)
        {
            int i = y*64 + x;
            if (gfx[i] == 1)
            {
                line += "**";
            }
            else
            {
                line += "  ";
            }
        }
        line += "|";
        std::cout << line << std::endl;
    }
    std::cout << "==================================================================================================================================" << std::endl;
    std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << std::endl;
}