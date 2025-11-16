// Malubag, Villanueva
// 4 BS Computer Engineering

// ENGG 123.01
// Project 3: RISC-V Pipeline Simulator using C++

#include <iostream>
#include <algorithm>
#include <bitset>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <iomanip>
#include "Malubag-VIllanueva_Project-3_Functions.h"

using namespace std;

bool isValidHex(const string &s, int length)
{
  if(s.empty() || (int)s.length() > length) return false;
  for(int i=0; i<s.length(); i++)
  {
    char c = s[i];
    if(!isxdigit(c)) return false;
  }
  return true;
}

bool loadData(const string &address, const string &filename, 
              uint8_t * &data_memory, const int memory_size)
{
  unsigned long long addr = stoull(address, nullptr, 16);
  if(addr >= memory_size)
  {
    cout << "\nERROR: Address exceeds memory size." << endl;
    return false;
  }
  
  ifstream file(filename);
  if(!file.is_open())
  {
    cout << "\nERROR: " << filename << " not found." << endl;
    return false;
  }

  string line;  
  while(getline(file, line))
  {
    stringstream ss(line);
    string hexStr;

    if(!(ss >> hexStr)) 
    {
      cout << "\nERROR: File contains invalid data." << endl;
      return false;
    }
    if(hexStr.empty()) 
    {
      cout << "\nERROR: File is empty." << endl;
      return false;
    }
    if(!isValidHex(hexStr, 16)) 
    {
      cout << "\nERROR: File contains an invalid hex string length." 
           << endl;
      return false;
    }

    if(addr+8 > memory_size)
    {
      cout << "\nERROR: Not enough space to store data." << endl;
      break;
    }

    unsigned long long value = stoull(hexStr, nullptr, 16);
    for(int i=0; i<8; i++)
    {
      data_memory[addr + i] = (value >> (i * 8)) & 0xFF;
    }
    addr += 8;
  }

  file.close();

  return true;
}

bool loadCode(const string &address, const string &filename, 
              uint8_t * &instruction_memory, const int memory_size)
{
  unsigned long long addr = stoull(address, nullptr, 16);
  if(addr >= memory_size)
  {
    cout << "\nERROR: Address exceeds memory size." << endl;
    return false;
  }

  ifstream file(filename);
  if(!file.is_open())
  {
    cout << "\nERROR: " << filename << " not found." << endl;
    return false;
  }

  string line;  
  while(getline(file, line))
  {
    stringstream ss(line);
    string hexStr;

    if(!(ss >> hexStr)) 
    {
      cout << "\nERROR: File contains invalid data." << endl;
      return false;
    }
    if(hexStr.empty()) 
    {
      cout << "\nERROR: File is empty." << endl;
      return false;
    }
    if(!isValidHex(hexStr, 8)) 
    {
      cout << "\nERROR: File contains an invalid hex string length." 
           << endl;
      return false;
    }

    if(addr+4 > memory_size)
    {
      cout << "\nERROR: Not enough space to store data." << endl;
      break;
    }

    unsigned long long value = stoull(hexStr, nullptr, 16);
    for(int i=0; i<4; i++)
    {
      instruction_memory[addr + i] = (value >> (i * 8)) & 0xFF;
    }
    addr += 4;
  }

  file.close();

  return true;
}

void showData(string &address, int N, uint8_t * &data_memory,
              const int memory_size)
{
  unsigned long long addr = stoull(address, nullptr, 16);
  if(addr >= memory_size)
  {
    cout << "\nERROR: Address exceeds memory size." << endl;
    return;
  }

  for(int i=0; i<N; i++)
  {
    uint64_t val = 0;
    for(int j=0; j<8; j++)
    {
      val |= ((uint64_t)data_memory[addr + j]) << (j * 8);
    }
    
    cout << "0x" << hex << uppercase << setw(8) << setfill('0') 
         << addr << "\t";
    cout << setw(16) << setfill('0') << val << dec << endl;

    addr += 8;
  }
}

void showCode(string &address, int N, uint8_t * &instruction_memory,
              const int memory_size)
{
  unsigned long long addr = stoull(address, nullptr, 16);
  if(addr >= memory_size)
  {
    cout << "\nERROR: Address exceeds memory size." << endl;
    return;
  }

  for(int i=0; i<N; i++)
  {
    uint32_t val = 0;
    for(int j=0; j<4; j++)
    {
      val |= (uint32_t)instruction_memory[addr + j] << (j * 8);
    }
    
    cout << "0x" << hex << uppercase << setw(8) << setfill('0') 
         << addr << "\t";
    cout << setw(8) << setfill('0') << val << dec << endl;

    addr += 4;
  }
}

// PIPELINING STAGES

// fetches a single instruction from address in memory location 
// whose value is stored in program counter
// unsigned int instruction_fetch(int pc, uint8_t * &instruction_memory)
// {
//   unsigned int instruction = 0;
  
//   for(int i = 0; i < 4; i++)
//   {
//     instruction |= ((unsigned int)instruction_memory[pc + i]) 
//                 << (i * 8);
//   }

//   return instruction;
// }

// decodes instruction and register file is accessed to obtain values
// of registers used in instruction
void instruction_decode(unsigned int instruction, long long *&reg,
                        string &inst, long long &a, long long &b,
                        long long &c, int &rd)
{  
  unsigned int opcode = instruction & 0x7F;
  rd = (instruction >> 7) & 0x1F;

  // R Format
  unsigned int funct3 = (instruction >> 12) & 0x07;
  unsigned int rs1 = (instruction >> 15) & 0x1F;
  unsigned int rs2 = (instruction >> 20) & 0x1F;
  unsigned int funct7 = (instruction >> 25) & 0x7F;

  // I Format
  int immediate_i = (instruction >> 20) & 0xFFF;
  if (immediate_i & 0x800) immediate_i |= 0xFFFFF000;

  // S Format
  int imm_s = ((instruction >> 7) & 0x1F) | 
              (((instruction >> 25) & 0x7F) << 5);
  if (imm_s & 0x800) imm_s |= 0xFFFFF000;

  // U Format
  int imm_u = instruction & 0xFFFFF000;

  cout << "[DECODE] Instruction: 0x" << hex << instruction 
       << " opcode: 0x" << opcode << " funct3: 0x" << funct3 
       << " funct7: 0x" << funct7 << dec << endl;

  switch (opcode)
  {
  case 0b0110011: // R-type ADD/SUB/MUL
    if (funct3 == 0 && funct7 == 0x00)
    {
      inst = "ADD";
      a = reg[rs1];
      b = reg[rs2];
      c = reg[rd];
    }
    else if (funct3 == 0 && funct7 == 0x20)
    {
      inst = "SUB";
      a = reg[rs1];
      b = reg[rs2];
      c = reg[rd];
    }
    else if (funct3 == 0 && funct7 == 0x01)
    {
      inst = "MUL";
      a = reg[rs1];
      b = reg[rs2];
      c = reg[rd];
    }
    break;

  case 0b0010011: // I-type ADDI/SLLI
    if (funct3 == 0)
    {
      inst = "ADDI";
      a = reg[rs1];
      b = immediate_i;
      c = reg[rd];
    }
    else if (funct3 == 1 && funct7 == 0x00)
    {
      inst = "SLLI";
      a = reg[rs1];
      b = rs2;
      c = reg[rd];
    }
    break;

  case 0b0110111: // U-type LUI
    inst = "LUI";
    a = imm_u;
    b = 0;
    c = reg[rd];
    break;

  case 0b0000011: // I-type Load
    if (funct3 == 0x3) // LD
    {
      inst = "LD";
      a = reg[rs1];
      b = immediate_i;
      // c is set in memory_access
    }
    break;

  case 0b0100011: // S-type SD/SW
    if (funct3 == 0x3) // 64-bit store
    {
      inst = "SD";
      a = reg[rs1];
      b = imm_s;
      c = reg[rs2];
      rd = 0;
      cout << "[DECODE] SD instruction: rs1=x" << rs1 
           << " rs2=x" << rs2 << " imm=" << imm_s 
           << " base=" << a << " value=" << c << endl;
    }
    else if (funct3 == 0x2) // 32-bit store  
    {
      inst = "SW";
      a = reg[rs1];
      b = imm_s;
      c = reg[rs2];
      rd = 0;
      cout << "[DECODE] SW instruction: rs1=x" << rs1 
           << " rs2=x" << rs2 << " imm=" << imm_s 
           << " base=" << a << " value=" << c << endl;
    }
    break;

  default:
    inst = "UNKNOWN";
    cout << "[DECODE] Unknown instruction: opcode=0x" 
         << hex << opcode << dec << endl;
    break;
  }
  
  cout << "[DECODE] Decoded as: " << inst << endl;
}

// some activities are done such as ALU operations
void instruction_execute(string &inst, long long &a, long long &b,
                         long long &c, int &rd, int &pc_offset)
{
  if (inst == "ADD")
  {
    c = a + b;
    pc_offset = 4;
  }
  else if (inst == "SUB")
  {
    c = a - b;
    pc_offset = 4;
  }
  else if (inst == "ADDI")
  {
    c = a + b;
    pc_offset = 4;
  }
  else if (inst == "SLLI")
  {
    c = a << b;
    pc_offset = 4;
  }
  else if (inst == "LUI")
  {
    c = a;
    pc_offset = 4;
  }
  else if (inst == "MUL")
  {
    c = a * b;
    pc_offset = 4;
  }
  else if (inst == "LD" || inst == "SD")
  {
    // For load/store, the address calculation is done in 
    // memory_access
    // Just pass through to next stage
    pc_offset = 4;
  }
  else if (inst == "BLT")
  {
    if (a < b)
      pc_offset = c;
    else
      pc_offset = 4;
  }
  else if (inst == "BEQ")
  {
    if (a == b)
      pc_offset = c;
    else
      pc_offset = 4;
  }
  else
  {
    pc_offset = 4;
  }
}

// memory operands are read and written from/to the memory 
// that is present in the instruction
void memory_access(string &inst, long long &a, long long &b, 
                   long long &c, int &rd, uint8_t *&mem, 
                   const int memory_size)
{
  uint64_t address = a + b;
  
  cout << "[MEMORY_ACCESS] " << inst << " address=0x" << hex 
       << address << " value=" << dec << c << endl;
  
  if (inst == "LD") 
  {
    if (address + 8 > memory_size) 
    {
      cout << "ERROR: Memory access out of bounds." << endl;
      return;
    }
    c = 0;
    for (int i = 0; i < 8; i++) 
    {
      c |= ((uint64_t)mem[address + i]) << (i * 8);
    }
    cout << "[MEMORY_ACCESS] LD loaded value: " << c << endl;
  }
  else if (inst == "SD") 
  {
    if (address + 8 > memory_size) 
    {
      cout << "ERROR: Memory access out of bounds." << endl;
      return;
    }
    cout << "[MEMORY_ACCESS] SD storing value " << c 
         << " to address 0x" << hex << address << dec << endl;
    for (int i = 0; i < 8; i++) 
    {
      mem[address + i] = (c >> (i * 8)) & 0xFF;
    }
    cout << "[MEMORY_ACCESS] SD store completed" << endl;
  }  
  else if (inst == "SW") 
  {
    if (address + 4 > memory_size) 
    {
      cout << "ERROR: Memory access out of bounds." << endl;
      return;
    }
    cout << "[MEMORY_ACCESS] SW storing value " << c 
         << " to address 0x" << hex << address << dec << endl;
    for (int i = 0; i < 4; i++) 
    {
      mem[address + i] = (c >> (i * 8)) & 0xFF;
    }
    cout << "[MEMORY_ACCESS] SW store completed" << endl;
  }
}


// computed/fetched value is written back to the register 
// present in the instructions
void write_back(string &inst, long long &c, int &rd, 
                long long * &reg)
{
  if (rd != 0) // x0 is hardwired to 0
  {
    reg[rd] = c;
  }
}

// executes RISC-v instructions in a pipelined manner
int pipeline_loop(unsigned int instr, long long *&reg, 
                  uint8_t *&mem, const int memory_size)
{
  string inst = "";
  long long a = 0, b = 0, c = 0;
  int rd = 0;
  int pc_offset = 4;

  unsigned int fetched = instr;
  instruction_decode(fetched, reg, inst, a, b, c, rd);
  instruction_execute(inst, a, b, c, rd, pc_offset);
  memory_access(inst, a, b, c, rd, mem, memory_size);
  write_back(inst, c, rd, reg);

  cout << "[PIPELINE] " << inst << " executed. (rd=x" << rd 
       << ", result=" << c << ")" << endl;

  return pc_offset;
}