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
  
  for(int i=0; i<memory_size; i++) 
  {
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
                   << filename << " to address 0x" << hex 
                   << uppercase << setw(8) << setfill('0') 
                   << addr_num << dec << endl;
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

        // instruction_fetch() is done here
        // fetches a single instruction from address in 
        // memory location whose value is stored in program counter
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

        int pcOffset = pipeline_loop(instr, registers, 
                                     data_memory, memory_size);

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