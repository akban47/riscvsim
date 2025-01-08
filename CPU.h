#ifndef CPU_H
#define CPU_H
#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstdint>
#include <fstream>
#include <vector>
#include <array>
using namespace std;

class ALU
{
public:
    uint32_t performOperation(uint32_t operand1, uint32_t operand2, uint8_t aluControl);
};

class ALUController
{
public:
    uint8_t generateALUControl(uint8_t aluOp, uint32_t funct3, uint32_t funct7);
};

struct ControlSignals
{
    bool regWrite;     // Write to register
    bool aluSrc;       // Select between register and immediate
    bool branch;       // Branch signal
    bool memRead;      // Read from memory
    bool memWrite;     // Write to memory
    bool jump;         // Jump signal
    bool memToReg;     // Select between ALU result or memory data
    uint8_t aluOp;     // ALU operation
    uint32_t rs2_val;  // Value from rs2 for stores
    uint32_t funct3;   // Function code for memory ops
    int32_t branchImm; //  branch/jump immediate
};

class CPU
{
private:
    uint32_t PC;                        // pc
    uint32_t nextPC;                    // next pc
    vector<uint32_t> instructionMemory; // instruction memory
    array<uint32_t, 32> registers;      // 32 general-purpose registers
    array<uint8_t, 4096> memory;        // data memory
    uint32_t aluResult;                 // ALU result
    uint32_t memData;                   // Memory data

public:
    CPU();
    uint32_t fetchInstruction(uint32_t address);        // Fetch instruction at given address
    void loadInstruction(uint32_t instruction);         // Load instruction into memory
    uint32_t readRegister(uint8_t regNum);              // Reads a value from a register
    void writeRegister(uint8_t regNum, uint32_t value); // Writes a value to a register
    uint8_t loadByte(uint32_t address);                 // Loads a byte from memory
    uint32_t loadWord(uint32_t address);                // Loads a word from memory
    void storeByte(uint32_t address, uint8_t value);    // Stores a byte in memory
    void storeWord(uint32_t address, uint32_t value);   // Stores a word in memory
    void setPC(uint32_t newPC);                         // Sets the PC to a new value
    void incPC();                                       // Increments PC by 4
    uint32_t readPC();                                  // Returns the current PC value
    uint32_t maxPC();
    uint32_t getaluResult();
    void executePhase(ControlSignals signals, uint32_t operand1, uint32_t operand2, ALU &alu, ALUController &aluController, uint32_t funct3, uint32_t funct7); // ALU Execution
    void memoryPhase(ControlSignals signals, uint32_t aluResult, uint32_t &memData);                                                                           // Memory access phase
    void writeBackPhase(ControlSignals signals, uint32_t memData, uint32_t aluResult, uint32_t rd);                                                            // Write-back phase
    void clockTickPhase(ControlSignals signals);                                                                                                               // Clock tick phase to update state

    enum OPCODES
    {                  // enum for opcodes
        R_TYPE = 0x33, // R-type (ADD, XOR)
        I_TYPE = 0x13, // I-type (ORI, SRAI)
        LB_LW = 0x03,  // Load instructions (LB, LW)
        SB_SW = 0x23,  // Store instructions (SB, SW)
        BEQ = 0x63,    // B-type (BEQ)
        LUI = 0x37,    // U-type (LUI)
        JAL = 0x6F     // J-type (JAL)
    };
};
class ControlUnit
{
public:
    ControlSignals decode(uint32_t instruction, uint32_t &operand1, uint32_t &operand2, uint32_t &rd, CPU &cpu); // Decodes instruction
};

#endif // CPU_H