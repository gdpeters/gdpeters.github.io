/** CSC 376 Computer Organization (Dr. Luke Vespa)
 Midterm Project - PEP/8 Simulation
 Christopher Cook & Genevieve Peters
 
 This program attempts to simulate the PEP/8 Virtual Computer. It computes a set of
 hexadecimal machine code instructions from a file. Only 18 instruction specifiers and 2 addressing
 modes are allowed. There should be 3-bytes to an instruction (1-byte instruction specifier,
 2-byte operand) except for Unary instructions which have only 1-byte.
 
 For clarification:
 Accumulator = 0
 Index = 1
 Immediate Addressing = 000
 Direct Addressing = 001
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <locale>
using namespace std;

unsigned int MEMORY_SIZE = 10000;   //Size can be changed as desired
unsigned int sP = MEMORY_SIZE - 1072;    //Stack Pointer positioning

struct registerAI {     //Accumulator and Index registers
    union {
        struct {
            unsigned int lsd8 : 8;
            unsigned int msd8 : 8;
        };
        unsigned int full : 16;
    };
};

struct registers {      //Processing unit

    unsigned int pC : 16;    //Program counter
    registerAI pickRegister[2];    //Accesses Accumulator/Index registers
    
    //Instruction Specifier divided into various bit structures
    union {
        struct {
            unsigned int r1 : 1;
            unsigned int instr7 : 7;
        } unary;    //Unary instructions (0000 000r)
        
        struct {
            unsigned int aaa3 : 3;
            unsigned int instr5 : 5;
        } ioTrap;    //I/O instructions (0000 0aaa)
        
        struct {
            unsigned int aaa3 : 3;
            unsigned int r1 : 1;
            unsigned int instr4 : 4;
        } arith;    //Arithmetic instructions (0000 raaa)
        
        unsigned int full : 8;  //Full instruction specifier byte
    } iSpec;
    
    //Operand Specifier, Operand, and temporary values (for Deci/Deco)
    union {
        struct {
            unsigned int lsd8 : 8;
            unsigned int msd8 : 8;
        };
        unsigned int full : 16;
    } oSpec, operand, tempValue, tempAddress;
};

string printRegisters(registers rgstr)      //Prints register values when called after each computation
 {
     stringstream stream;
     stream << showbase << uppercase << hex << internal << setfill('0');
     stream << "Accumulator: " << setw(6) << rgstr.pickRegister[0].full
            << "\nIndex: " << setw(6) << rgstr.pickRegister[1].full
            << "\nStack Pointer: " << setw(6) << sP
            << "\nProgram Counter: " << setw(6) << rgstr.pC
            << "\nInstruction Specifier: " << setw(4) << rgstr.iSpec.full
            << "\nOperand Specifier: " << setw(6) << rgstr.oSpec.full
            << "\nOperand: " << setw(6) << rgstr.operand.full << "\n\n";

     return stream.str();
 }

int main() {
    
    ifstream readFile;
    readFile.open("input.txt", ios::in);        //Input file with hexadecimal machine code
    
    ofstream outputFile;
    outputFile.open ("output.txt");             //Output file with register values after each computation

    unsigned int mainMem[MEMORY_SIZE];
    unsigned int tempHex;
    int byteIndex = 0;
    
    while (readFile >> hex >> tempHex)
    {
        mainMem[byteIndex++] = tempHex;  //Load 1 byte from input file into main memory
    }
    
    registers cpu;          //Declare a set of new registers
    cpu.pC = 0;             //Initiate program counter to 0
    cpu.iSpec.full = mainMem[cpu.pC++];    //Load first instruction from memory array
    
    while (cpu.iSpec.full != 0)     //While the instruction is not 00 = Stop
    {
        if (cpu.iSpec.arith.instr4 > 3)
        {
            cpu.oSpec.msd8 = mainMem[cpu.pC++];  //Update operand specifier if not Unary
            cpu.oSpec.lsd8 = mainMem[cpu.pC++];
        }
        if (cpu.iSpec.arith.instr4 == 5)
        {
            cpu.operand.full = (cpu.iSpec.arith.aaa3 == 0) * (cpu.oSpec.full - mainMem[cpu.oSpec.full]) + mainMem[cpu.oSpec.full];
        }

        if ((cpu.iSpec.arith.instr4 > 5) && (cpu.iSpec.arith.instr4 < 14))  //Update operand if instruction code is not unary, I/O, or storing registers
        {
            cpu.operand.msd8 = (cpu.iSpec.arith.aaa3 == 0) * (cpu.oSpec.msd8 - mainMem[cpu.oSpec.full]) + mainMem[cpu.oSpec.full];
            cpu.operand.lsd8 = (cpu.iSpec.arith.aaa3 == 0) * (cpu.oSpec.lsd8 - mainMem[cpu.oSpec.full + 1]) + mainMem[cpu.oSpec.full + 1];
        }
        
        switch (cpu.iSpec.arith.instr4)    //Check first nibble to extract instruction code
        {
            case 1:     //Shifts and Invert
            {
                cpu.operand.full = 0;    //Unary no operand
                switch (cpu.iSpec.unary.instr7)
                {
                    case 12: //Bitwise invert
                        cpu.pickRegister[cpu.iSpec.unary.r1].full = cpu.pickRegister[cpu.iSpec.unary.r1].full ^ ((1 << 16) - 1);
                        break;
                    case 14: //Arithmetic shift left
                        cpu.pickRegister[cpu.iSpec.unary.r1].full <<= 1;
                        break;
                    case 15: //Arithmetic shift right
                        cpu.pickRegister[cpu.iSpec.unary.r1].full >>= 1;
                        break;
                }
                break;
            }
            case 2:     //Rotate
            {
                cpu.operand.full = 0;    //Unary no operand
                switch (cpu.iSpec.unary.instr7)
                {
                    case 16: //Rotate left
                        cpu.pickRegister[cpu.iSpec.unary.r1].full = (cpu.pickRegister[cpu.iSpec.unary.r1].full << 1) + (cpu.pickRegister[cpu.iSpec.unary.r1].full >> 15);
                        break;
                    case 17: //Rotate right
                        cpu.pickRegister[cpu.iSpec.unary.r1].full = (cpu.pickRegister[cpu.iSpec.unary.r1].full >> 1) + (cpu.pickRegister[cpu.iSpec.unary.r1].full << 15);
                        break;
                }
                break;
            }
            case 3:     //Decimal I/O
            {
                
                switch (cpu.iSpec.ioTrap.instr5)
                {
                    case 6:    //Decimal input
                        int decI;
                        cin >> decI;
                        cpu.tempValue.full = decI;
                        cpu.tempAddress.msd8 = mainMem[cpu.pC++];
                        cpu.tempAddress.lsd8 = mainMem[cpu.pC++];
                        mainMem[cpu.tempAddress.full] = cpu.tempValue.msd8;
                        mainMem[cpu.tempAddress.full + 1] = cpu.tempValue.lsd8;
                        break;
                    case 7:    //Decimal output
                        unsigned int decO;
                        if (cpu.iSpec.ioTrap.aaa3 == 1)
                        {
                            cpu.tempAddress.msd8 = mainMem[cpu.pC++];
                            cpu.tempAddress.lsd8 = mainMem[cpu.pC++];
                            cpu.tempValue.msd8 = mainMem[cpu.tempAddress.full];
                            cpu.tempValue.lsd8 = mainMem[cpu.tempAddress.full + 1];
                        }
                        else
                        {
                            cpu.tempValue.msd8 = mainMem[cpu.pC++];
                            cpu.tempValue.lsd8 = mainMem[cpu.pC++];
                        }

                        decO = cpu.tempValue.full;
                        cout<<endl<<decO;
                        break;
                }
                break;
            }
            case 4:    //Character input only takes 1-byte (0x00)
            {
                string charI;
                cin>>charI;
                mainMem[cpu.oSpec.full] = stoi(charI);
                cpu.operand.full = mainMem[cpu.oSpec.full];
                break;
            }
            case 5:    //Character output only takes 1-byte (00)
            {
                char charO = (char) cpu.operand.full;
                cout<<charO;
                break;
            }
                
            //Case 6 not included in instruction set
                
            case 7:    //Add to register
            {
                cpu.pickRegister[cpu.iSpec.arith.r1].full += cpu.operand.full;
                break;
            }
            case 8:    //Subtract from register
            {
                cpu.pickRegister[cpu.iSpec.arith.r1].full -= cpu.operand.full;
                break;
            }
            case 9: //Bitwise AND
            {
                cpu.pickRegister[cpu.iSpec.arith.r1].full &= cpu.operand.full;
                break;
            }
            case 10: //Bitwise OR
            {
                cpu.pickRegister[cpu.iSpec.arith.r1].full |= cpu.operand.full;
                break;
            }
            
            //Case 11 not included in instruction set
                
            case 12: //Load register from memory
            {
                cpu.pickRegister[cpu.iSpec.arith.r1].full = cpu.operand.full;
                break;
            }
            case 13: //Load byte from memory
            {
                cpu.pickRegister[cpu.iSpec.arith.r1].lsd8 = cpu.operand.lsd8;
                break;
            }
            case 14: //Store register to memory
            {
                cpu.operand.full = cpu.pickRegister[cpu.iSpec.arith.r1].full;
                mainMem[cpu.oSpec.full] = cpu.operand.msd8;
                mainMem[cpu.oSpec.full + 1] = cpu.operand.lsd8;
                break;
            }
            case 15: //Store byte from register to memory
            {
                cpu.operand.full = cpu.pickRegister[cpu.iSpec.arith.r1].lsd8;
                mainMem[cpu.oSpec.full] = cpu.operand.lsd8;
                break;
            }
        }
        cpu.tempValue.full = 0;     //Reset temporary value
        cpu.tempAddress.full = 0;   //Reset temporary address
        outputFile<<printRegisters(cpu);    //Print registers to output file

        cpu.iSpec.full = mainMem[cpu.pC++]; //Load next instruction into the Instruction Specifier
    }
    outputFile.close(); //Close output file
    
    //Used to pause termination
    int s;
    cin>>s;
    
    return 0;
}
