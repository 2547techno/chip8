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
    }
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

unsigned char* Chip::getGfx()
{
    return this->gfx;
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
    case 0x2000: // 2NNN | call subroutine at address NNN
        {
            unsigned short address = op & 0x0FFF;
            this->stack[this->sp] = this->pc;
            this->sp++;
            this->pc = address;
            break;
        }
    case 0x3000: // 3XNN | skip next instruction if Vx == NN
        {
            unsigned char n = op & 0x00FF;
            unsigned short x = (op & 0x0F00) >> 8;
            if (n == x) {
                this->incrPC(4);
            } else {
                this->incrPC();
            }
            break;
        }
    case 0x4000: // 4XNN | skip next instruction if Vx != NN
        {
            unsigned char n = op & 0x00FF;
            unsigned short x = (op & 0x0F00) >> 8;
            if (n != x) {
                this->incrPC(4);
            } else {
                this->incrPC();
            }
            break;
        }
    case 0x5000: // 5XY0 | skip next instruction if Vx = Vy
        {
            unsigned short x = (op & 0x0F00) >> 8;
            unsigned short y = (op & 0x00F0) >> 4;
            if (x == y) {
                this->incrPC(4);
            } else {
                this->incrPC();
            }
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
    case 0x8000:
        {
            switch (op & 0x000F)
            {
            case 0x0: // 8XY0 | Vx := Vy
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    unsigned short y = (op & 0x00F0) >> 4;
                    this->V[x] = this->V[y];
                    break;
                }
            case 0x1: // 8XY1 | Vx := Vx | Vy
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    unsigned short y = (op & 0x00F0) >> 4;
                    this->V[x] = this->V[x] | this->V[y];
                    break;
                }
            case 0x2: // 8XY2 | Vx := Vx & Vy
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    unsigned short y = (op & 0x00F0) >> 4;
                    this->V[x] = this->V[x] & this->V[y];
                    break;
                }
            case 0x3: // 8XY3 | Vx := Vx ^ Vy
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    unsigned short y = (op & 0x00F0) >> 4;
                    this->V[x] = this->V[x] ^ this->V[y];
                    break;
                }
            case 0x4: // 8XY4 | Vx := Vx + Vy
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    unsigned short y = (op & 0x00F0) >> 4;
                    this->V[x] += this->V[y];
                    if (this->V[x] > 0xFF)
                    {
                        this->V[0xF] = 1;
                    }
                    else
                    {
                        this->V[0xF] = 0;
                    }
                    break;
                }
            case 0x5: // 8XY5 | Vx := Vx - Vy
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    unsigned short y = (op & 0x00F0) >> 4;
                    this->V[x] -= this->V[y];
                    if (this->V[x] < 0)
                    {
                        this->V[0xF] = 1;
                    }
                    else
                    {
                        this->V[0xF] = 0;
                    }
                    break;
                }
            case 0x6: // 8XY6 | stores the least significant bit of Vx in Vf and then shifts Vx to the right by 1
                {
                    // unsigned short x = (op & 0x0F00) >> 8;
                    // this->V[0xF] = x & 0x000F;
                    // this->V[x] = this->V[x] >> 4;
                    break;
                }
            case 0x7: // 8XY7 | Vx := Vy - Vx
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    unsigned short y = (op & 0x00F0) >> 4;
                    this->V[x] = this->V[y] - this->V[x];
                    if (this->V[x] < 0)
                    {
                        this->V[0xF] = 1;
                    }
                    else
                    {
                        this->V[0xF] = 0;
                    }
                    break;
                }
            case 0xE: // 8XYE | stores the most significant bit of Vx in Vf and then shifts Vx to the left by 1
                {
                    // unsigned short x = (op & 0xF000) >> 12;
                    // this->V[0xF] = x & 0x000F;
                    // this->V[x] = this->V[x] >> 4;
                    break;
                }
            default:
                {
                    unhandled = true;
                    break;
                }
            }
            break;
        }
    case 0x9000: // 9XY0 | skip next instruction if Vx != Vy
        {
            unsigned short x = (op & 0x0F00) >> 8;
            unsigned short y = (op & 0x00F0) >> 4;
            if (x != y)
            {
                this->incrPC(4);
            }
            else
            {
                this->incrPC();
            }
            break;
        }
    case 0xA000: // ANNN | set I to address NNN
        {
            unsigned short address = op & 0x0FFF;
            this->I = address;
            this->incrPC();
            break;
        }
    case 0xB000: // BNNN | jump to addess NNN + V0
        {
            unsigned short address = op & 0x0FFF;
            this->pc = this->V[0] + address;
            break;
        }
    case 0xC000: // CXNN | Vx := rand() & NN
        {
            unsigned short x = (op & 0x0F00) >> 8;
            unsigned char n = op & 0x00FF;
            this->V[x] = n; // TODO: rand
            break;
        }
    case 0xD000: // DXYN | draw sprite at (Vx, Vy), width 8, height N, rows string from mem location I
        {
            this->drawSprite();
            this->incrPC();
            break;
        }
    case 0xE000:
        {
            switch (op & 0x000F)
            {
            case 0xE: // EX9E | skip next instruction key stored in Vx is pressed
                {
                    
                    break;
                }
            case 0x1: // EXA1 | skip next instruction key stored in Vx is not pressed
                {

                    break;
                }
            
            default:
                {
                    unhandled = true;
                    break;
                }
            }
            break;
        }
    case 0xF000:
        {
            switch (op & 0x00FF)
            {
            case 0x07: // FX07 | Vx := delayTimer
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    this->V[x] = this->delayTimer;
                    break;
                }            
            case 0x0A: // FX0A | Vx := getKey() (program is halted until keypress)
                {

                    break;
                }            
            case 0x15: // FX15 | delayTimer := Vx
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    this->delayTimer = this->V[x];
                    break;
                }            
            case 0x18: // FX18 | soundTimer := Vx
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    this->soundTimer = this->V[x];
                    break;
                }            
            case 0x1E: // FX1E | I := I + Vx
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    this->I += this->V[x];
                    break;
                }            
            case 0x29: // FX29 | sets I to the location of the sprite for the character in Vx
                {
                    unsigned short x = (op & 0x0F00) >> 8;
                    this->I = 0x200 + 5*this->V[x];
                    break;
                }            
            case 0x33: // FX33 | stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2
                {

                    break;
                }            
            case 0x55: // FX55 | stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
                {

                    break;
                }            
            case 0x65: // FX65 | fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified.
                {

                    break;
                }            
            default:
                {
                    unhandled = true;
                    break;
                }
            }
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