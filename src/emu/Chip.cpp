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

    this->drawFlag = false;

    log("Chip initialized\n");
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
        // log("\n");
    }
    this->memEnd = size + 0x200;
    // log(memEnd);
    log("\nROM loaded\n");
}

void Chip::tick()
{
    this->opcode = this->memory[this->pc] << 8 | this->memory[this->pc+1]; //2 bytes
    // log("opcode: ");
    // log(this->opcode);
    // log("\n");

    this->execOp(this->opcode);

    if (this->delayTimer > 0) this->delayTimer--;
    if (this->soundTimer > 0) 
    {
        if (this->soundTimer == 1)
        {
            log("[sound timer] play sound\n");
        }
    }

}

void Chip::incrPC(int value)
{
    this->pc += value;
}

void Chip::setFlag(Flag flag, unsigned char value)
{
    switch (flag)
    {
    case Flag::DRAW:
        this->memory[15] = value;
        break;
    }
}

void Chip::enableDrawFlag()
{
    this->setFlag(Flag::DRAW, 1);
}

void Chip::clearDrawFlag()
{
    this->setFlag(Flag::DRAW, 0);
}

unsigned short Chip::getPC()
{
    return this->pc;
}

unsigned short Chip::getMemEnd()
{
    return this->memEnd;
}

unsigned char* Chip::getGfx()
{
    return this->gfx;
}

unsigned char Chip::getFlag(Flag flag)
{
    switch (flag)
    {
    case Flag::DRAW:
        return this->memory[15];
        break;

    default:
        return 0;
    }
}

void Chip::clearDisplay()
{
    std::memset(this->gfx, 0, sizeof(this->gfx));
    // techinically i should be checking if clearing changes anything before setting draw flag
    this->drawFlag = true;
    // this->enableDrawFlag();
}

void Chip::drawSprite()
{
    unsigned short x = V[(opcode & 0x0F00) >> 8];
    unsigned short y = V[(opcode & 0x00F0) >> 4];
    unsigned short height = opcode & 0x000F;
    unsigned short pixel;

    V[0xF] = 0;
    for (int yline = 0; yline < height; yline++)
    {
        pixel = memory[I + yline];
        for(int xline = 0; xline < 8; xline++)
        {
        if((pixel & (0x80 >> xline)) != 0)
        {
            if(gfx[(x + xline + ((y + yline) * 64))] == 1)
            V[0xF] = 1;                                 
            gfx[x + xline + ((y + yline) * 64)] ^= 1;
        }
        }
    }

    drawFlag = true;
}

void Chip::execOp(unsigned short op)
{
    bool unhandled = false;
    switch (op & 0xF000)
    {
    case 0x0000:
        {

            switch (op & 0x000F)
            {
            case 0x0: // 00E0 | clear display
                this->clearDisplay();
                this->incrPC();
                break;
            case 0xE: // 00EE | return from subroutine
                /* code */
                break;
            
            default:
                unhandled = true;
                break;
            }
            break;
        }
    case 0x1000: // 1NNN | jump to address NNN
        {
            unsigned short address = op & 0x0FFF;
            this->pc = address;
            break;
        }

    case 0x6000: // 6XNN | set Vx to NN
        {
            unsigned char value = op & 0x00FF;
            unsigned short x = (op & 0x0F00) >> 8;
            this->V[x] = value;
            this->incrPC();
            break;
        }

    case 0x7000: // 7XNN | add NN to Vx (carry flag not changed)
        {
            unsigned char value = op & 0x00FF;
            unsigned short x = (op & 0x0F00) >> 8;
            this->V[x] += value;
            this->incrPC();
            break;
        }

    case 0xA000: // ANNN | set I to address NNN
        {
            unsigned short address = op & 0x0FFF;
            this->I = address;
            this->incrPC();
            break;
        }

    case 0xD000: // DXYN | draw sprite at (Vx, Vy), width 8, height N, rows string from mem location I
        {
            /*
            unsigned short x = V[(op & 0x0F00) >> 8];
            unsigned short y = V[(op & 0x00F0) >> 4];
            unsigned short n = op & 0x000F;
            unsigned short spriteRow;

            V[0xF] = 0; // reset collision check
            for(int row = 0; row < n; row++)
            {
                spriteRow = this->memory[this->I + row];
                for(int pixelIndex = 0; pixelIndex < 8; pixelIndex++)
                {
                    int screenX = x + pixelIndex;
                    int screenY = row;



                }
            }

            this->incrPC();
            */
            this->drawSprite();
            this->incrPC();
            break;
        }
    
    default:
        {
            unhandled = true;
            break;
        }
    }

    if (unhandled)
    {
        log("unhandled op code: ");
        log(op);
        log("\n");
        this->incrPC();
    }
}