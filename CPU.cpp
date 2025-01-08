#include "CPU.h"

CPU::CPU()
{
    PC = nextPC = 0;   // set PC to 0
    memory.fill(0);    // initialize data memory to 0
    registers.fill(0); // initialize registers to 0
}

uint32_t CPU::fetchInstruction(uint32_t address)
{
    return instructionMemory[address / 4]; // Fetch the 32-bit instruction
}

// Load instruction into memory
void CPU::loadInstruction(uint32_t instruction)
{
    instructionMemory.push_back(instruction); // Load instruction into memory
}

uint32_t CPU::readRegister(uint8_t regNum)
{
    return registers[regNum]; // Read the value from the register
}

void CPU::writeRegister(uint8_t regNum, uint32_t value)
{
    if (regNum != 0) // x0 is hardwired to 0
        registers[regNum] = value;
}

uint8_t CPU::loadByte(uint32_t address)
{
    return memory[address];
}

uint32_t CPU::loadWord(uint32_t address)
{
    return (memory[address] | (memory[address + 1] << 8) | (memory[address + 2] << 16) | (memory[address + 3] << 24));
}

void CPU::storeByte(uint32_t address, uint8_t value)
{
    memory[address] = value;
}

void CPU::storeWord(uint32_t address, uint32_t value)
{
    memory[address] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
    memory[address + 2] = (value >> 16) & 0xFF;
    memory[address + 3] = (value >> 24) & 0xFF;
}

void CPU::setPC(uint32_t newPC)
{
    PC = newPC;
}

uint32_t CPU::readPC()
{
    return PC;
}

void CPU::incPC()
{
    PC += 4;
}

uint32_t CPU::maxPC()
{
    return instructionMemory.size() * 4;
}

uint32_t CPU::getaluResult()
{
    return aluResult;
}

uint32_t ALU::performOperation(uint32_t operand1, uint32_t operand2, uint8_t aluControl)
{
    uint32_t result;
    switch (aluControl)
    {
    case 0b0001: // OR
        result = operand1 | operand2;
        break;
    case 0b0010: // ADD
        result = operand1 + operand2;
        break;
    case 0b0110: // SUB (for BEQ)
        result = operand1 - operand2;
        break;
    case 0b0100: // XOR
        result = operand1 ^ operand2;
        break;
    case 0b0101: // SRAI
        result = (int32_t)operand1 >> (operand2 & 0x1F);
        break;
    default:
        result = 0;
    }
    return result;
}

// ALU Controller generates control signals based on ALUOp, funct3, and funct7
uint8_t ALUController::generateALUControl(uint8_t aluOp, uint32_t funct3, uint32_t funct7)
{
    uint8_t aluControl;

    if (aluOp == 0b00)
    {
        return 0b0010; // ADD for load/store/lui
    }
    else if (aluOp == 0b01)
    {
        return 0b0110; // SUB for branch comparison
    }
    else if (aluOp == 0b10)
    {
        switch (funct3)
        {
        case 0x0: // ADD/SUB
            aluControl = (funct7 == 0x20) ? 0b0110 : 0b0010;
            break;
        case 0x4: // XOR
            aluControl = 0b0100;
            break;
        case 0x5: // SRAI
            aluControl = 0b0101;
            break;
        case 0x6: // OR
            aluControl = 0b0001;
            break;
        default:
            aluControl = 0b0010; // Default to ADD
        }
    }
    else
    {
        aluControl = 0b0010; // Default to ADD
    }
    return aluControl;
}

ControlSignals ControlUnit::decode(uint32_t instruction, uint32_t &operand1, uint32_t &operand2, uint32_t &rd, CPU &cpu)
{
    ControlSignals signals = {0};
    uint32_t opcode = instruction & 0x7F;
    rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t funct7 = (instruction >> 25) & 0x7F;

    switch (opcode)
    {
    case 0x13:
    { // I-type (ORI, SRAI)
        signals.regWrite = true;
        signals.aluSrc = true;
        signals.aluOp = 0b10;
        operand1 = cpu.readRegister(rs1);

        if (funct3 == 0x5)
        {                                          // SRAI
            operand2 = (instruction >> 20) & 0x1F; // shamt
        }
        else if (funct3 == 0x6)
        { // ORI
            // Extract 12-bit immediate and sign extend
            int32_t imm = ((int32_t)instruction) >> 20;
            operand2 = imm;
        }
        break;
    }

    case 0x33:
    { // R-type (ADD, XOR)
        signals.regWrite = true;
        signals.aluSrc = false;
        signals.aluOp = 0b10;
        operand1 = cpu.readRegister(rs1);
        operand2 = cpu.readRegister(rs2);
        break;
    }
    case 0x03:
    { // Load (LB, LW)
        signals.memRead = true;
        signals.regWrite = true;
        signals.aluSrc = true;
        signals.memToReg = true;
        signals.aluOp = 0b00;
        operand1 = cpu.readRegister(rs1);
        operand2 = ((int32_t)instruction) >> 20; // Sign-extended immediate
        break;
    }
    case 0x23:
    { // Store (SB, SW)
        signals.memWrite = true;
        signals.aluSrc = true;
        signals.aluOp = 0b00;
        // S-type immediate: combine imm[11:5] and imm[4:0]
        uint32_t imm_hi = (instruction >> 25) & 0x7F; // imm[11:5]
        uint32_t imm_lo = (instruction >> 7) & 0x1F;  // imm[4:0]
        int32_t imm = (imm_hi << 5) | imm_lo;
        // Sign extend
        imm = (imm << 20) >> 20;
        operand1 = cpu.readRegister(rs1);        // Base address
        operand2 = imm;                          // Offset
        signals.rs2_val = cpu.readRegister(rs2); // Value to store
        break;
    }

    case 0x37:
    { // LUI
        signals.regWrite = true;
        signals.aluSrc = true;
        signals.aluOp = 0b00;
        operand1 = 0;
        operand2 = instruction & 0xFFFFF000;
        break;
    }
    case 0x63:
    { // BEQ
        signals.branch = true;
        signals.aluOp = 0b01; // For comparison

        // B-type immediate construction
        int32_t imm = (((instruction >> 31) & 0x1) << 12) | // imm[12]
                      (((instruction >> 7) & 0x1) << 11) |  // imm[11]
                      (((instruction >> 25) & 0x3F) << 5) | // imm[10:5]
                      (((instruction >> 8) & 0xF) << 1);    // imm[4:1]
        // Sign extend
        imm = (imm << 19) >> 19;
        signals.branchImm = imm;
        operand1 = cpu.readRegister(rs1);
        operand2 = cpu.readRegister(rs2);
        break;
    }
    case 0x6F:
    { // JAL
        signals.jump = true;
        signals.regWrite = true;
        signals.aluOp = 0b00;

        // J-type immediate construction
        int32_t imm = (((instruction >> 31) & 0x1) << 20) |  // imm[20]
                      (((instruction >> 12) & 0xFF) << 12) | // imm[19:12]
                      (((instruction >> 20) & 0x1) << 11) |  // imm[11]
                      (((instruction >> 21) & 0x3FF) << 1);  // imm[10:1]
        // Sign extend
        imm = (imm << 11) >> 11;

        signals.branchImm = imm;
        operand1 = cpu.readPC();
        operand2 = imm;
        break;
    }
    }

    // Generate ALU control
    uint8_t aluControl;
    if (opcode == 0x13 && funct3 == 0x6)
    {                        // ORI
        aluControl = 0b0001; // OR operation
    }
    else if (opcode == 0x13 && funct3 == 0x5)
    {                        // SRAI
        aluControl = 0b0101; // Shift right arithmetic
    }
    else if (opcode == 0x33 && funct3 == 0x4)
    {                        // XOR
        aluControl = 0b0100; // XOR operation
    }
    else
    {
        aluControl = 0b0010; // Default ADD
    }

    return signals;
}

void CPU::memoryPhase(ControlSignals signals, uint32_t aluResult, uint32_t &memData)
{
    if (signals.memWrite)
    {
        // Check funct3 for type of store
        if (signals.funct3 == 0x0)
        {                                              // SB
            uint8_t byte_val = signals.rs2_val & 0xFF; // Get least significant byte
            storeByte(aluResult, byte_val);
        }
        else // SW
            storeWord(aluResult, signals.rs2_val);
    }
    else if (signals.memRead)
        memData = loadWord(aluResult);
}

void CPU::executePhase(ControlSignals signals, uint32_t operand1, uint32_t operand2, ALU &alu, ALUController &aluController, uint32_t funct3, uint32_t funct7)
{
    if (signals.jump)
    {
        aluResult = PC + 4;              // Store return address first
        nextPC = PC + signals.branchImm; // Calculate jump target
    }
    else if (signals.branch)
    {
        uint8_t aluControl = aluController.generateALUControl(signals.aluOp, funct3, funct7);
        uint32_t comparison = alu.performOperation(operand1, operand2, aluControl);
        if (comparison == 0)
            nextPC = PC + signals.branchImm;
        else
            nextPC = PC + 4;
    }
    else
    {
        uint8_t aluControl = aluController.generateALUControl(signals.aluOp, funct3, funct7);
        aluResult = alu.performOperation(operand1, operand2, aluControl);
        nextPC = PC + 4;
    }
}

void CPU::writeBackPhase(ControlSignals signals, uint32_t memData, uint32_t aluResult, uint32_t rd)
{
    if (signals.regWrite)
    {
        if (signals.memToReg)
            writeRegister(rd, memData);
        else
            writeRegister(rd, aluResult);
    }
}

void CPU::clockTickPhase(ControlSignals signals)
{
    PC = nextPC;
}