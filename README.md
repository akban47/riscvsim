# RISC-V CPU Simulator
A cycle accurate RISC-V processor simulator implementing a subset of the RV32I ISA. Includes instruction fetch/decode/execute cycles, pipelined execution phases, and memory hierarchy simulation.
## Architecture Overview
The simulator implements the following phases:
- **Fetch**: Retrieve instruction from instruction memory
- **Decode**: Parse instruction fields and generate control signals  
- **Execute**: Perform ALU operations and address calculations
- **Memory**: Handle load/store operations with data memory
- **Write back**: Update register file with results
## Supported Instructions
### R-Type Instructions
- `ADD` - Add two registers
- `SUB` - Subtract two registers  
- `XOR` - XOR of two registers
### I-Type Instructions
- `ORI` - OR with immediate value
- `SRAI` - Shift right arithmetic immediate
### Load/Store Instructions
- `LB` - Load byte from memory
- `LW` - Load word from memory
- `SB` - Store byte to memory
- `SW` - Store word to memory
### Branch/Jump Instructions
- `BEQ` - Branch if equal
- `JAL` - Jump and link
### Misc
- `LUI` - Load upper immediate
## File Structure
```
├── CPU.h           # Class definitions
├── CPU.cpp         # Core CPU
├── cpusim.cpp      # Driver
└── README.md       # This file
```
## Usage
### Input Format
The simulator expects machine code in hexadecimal format, with one byte per line:
```
13
01
51
00
b3
82
24
00
```
Each instruction is represented as 4 byte little endian
### Output
The simulator outputs the final values of registers x10 and x11:
```
(42,84)
```
