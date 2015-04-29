///////////////////////////////////////////////////////////////////////////////
//
// m68000.hxx
//
// Motorola 68000 microprocessor class
//
// Sim68000 "Motorola 68000 Simulator"
// Copyright (c) 1993
// By: Bradford W. Mott
// October 31,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: m68000.hxx,v 1.1 1996/08/02 14:59:25 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef M68000_HXX
#define M68000_HXX

#include <string>

#ifdef WIN32
  #include <iostream>
#else
  #include <iostream.h>
#endif

#ifdef USE_STD
  using namespace std;
#endif

class BasicDevice;

#include "BasicCPU.hxx"

// Instruction Size Constants
#define BYTE   0
#define WORD   1
#define LONG   2

// Set Condition Code operation types
#define ADDITION    0
#define SUBTRACTION 1
#define OTHER       2

class m68000 : public BasicCPU {
  public:
    // Constructor
    m68000();

    // Destructor
    virtual ~m68000();

    // Execute next instruction. Answers pointer to an error message or null
    const char* ExecuteInstruction(string& traceRecord, bool tracing);

    // Service pending interrupts, serviceFlag set true iff something serviced
    int ServiceInterrupts(bool& serviceFlag);

    // Handle an interrupt request from a device
    void InterruptRequest(BasicDevice* device, int level);

    // Preform a system reset
    void Reset();

    // Return the value of the program counter register
    unsigned long ValueOfProgramCounter();

    // Sets named register to the given hexidecimal value
    void SetRegister(const string& name, const string& hexValue);

    // Clear the CPU's Statistics
    void ClearStatistics()
    {}

    // Append all of the CPU's registers to the RegisterInformationList object
    void BuildRegisterInformationList(RegisterInformationList& list);

    // Append all of the CPU's stats to the StatisticalInformationList object
    void BuildStatisticalInformationList(StatisticalInformationList& list);

  private:
    // Used for register information table
    struct RegisterData {
      char* name;
      unsigned long mask;
      char* description;
    };

  private:
    // Number of registers in the cpu
    const int myNumberOfRegisters;

    // Array of static information for each register 
    static RegisterData ourRegisterData[];

    // Pointer to an array of values for each register
    unsigned long* register_value;

    // Status Register masks
    const unsigned long C_FLAG;         // Carry
    const unsigned long V_FLAG;         // Overflow
    const unsigned long Z_FLAG;         // Zero
    const unsigned long N_FLAG;         // Negative
    const unsigned long X_FLAG;         // Extend
    const unsigned long I0_FLAG;        // Interrupt Mask Level
    const unsigned long I1_FLAG;        // Interrupt Mask Level
    const unsigned long I2_FLAG;        // Interrupt Mask Level
    const unsigned long S_FLAG;         // Supervisory
    const unsigned long T_FLAG;         // Trace Mode

    // Indices into the register arrays (Note: D0-D7-A0-A7' must be sequential)
    const unsigned int D0_INDEX;
    const unsigned int A0_INDEX;
    const unsigned int USP_INDEX;
    const unsigned int SSP_INDEX;
    const unsigned int PC_INDEX;
    const unsigned int SR_INDEX;
 
    // Execution return constants
    const int EXECUTE_OK;
    const int EXECUTE_PRIVILEGED_OK;
    const int EXECUTE_BUS_ERROR;
    const int EXECUTE_ADDRESS_ERROR;
    const int EXECUTE_ILLEGAL_INSTRUCTION;

    // Processor state (NORMAL_STATE, HALT_STATE, or STOP_STATE, BREAK_STATE)
    unsigned int myState;

    // Procesor state constants
    const unsigned int NORMAL_STATE;
    const unsigned int HALT_STATE;
    const unsigned int STOP_STATE;
    const unsigned int BREAK_STATE;
 
    // Pointer to an instruction execution routine
    typedef int (m68000::*ExecutionPointer)(int, string&, int);

    // DecodeEntry structure for the Decode Table
    struct DecodeEntry {
      unsigned int mask;
      unsigned int signature;
      ExecutionPointer execute;
    };

    static DecodeEntry ourDecodeTable[];
    static ExecutionPointer* ourDecodeCacheTable;

    // Decode the given instruction
    ExecutionPointer DecodeInstruction(int opcode);

    // Routines to simulate the execution of the instruction
    int ExecuteABCD(int opcode, string& description, int trace);
    int ExecuteADD(int opcode, string& description, int trace);
    int ExecuteADDA(int opcode, string& description, int trace);
    int ExecuteADDI(int opcode, string& description, int trace);
    int ExecuteADDQ(int opcode, string& description, int trace);
    int ExecuteADDX(int opcode, string& description, int trace);
    int ExecuteAND(int opcode, string& description, int trace);
    int ExecuteANDI(int opcode, string& description, int trace);
    int ExecuteANDItoCCR(int opcode, string& description, int trace);
    int ExecuteANDItoSR(int opcode, string& description, int trace);
    int ExecuteASL(int opcode, string& description, int trace);
    int ExecuteASR(int opcode, string& description, int trace);
    int ExecuteBRA(int opcode, string& description, int trace);
    int ExecuteBREAK(int opcode, string& description, int trace);
    int ExecuteBSR(int opcode, string& description, int trace);
    int ExecuteBcc(int opcode, string& description, int trace);
    int ExecuteBit(int opcode, string& description, int trace);
    int ExecuteCHK(int opcode, string& description, int trace);
    int ExecuteCLR(int opcode, string& description, int trace);
    int ExecuteCMP(int opcode, string& description, int trace);
    int ExecuteCMPA(int opcode, string& description, int trace);
    int ExecuteCMPI(int opcode, string& description, int trace);
    int ExecuteCMPM(int opcode, string& description, int trace);
    int ExecuteDBcc(int opcode, string& description, int trace);
    int ExecuteDIVS(int opcode, string& description, int trace);
    int ExecuteDIVU(int opcode, string& description, int trace);
    int ExecuteEOR(int opcode, string& description, int trace);
    int ExecuteEORI(int opcode, string& description, int trace);
    int ExecuteEORItoCCR(int opcode, string& description, int trace);
    int ExecuteEORItoSR(int opcode, string& description, int trace);
    int ExecuteEXG(int opcode, string& description, int trace);
    int ExecuteEXT(int opcode, string& description, int trace);
    int ExecuteILLEGAL(int opcode, string& description, int trace);
    int ExecuteJMP(int opcode, string& description, int trace);
    int ExecuteJSR(int opcode, string& description, int trace);
    int ExecuteLEA(int opcode, string& description, int trace);
    int ExecuteLINK(int opcode, string& description, int trace);
    int ExecuteLSL(int opcode, string& description, int trace);
    int ExecuteLSR(int opcode, string& description, int trace);
    int ExecuteMOVE(int opcode, string& description, int trace);
    int ExecuteMOVEA(int opcode, string& description, int trace);
    int ExecuteMOVEM(int opcode, string& description, int trace);
    int ExecuteMOVEP(int opcode, string& description, int trace);
    int ExecuteMOVEQ(int opcode, string& description, int trace);
    int ExecuteMOVEUSP(int opcode, string& description, int trace);
    int ExecuteMOVEfromSR(int opcode, string& description, int trace);
    int ExecuteMOVEtoCCR(int opcode, string& description, int trace);
    int ExecuteMOVEtoSR(int opcode, string& description, int trace);
    int ExecuteMULS(int opcode, string& description, int trace);
    int ExecuteMULU(int opcode, string& description, int trace);
    int ExecuteNBCD(int opcode, string& description, int trace);
    int ExecuteNEG(int opcode, string& description, int trace);
    int ExecuteNEGX(int opcode, string& description, int trace);
    int ExecuteNOP(int opcode, string& description, int trace);
    int ExecuteNOT(int opcode, string& description, int trace);
    int ExecuteOR(int opcode, string& description, int trace);
    int ExecuteORI(int opcode, string& description, int trace);
    int ExecuteORItoCCR(int opcode, string& description, int trace);
    int ExecuteORItoSR(int opcode, string& description, int trace);
    int ExecutePEA(int opcode, string& description, int trace);
    int ExecuteRESET(int opcode, string& description, int trace);
    int ExecuteROL(int opcode, string& description, int trace);
    int ExecuteROR(int opcode, string& description, int trace);
    int ExecuteROXL(int opcode, string& description, int trace);
    int ExecuteROXR(int opcode, string& description, int trace);
    int ExecuteRTE(int opcode, string& description, int trace);
    int ExecuteRTR(int opcode, string& description, int trace);
    int ExecuteRTS(int opcode, string& description, int trace);
    int ExecuteSBCD(int opcode, string& description, int trace);
    int ExecuteSTOP(int opcode, string& description, int trace);
    int ExecuteSUB(int opcode, string& description, int trace);
    int ExecuteSUBA(int opcode, string& description, int trace);
    int ExecuteSUBI(int opcode, string& description, int trace);
    int ExecuteSUBQ(int opcode, string& description, int trace);
    int ExecuteSUBX(int opcode, string& description, int trace);
    int ExecuteSWAP(int opcode, string& description, int trace);
    int ExecuteScc(int opcode, string& description, int trace);
    int ExecuteTAS(int opcode, string& description, int trace);
    int ExecuteTRAP(int opcode, string& description, int trace);
    int ExecuteTRAPV(int opcode, string& description, int trace);
    int ExecuteTST(int opcode, string& description, int trace);
    int ExecuteUNLK(int opcode, string& description, int trace);
    int ExecuteInvalid(int opcode, string& description, int trace);

    int ExecuteBusError(int opcode, string& description, int trace);
    int ExecuteAddressError(int opcode, string& description, int trace);

    // Helpful routines for executing instructions
    int ComputeEffectiveAddress(unsigned long& address,
        int& in_register, string &description, int mode_register,
        int size, int trace);

    int Peek(unsigned long address, unsigned int& value, int size);
    int Poke(unsigned long address, unsigned int value, int size);

    unsigned int SignExtend(unsigned int value, int size);

    void SetConditionCodes(unsigned int src, unsigned int dest,
        unsigned int result, int size, int operation, int mask);

    void ClearConditionCodes(int mask);

    int  CheckConditionCodes(int code, string& mnemonic, int trace);

    void SetRegister(int register_number, unsigned int value, int size);

    int  ProcessException(int vector);
 
    long my_interrupt;
    BasicDevice* my_device;
};
#endif

