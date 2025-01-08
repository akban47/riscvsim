#include "CPU.h"
#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

int main(int argc, char *argv[])
{

	if (argc < 2)
		return -1;

	ifstream infile(argv[1]); // open the file
	if (!(infile.is_open() && infile.good()))
		return 0;

	CPU myCPU;
	ControlUnit controlUnit;
	ALUController aluController;
	ALU alu;

	string line;
	int i = 0;
	uint32_t instruction;
	while (getline(infile, line))
	{
		stringstream line2(line);
		unsigned int hexByte;
		line2 >> hex >> hexByte;
		instruction |= (hexByte & 0xFF) << (8 * i); // Accumulate bytes into the 32-bit instruction
		i++;

		if (i == 4)
		{
			myCPU.loadInstruction(instruction); // Store the full 32-bit instruction
			instruction = 0;					// Reset for the next instruction
			i = 0;								// Reset byte counter
		}
	}
	infile.close();

	while (true)
	{
		uint32_t currentPC = myCPU.readPC();
		// Fetch instruction
		uint32_t instruction = myCPU.fetchInstruction(myCPU.readPC());

		// Decode instruction using ControlUnit
		uint32_t operand1, operand2, rd;
		ControlSignals signals = controlUnit.decode(instruction, operand1, operand2, rd, myCPU);

		// Execute instruction
		uint32_t funct3 = (instruction >> 12) & 0x7;
		uint32_t funct7 = (instruction >> 25) & 0x7F;
		myCPU.executePhase(signals, operand1, operand2, alu, aluController, funct3, funct7);

		// Memory phase: Handle memory load/store operations
		uint32_t memData = 0;
		myCPU.memoryPhase(signals, myCPU.getaluResult(), memData);

		// Write back phase: Write result to register file
		myCPU.writeBackPhase(signals, memData, myCPU.getaluResult(), rd);

		// Clock tick phase: Update program counter (PC)
		myCPU.clockTickPhase(signals);

		if (myCPU.readPC() >= myCPU.maxPC())
			break;
	}

	cout << "(" << dec << myCPU.readRegister(10) << "," << dec << myCPU.readRegister(11) << ")" << endl;
	return 0;
}