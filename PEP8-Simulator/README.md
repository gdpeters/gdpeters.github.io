# PEP8-Simulator
This program simulates the PEP/8 Virtual Computer.

# PEP/8 Virtual Computer Background
The PEP/8 Virtual Computer executes a user's assembly source program or a hexadecimal object program. Its interface features a main memory block, registers, status bits, and I/O window panes. Its simplicity provides a direct look into how data is transferred from memory to registers. There is a debugging mode that lets the user trace the execution steps, moving on to the next instruction only when initiated by the user. There are thirty-nine instructions and eight addressing modes in its instruction set. PEP/8 can be downloaded at http://computersystemsbook.com/4th-edition/pep8/ and its source code written in C++ is available at https://github.com/StanWarford/pep8

# Introduction
This program attempts to simulate a subset the PEP/8 Virtual Computer using C++. It focuses strictly on machine code execution and does not provide a user interface. Users can input a text file. After each instruction, the registers, instruction specifier, and operand specifier are displayed. Trace/debugging mode is not available. Specific features and requirements are outlined below.

# Requirements
Input (.txt): Object code must be written in hexadecimal as two adjacent digits (2-nibbles) followed by a space. Multiple lines are accepted but no trailing spaces.

Syntax: Only 18 instruction specifiers and 2 addressing modes are allowed. There should be 3-bytes to an instruction (1-byte instruction specifier, 2-byte operand) except for Unary instructions which have only 1-byte.

# Instruction Set
/*List all instructions*/

