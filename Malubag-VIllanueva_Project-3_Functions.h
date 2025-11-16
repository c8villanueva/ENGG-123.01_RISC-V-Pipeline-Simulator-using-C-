// Malubag, Villanueva
// 4 BS Computer Engineering

// ENGG 123.01
// Project 3: RISC-V Pipeline Simulator using C++

#ifndef FUNCTION_H
#define FUNCTION_H

#include <iostream>
#include <algorithm>
#include <bitset>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <iomanip>

using namespace std;

bool isValidHex(const string &s, int length);

bool loadData(const string &address, const string &filename, 
              uint8_t * &data_memory, const int memory_size);
bool loadCode(const string &address, const string &filename, 
              uint8_t * &instruction_memory, const int memory_size);
void showData(string &address, int N, uint8_t * &data_memory,
              const int memory_size);
void showCode(string &address, int N, uint8_t * &instruction_memory,
              const int memory_size);

// PIPELINING STAGES
//unsigned int instruction_fetch(int pc, uint8_t 
//                               * &instruction_memory);
void instruction_decode(unsigned int instruction, long long *&reg,
                        string &inst, long long &a, long long &b,
                        long long &c, int &rd);
void instruction_execute(string &inst, long long &a, long long &b,
                         long long &c, int &rd, int &pc_offset);
void memory_access(string &inst, long long &a, long long &b, 
                   long long &c, int &rd, uint8_t *&mem, 
                   const int memory_size);
void write_back(string &inst, long long &c, int &rd, 
                long long * &reg);

int pipeline_loop(unsigned int instr, long long *&reg, 
                  uint8_t *&mem, const int memory_size);

#endif