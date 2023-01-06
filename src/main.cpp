#include "Logging.hpp"
#include "emu/Chip.hpp"
#include "emu/Display.hpp"

#include <iostream>

int main() {
    Chip chip;
    Display display;

    chip.init();
    chip.loadRom("IBM.ch8");
    while(true)
    // for(int i = 0; i < 0x18; i++)
    {
        chip.tick();
        if (chip.drawFlag)
        {
            display.drawBuffer(chip.getGfx());
            chip.drawFlag = false;
        }
    }
}