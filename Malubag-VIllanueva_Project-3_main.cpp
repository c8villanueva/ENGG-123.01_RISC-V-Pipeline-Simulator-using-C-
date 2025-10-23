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
    
    cout << hex << uppercase << "0x" << setw(8) << setfill('0') 
         << addr << "\t";
    cout << hex << uppercase << setw(16) << setfill('0') << val 
         << dec << setfill(' ') << endl;

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
    uint64_t val = 0;
    for(int j=0; j<4; j++)
    {
      val |= (uint32_t)instruction_memory[addr + j] << (j * 8);
    }
    
    cout << hex << uppercase << "0x" << setw(8) << setfill('0') 
         << addr << "\t";
    cout << hex << uppercase << setw(8) << setfill('0') << val 
         << dec << setfill(' ') << endl;

    addr += 4;
  }
}

int execInstruction(unsigned int instruction, long long * &reg, 
                     uint8_t * &mem)
{
  cout << "\nInstruction: " << bitset<32>(instruction) << "\n\n";

  unsigned int opcode =  instruction & 0x7F;
  unsigned int rd     = (instruction >> 7)  & 0x1F;

  // R Format
  unsigned int funct3 = (instruction >> 12) & 0x07;
  unsigned int rs1    = (instruction >> 15) & 0x1F;
  unsigned int rs2    = (instruction >> 20) & 0x1F;
  unsigned int funct7 = (instruction >> 25) & 0x7F;

  // I Format
  int immediate = (instruction >> 20) & 0xFFF;
  if (immediate & 0x800) immediate |= 0xFFFFF000;

  // S Format
  int imm_s = ((instruction >> 7) & 0x1F) | 
              (((instruction >> 25) & 0x7F) << 5);
  if (imm_s & 0x800) imm_s |= 0xFFFFF000;

  // B Format
  int imm_b = (((instruction >> 8) & 0x0F)  << 1)  | 
              (((instruction >> 25) & 0x3F) << 5)  | 
              (((instruction >> 7) & 0x01)  << 11) | 
              (((instruction >> 31) & 0x01) << 12);
  if (imm_b & 0x1000) imm_b |= 0xFFFFE000;

  vector<unsigned int> usedRegs;
  int pcOffset = 4;

  switch (opcode) {
    case 0b0110011: // R-type ADD/SUB

      if (funct3 == 0 && funct7 == 0x00) 
      { // ADD
        if (rd == 0) 
        {
          cout << "ERROR: Cannot write to x0 (rd = 0)." 
                << endl;
        } 
        else 
        {
          reg[rd] = reg[rs1] + reg[rs2];
          cout << "add x" << rd << ", x" << rs1 << ", x" 
                << rs2 << endl;
          usedRegs = {rd, rs1, rs2};
        }
      }
      if (funct3 == 0 && funct7 == 0x20) 
      { // SUB
        if (rd == 0) 
        {
          cout << "ERROR: Cannot write to x0 (rd = 0)." << endl;
        } 
        else 
        {
          reg[rd] = reg[rs1] - reg[rs2];
          cout << "sub x" << rd << ", x" << rs1 << ", x" << rs2 
                << endl;
          usedRegs = {rd, rs1, rs2};
        }
      }
      break;

    case 0b0010011: // I-type
      if (funct3 == 0) 
      { //ADDI
        if (rd == 0) 
        {
          if (!(opcode == 0b0010011 
              && funct3 == 0 
              && rs1 == 0 
              && immediate == 0)) 
          {
            cout << "ERROR: Cannot write to x0 (rd = 0)." << endl;
          } 
          else 
          {
            cout << "NOP (addi x0, x0, 0)" << endl;
          }

        } 
        else 
        {
          reg[rd] = reg[rs1] + immediate;
          cout << "addi x" << rd << ", x" << rs1 << ", " 
                << immediate << endl;
          usedRegs = {rd, rs1};
        }
      }
      if (funct3 == 1 && funct7 == 0)
      { // SLLI
        unsigned int shamt = rs2;
        if (rd == 0) 
        {
          cout << "ERROR: Cannot write to x0 (rd = 0)." << endl;
        } 
        else 
        {
          reg[rd] = reg[rs1] << shamt;
          cout << "slli x" << rd << ", x" << rs1 << ", " 
                << shamt << endl;
          usedRegs = {rd, rs1};
        }
      }
      break;

    case 0b0000011: // I-type LD
      if (funct3 == 3) 
      {
        if (rd == 0) 
        {
          cout << "ERROR: Cannot write to x0 (rd = 0)." << endl;
        } 
        else 
        {
          uint64_t mem_addr = reg[rs1] + immediate;
          uint64_t val = 0;
          for (int i = 0; i < 8; i++) 
          {
            val |= ((uint64_t)mem[mem_addr + i]) << (i * 8);
          }
          reg[rd] = val;
          cout << "ld x" << rd << ", " << immediate 
                << "(x" << rs1 << ")" << endl;
          usedRegs = {rd, rs1};

          cout << "[DEBUG] Stored value " << hex << reg[rs2] 
               << " at address 0x" << mem_addr 
               << endl;

        }
      }
      break;

    case 0b0100011: // S-type SD
      if (funct3 == 3) 
      {
        if (rs2 == 0) 
        {
          cout << "\nERROR: Cannot store from x0 (rs2 = 0)." 
                << endl; 
        } 
        else 
        {
          uint64_t mem_addr = reg[rs1] + imm_s;
          uint64_t val = reg[rs2];
          for (int i = 0; i < 8; i++) 
          {
            mem[mem_addr + i] = (val >> (i * 8)) & 0xFF;
          }
          cout << "sd x" << rs2 << ", " << imm_s 
                << "(x" << rs1 << ")" << endl;
          usedRegs = {rs1, rs2};

          cout << "[DEBUG] mem_addr = 0x" << hex << mem_addr << dec << endl;
        }
      }
      break;

     case 0b1100011: // B-type Branch instructions
      if (funct3 == 0x1) 
        { // BLT 
          if ((long long)reg[rs1] < (long long)reg[rs2]) 
          {
            cout << "blt x" << rs1 << ", x" << rs2 << ", " 
                 << imm_b << " (branch taken)" << endl;
            pcOffset = imm_b;
          } 
          else 
          {
            cout << "blt x" << rs1 << ", x" << rs2 << ", " 
                 << imm_b << " (branch not taken)" << endl;
          }
          usedRegs = {rs1, rs2};
        }
        else if (funct3 == 0x0)
        { // BEQ
          if (reg[rs1] == reg[rs2]) 
          {
            cout << "beq x" << rs1 << ", x" << rs2 << ", " 
                 << imm_b << " (branch taken)" << endl;
            pcOffset = imm_b;
          } 
          else 
          {
            cout << "beq x" << rs1 << ", x" << rs2 << ", " 
                 << imm_b << " (branch not taken)" << endl;
          }
          usedRegs = {rs1, rs2};
        }
        else if (funct3 == 0x5)
        { // BGE
          if ((long long)reg[rs1] >= (long long)reg[rs2]) 
          {
            cout << "bge x" << rs1 << ", x" << rs2 << ", " 
                 << imm_b << " (branch taken)" << endl;
            pcOffset = imm_b;
          } 
          else 
          {
            cout << "bge x" << rs1 << ", x" << rs2 << ", " 
                 << imm_b << " (branch not taken)" << endl;
          }
          usedRegs = {rs1, rs2};
        }
      break;

    default:
      cout << "Unsupported instruction.\n"
            << "opcode = "   << opcode
            << "\tfunct3 = " << funct3
            << "\tfunct7 = " << funct7 << endl;
  }

  if (!usedRegs.empty()) 
  {
    cout << "\n[Register Dump]\n";
    for (int r : usedRegs) {
      if (r == 0) continue;
      cout << "x" << setw(2) << left << r
           << " = " << dec << reg[r]
           << "\t(0x" << hex << uppercase << reg[r] << ")\n";
    }
    cout << dec;
  }

  return pcOffset;
}

int main()
{
  cout << "\nInteractive Basic RISC-V Simulator"
       << "\n(Type \"HELP\" to display all commands.)"
       << "\n(Type \"EXIT\" to terminate the program.)" << endl;

  stringstream ss;
  string userInput = "",
         command = "", 
         address = "", 
         filename = "";
  int N = 0;
  ifstream file;

  // 32 64-bit registers
  long long *registers = new long long [32];
  for(int i=0; i<32; i++) registers[i] = 0;

  // memory
  const int memory_size = 1024 * 64; // 64 KB
  uint8_t *data_memory = new uint8_t [memory_size]; 
  uint8_t *instruction_memory = new uint8_t [memory_size];
  
  while(true)
  {
    ss.clear();
    cout << "\nInput instructions here:\n> ";
    getline(cin, userInput);
    transform(userInput.begin(), userInput.end(), userInput.begin(), 
              [](unsigned char c) {return toupper(c); });

    ss.str(userInput);
    ss >> command;

    if(command == "EXIT")
    {
      ss.clear();
      file.close();
      delete [] registers;
      delete [] data_memory;
      delete [] instruction_memory;

      cout << "\nProgram has been terminated.\n" << endl;
      return 0;
    }

    else if(command == "HELP")
    {
      cout << "\n1. loaddata <address> <filename> - obtains 64-bit"
           << " data from <filename> and stores to <address>"
           << "\n2. showdata <address> <N>        - displays <N>"
           << " data starting from <address>"
           << "\n3. loadcode <address> <filename> - obtains 32-bit"
           << " instructions from <filename> and stores to <address>"
           << "\n4. showcode <address> <N>        - displays <N>"
           << " instructions starting from <address>"
           << "\n5. exec <address>                - simulates"
           << " execution of codes starting from <address>"
           << "\n6. help                          - displays this"
           << " message"
           << "\n7. exit                          - terminates the"
           << " program"
           << endl;
    }

    else if(command == "LOADDATA" || command == "LOADCODE")
    {
      ss >> address >> filename;
      if(address.empty() || filename.empty())
      {
        cout << "\nERROR: Missing arguments. ";
        cout << "Type \"HELP\" to display all commands.\n";
        continue;
      }
      else
      {
        if(!isValidHex(address, 8))
        {
          cout << endl
          << "ERROR: Please input an 8-bit hex value for <address>."
          << endl;
          continue;
        }
        else
        {
          if(command == "LOADDATA") 
          {
            if (!loadData(address, filename, 
                          data_memory, memory_size))
              cout << "\nERROR: Failed to load data from " 
                   << filename << endl;
            else
            {
              cout << "\nData loaded successfully from " << filename 
                   << " to address " << hex << uppercase 
                   << "0x" << setw(8) << setfill('0') << address 
                   << endl;
            }
          }
          else if(command == "LOADCODE") 
          {
            if (!loadCode(address, filename, 
                          instruction_memory, memory_size))
              cout << "\nERROR: Failed to load code from " 
                   << filename << endl;
            else
            {
              cout << "\nInstructions loaded successfully from " 
                   << filename << " to address " << hex << uppercase 
                   << "0x" << setw(8) << setfill('0') 
                   << address << endl;
            }
          }
        }
      }
    }

    else if(command == "SHOWDATA" || command == "SHOWCODE")
    {
      ss >> address >> N;
      if(address.empty() || N <= 0)
      {
        cout << endl 
             << "ERROR: Missing arguments." 
             << " Type \"HELP\" to display all commands.\n";
        continue;
      }
      else
      {
        if(!isValidHex(address,8))
        {
          cout << endl 
               <<"ERROR: Please input an 8-bit hex value" 
               << " for <address>." << endl;
          continue;
        }
        else
        {
          cout << "\nShowing " << N << " "
               << (command == "SHOWDATA" ? "data" : "instructions") 
               << " from address " << hex << uppercase << "0x" 
               << setw(8) << setfill('0') << address << endl;

          if(command == "SHOWDATA") 
          {
            showData(address, N, data_memory, memory_size);
          }
          else if(command == "SHOWCODE") 
          {
            showCode(address, N, instruction_memory, memory_size);
          }
        }
      }      
    }

    else if(command == "EXEC")
    {
      ss >> address;
      if (address.empty()) 
      {
        cout << endl 
             << "ERROR: Missing <address> argument." 
             << " Usage: EXEC <address>\n";
        continue;
      }
      if (!isValidHex(address, 8)) 
      {
        cout << "\nERROR: Invalid address format.\n";
        continue;
      }

      unsigned long long addr = stoull(address, nullptr, 16);
      unsigned long long PC = addr;

      cout << "\n[Starting execution at 0x" << hex << uppercase 
           << setw(8) << setfill('0') << PC << "]\n";

      while (true)
      {
        if (PC + 4 >= memory_size)
        {
          cout << endl 
               << "[ERROR] Program counter out of bounds." 
               << " Halting execution.\n";
          break;
        }

        unsigned int instr = 0;
        for (int j = 0; j < 4; j++)
        {
          instr |= ((unsigned int)instruction_memory[PC + j]) 
                << (j * 8);
        }
        if (instr == 0x00000000)
        {
          break;
        }

        cout << "\nExecuting instruction at 0x" << hex << uppercase 
             << setw(8) << setfill('0') << PC << "...\n";

        int pcOffset = execInstruction(instr, registers, 
                                       data_memory);

        PC += pcOffset; 
      }

      cout << "\nExecution finished\n";
      }

    else
    {
      cout << "\nERROR: Invalid command."
           << " Type HELP to display all commands.\n";
    }
  }
}
