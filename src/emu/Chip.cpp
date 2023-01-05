#include "emu/Chip.hpp"
#include "Logging.hpp"

#include <cstring>
#include <stdio.h>

const unsigned char Chip::chip8_fontset[80] =
{ 
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

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
        this->memory[i+0x050] = chip8_fontset[i];	

    this-> delayTimer = 0;
    this-> soundTimer = 0;

    log("Chip initialized");
}

void Chip::loadRom(string file)
{
    char path[128];
    strcpy(path, "roms/");
    strcat(path, file.c_str());

    FILE* fp = fopen(path, "r");
    if (fp == NULL) perror("Error opening file");
    fseek(fp, 0, SEEK_END);
    long int size = ftell(fp);
    fclose(fp);

    fp = fopen(path, "r");
    unsigned char * buffer = (unsigned char *) malloc(size);
    fread(buffer, sizeof(unsigned char), size, fp);
    fclose(fp);

    for (int i = 0; i < size; i++)
    {
        this->memory[i+0x200] = buffer[i];
        // log(buffer[i]);
    }

    log("ROM loaded");
}

void Chip::tick()
{

}