#include "Logging.hpp"
#include "emu/Chip.hpp"

#include <iostream>

int main() {
    Chip chip;

    chip.init();
    chip.loadRom("IBM.ch8");
    chip.tick();
}