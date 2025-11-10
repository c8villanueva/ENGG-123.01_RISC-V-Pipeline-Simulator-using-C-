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

// TBD: MOVED TO INSTRUCTION_DECODE & _EXECUTE
int execInstruction(unsigned int instruction, long long * &reg, 
                     uint8_t * &mem)
{
  // check p2_code_dump.txt
}

// PIPELINING STAGES (info from geeksforgeeks)

// TBD: TO IMPROVE...
// [] make string isnt, long long a, b, c and int rd as struct/class

// fetches a single instruction from address in memory location whose value is stored in program counter
// TBD: TO ADD VALIDATION FOR OUT OF BOUNDS
unsigned int instruction_fetch(int pc, uint8_t * &instruction_memory)
{
  unsigned int instruction = 0;
  
  for(int i = 0; i < 4; i++)
  {
    instruction |= ((unsigned int)instruction_memory[pc + i]) << (i * 8);
  }

  return instruction;
}

// decodes instruction and register file is accessed to obtain values of registers used in instruction
// TBD: TO FIX
void instruction_decode(unsigned int instruction, long long * &reg, 
                        string inst, long long a, long long b, long long c)
{
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

  switch (opcode) {
    case 0b0110011: // R-type ADD/SUB
      if (funct3 == 0 && funct7 == 0x00) 
        if (rd == 0) 
        {
          cout << "ERROR: Cannot write to x0 (rd = 0)." << endl;
        } 
        else 
        {
          inst = "ADD";
          a = reg[rs1];
          b = reg[rs2];
          c = reg[rd];
        }
      }
      if (funct3 == 0 && funct7 == 0x20) 
        if (rd == 0) 
        {
          cout << "ERROR: Cannot write to x0 (rd = 0)." << endl;
        } 
        else 
        {
          inst = "SUB";
          a = reg[rs1];
          b = reg[rs2];
          c = reg[rd];
        }
      }
      break;

    case 0b0010011: // I-type
      if (funct3 == 0) 
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
          inst = "ADDI";
          a = reg[rs1];
          b = immediate;
          c = reg[rd];
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
          inst = "SLLI";
          a = reg[rs1];
          b = shamt;
          c = reg[rd];
        }
      }
      break;

    case 0b1100011: // B-type Branch instructions
      if (funct3 == 0x1) 
        { // BLT 
          inst = "BLT";
          a = reg[rs1];
          b = reg[rs2];
          c = imm_b;
        }
        else if (funct3 == 0x0)
        { // BEQ
          inst = "BEQ";
          a = reg[rs1];
          b = reg[rs2];
          c = imm_b;
        }
      break;

    default: 
      inst = "UNKNOWN";
      // a = 0;
      // b = 0;
      // c = 0;
      // rd = 0;
  }
}

// some activities are done such as ALU operations
// TBD: TO ADD VALIDATION IF NEEDED
void instruction_execute(string &inst, long long &a, long long &b, long long &c, int &rd)
{
  if (inst == "ADD") 
  {
    c = a + b;
    pc_offset += 4;
  }
  else if (inst == "SUB") 
  {
    c = a - b;
    pc_offset += 4;
  }
  else if (inst == "ADDI") 
  {
    c = a + b;
    pc_offset += 4;
  }
  else if (inst == "SLLI")
  {
    c = a << b;
    pc_offset += 4;
  }
  else if (inst == "BLT") 
  {
    if (a < b) 
    {
      pc_offset = c;
    } 
  }
  else if (inst == "BEQ")
  {
    if (a == b) 
    {
      pc_offset = c;
    } 
  }
}

// memory operands are read and written from/to the memory that is present in the instruction
void memory_access(string &inst, long long &a, long long &b, long long &c, int &rd, uint8_t * &mem)
{
  if (inst == "LD") 
  {
    uint64_t address = a + b;
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
  }
  else if (inst == "SD") 
  {
    uint64_t address = a + b;
    if (address + 8 > memory_size) 
    {
      cout << "ERROR: Memory access out of bounds." << endl;
      return;
    }
    for (int i = 0; i < 8; i++) 
    {
      mem[address + i] = (c >> (i * 8)) & 0xFF;
    }
  }  
}

// computed/fetched value is written back to the register present in the instructions
void write_back(string &inst, long long &c, int &rd, long long * &reg)
{
  if (rd != 0) // x0 is hardwired to 0
  {
    reg[rd] = c;
  }
}

// executes RISC-v instructions in a pipelined manner
void pipeline_loop(string &inst, long long &a, long long &b, long long &c, int &rd, long long * &reg)
{
  while(true)
  {
    write_back();
    memory_access();
    instruction_execute();
    instruction_decode();
    instruction_fetch();
  }
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
  
  for(int i=0; i<memory_size; i++) {
    data_memory[i] = 0;
    instruction_memory[i] = 0;
  }
  
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
          // Convert address string to number for display
          unsigned long long addr_num = stoull(address, nullptr, 16);
          
          if(command == "LOADDATA") 
          {
            if (!loadData(address, filename, 
                          data_memory, memory_size))
              cout << "\nERROR: Failed to load data from " 
                   << filename << endl;
            else
            {
              cout << "\nData loaded successfully from " << filename 
                   << " to address 0x" << hex << uppercase 
                   << setw(8) << setfill('0') << addr_num 
                   << dec << endl;
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
                   << filename << " to address 0x" << hex << uppercase 
                   << setw(8) << setfill('0') << addr_num 
                   << dec << endl;
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
          unsigned long long addr_num = stoull(address, nullptr, 16);
          cout << "\nShowing " << dec << N << " "
               << (command == "SHOWDATA" ? "data" : "instructions") 
               << " from address 0x" << hex << uppercase 
               << setw(8) << setfill('0') << addr_num << dec << endl;

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
           << setw(8) << setfill('0') << PC << dec << "]\n";

      while (true)
      {
        if (PC + 4 > memory_size)
        {
          cout << "\n[ERROR] Program counter out of bounds." 
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
             << setw(8) << setfill('0') << PC << dec << "...\n";

        // TBD: was previously exec_instruction
        int pcOffset = pipeline_loop(instr, registers, data_memory);

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