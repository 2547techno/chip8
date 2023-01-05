#include "emu/Chip.hpp"
#include "Logging.hpp"

#include <cstring>

void Chip::init()
{
    this->pc = 0x200;
    this->opcode = 0;
    this->I = 0;
    this->sp = 0;

    //clear graphics buffer
    std::memset(this->gfx, 0, sizeof(this->gfx));
    //clear stack
    std::memset(this->stack, 0, sizeof(this->stack));
    //clear registers
    std::memset(this->V, 0, sizeof(this->V));
    //clear memory
    std::memset(this->memory, 0, sizeof(this->memory));

    //load fontset
    for(int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];	

    this-> delayTimer = 0;
    this-> soundTimer = 0;

    log("Chip initialized");
}

void Chip::loadRom(string file)
{
    
}

void Chip::tick()
{

}