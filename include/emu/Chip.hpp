#pragma once

#include <string>

using std::string;

class Chip
{
public:
    enum Flag
    {
        DRAW
    };

    void init();
    void loadRom(string file);
    void tick();
    unsigned char getFlag(Flag flag);


private:
    unsigned short opcode;
    unsigned char memory[4096];
    /*
    0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
    0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
    0x200-0xFFF - Program ROM and work RAM
    */

    unsigned char V[16];    // registers
    unsigned short I;       // index register
    unsigned short pc;      // program counter

    unsigned char gfx[64 * 32]; // graphics buffer

    // timers (60Hz)
    unsigned char delayTimer;
    unsigned char soundTimer;

    // stack
    unsigned short stack[16];
    unsigned short sp;          // stack pointer

    // keyboard
    unsigned char key[16];

    // fontset
    const static unsigned char chip8_fontset[];

    void setFlag(Flag flag, unsigned char value);
    void enableDrawFlag();
    void clearDrawFlag();
};