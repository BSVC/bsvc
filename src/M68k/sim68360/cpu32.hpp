#ifndef M68K_SIM68360_CPU32_HPP_
#define M68K_SIM68360_CPU32_HPP_

#include <string>

#include "Framework/BasicCPU.hpp"

class BasicDevice;

// Set Condition Code operation types.
#define ADDITION 0
#define SUBTRACTION 1
#define OTHER 2

class cpu32 : public BasicCPU {
public:
  cpu32();
  virtual ~cpu32();

  // Executes the next instruction. Returns an error message or the empty string.
  std::string ExecuteInstruction(std::string &traceRecord, bool tracing);

  // Services pending interrupts. Sets serviceFlag true iff something serviced.
  int ServiceInterrupts(bool &serviceFlag);

  // Handles an interrupt request from a device.
  void InterruptRequest(BasicDevice *device, int level);

  // Performs a system reset.
  void Reset();

  // Returns the value of the program counter register.
  Address ValueOfProgramCounter();

  // Sets named register to the given hexidecimal value.
  void SetRegister(const std::string &name, const std::string &hexValue);

  // Clears the CPU's Statistics.
  void ClearStatistics() {}

  // Appends all of the CPU's registers to the RegisterInformationList object.
  void BuildRegisterInformationList(RegisterInformationList &list);

  // Appends all of the CPU's stats to the StatisticalInformationList object.
  void BuildStatisticalInformationList(StatisticalInformationList &list);

private:
  // Used for register information table.
  struct RegisterData {
    std::string name;
    Register mask;
    std::string description;
  };

private:
  // Number of registers in the CPU.
  const int myNumberOfRegisters;

  // Array of static information for each register.
  static RegisterData ourRegisterData[];

  // Pointer to an array of values for each register.
  Register *register_value;

  // Status Register masks.
  const Register C_FLAG;  // Carry
  const Register V_FLAG;  // Overflow
  const Register Z_FLAG;  // Zero
  const Register N_FLAG;  // Negative
  const Register X_FLAG;  // Extend
  const Register I0_FLAG; // Interrupt Mask Level
  const Register I1_FLAG; // Interrupt Mask Level
  const Register I2_FLAG; // Interrupt Mask Level
  const Register S_FLAG;  // Supervisory
  const Register T_FLAG;  // Trace Mode

  // Indices into the register arrays (Note: D0-D7-A0-A7' must be sequential)
  const unsigned int D0_INDEX;
  const unsigned int A0_INDEX;
  const unsigned int USP_INDEX;
  const unsigned int SSP_INDEX;
  const unsigned int PC_INDEX;
  const unsigned int SR_INDEX;
  // VBR, SFC and DFC registers
  const unsigned int VBR_INDEX;
  const unsigned int SFC_INDEX;
  const unsigned int DFC_INDEX;

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
  typedef int (cpu32::*ExecutionPointer)(int, std::string &, int);

  // DecodeEntry structure for the Decode Table
  struct DecodeEntry {
    unsigned int mask;
    unsigned int signature;
    ExecutionPointer execute;
  };

  static DecodeEntry ourDecodeTable[];
  static ExecutionPointer *ourDecodeCacheTable;

  // Decode the given instruction
  ExecutionPointer DecodeInstruction(int opcode);

  // Routines to simulate the execution of the instruction
  int ExecuteABCD(int opcode, std::string &description, int trace);
  int ExecuteADD(int opcode, std::string &description, int trace);
  int ExecuteADDA(int opcode, std::string &description, int trace);
  int ExecuteADDI(int opcode, std::string &description, int trace);
  int ExecuteADDQ(int opcode, std::string &description, int trace);
  int ExecuteADDX(int opcode, std::string &description, int trace);
  int ExecuteAND(int opcode, std::string &description, int trace);
  int ExecuteANDI(int opcode, std::string &description, int trace);
  int ExecuteANDItoCCR(int opcode, std::string &description, int trace);
  int ExecuteANDItoSR(int opcode, std::string &description, int trace);
  int ExecuteASL(int opcode, std::string &description, int trace);
  int ExecuteASR(int opcode, std::string &description, int trace);
  int ExecuteBRA(int opcode, std::string &description, int trace);
  int ExecuteBREAK(int opcode, std::string &description, int trace);
  int ExecuteBSR(int opcode, std::string &description, int trace);
  int ExecuteBcc(int opcode, std::string &description, int trace);
  int ExecuteBit(int opcode, std::string &description, int trace);
  int ExecuteCHK(int opcode, std::string &description, int trace);
  int ExecuteCLR(int opcode, std::string &description, int trace);
  int ExecuteCMP(int opcode, std::string &description, int trace);
  int ExecuteCMPA(int opcode, std::string &description, int trace);
  int ExecuteCMPI(int opcode, std::string &description, int trace);
  int ExecuteCMPM(int opcode, std::string &description, int trace);
  int ExecuteDBcc(int opcode, std::string &description, int trace);
  int ExecuteDIVS(int opcode, std::string &description, int trace);
  int ExecuteDIVU(int opcode, std::string &description, int trace);
  // DIVL: long division, S or U.
  int ExecuteDIVL(int opcode, std::string &description, int trace);
  int ExecuteEOR(int opcode, std::string &description, int trace);
  int ExecuteEORI(int opcode, std::string &description, int trace);
  int ExecuteEORItoCCR(int opcode, std::string &description, int trace);
  int ExecuteEORItoSR(int opcode, std::string &description, int trace);
  int ExecuteEXG(int opcode, std::string &description, int trace);
  int ExecuteEXT(int opcode, std::string &description, int trace);
  int ExecuteILLEGAL(int opcode, std::string &description, int trace);
  int ExecuteJMP(int opcode, std::string &description, int trace);
  int ExecuteJSR(int opcode, std::string &description, int trace);
  int ExecuteLEA(int opcode, std::string &description, int trace);
  int ExecuteLINK(int opcode, std::string &description, int trace);
  int ExecuteLSL(int opcode, std::string &description, int trace);
  int ExecuteLSR(int opcode, std::string &description, int trace);
  int ExecuteMOVE(int opcode, std::string &description, int trace);
  int ExecuteMOVEA(int opcode, std::string &description, int trace);
  // MOVES: move between address spaces
  int ExecuteMOVES(int opcode, std::string &description, int trace);
  // MOVEC: move from and to special registers VBR,SFC, DFC
  int ExecuteMOVEC(int opcode, std::string &description, int trace);
  int ExecuteMOVEM(int opcode, std::string &description, int trace);
  int ExecuteMOVEP(int opcode, std::string &description, int trace);
  int ExecuteMOVEQ(int opcode, std::string &description, int trace);
  int ExecuteMOVEUSP(int opcode, std::string &description, int trace);
  int ExecuteMOVEfromSR(int opcode, std::string &description, int trace);
  int ExecuteMOVEtoCCR(int opcode, std::string &description, int trace);
  int ExecuteMOVEtoSR(int opcode, std::string &description, int trace);
  int ExecuteMULS(int opcode, std::string &description, int trace);
  // MULL: long multiplication, S or U.
  int ExecuteMULL(int opcode, std::string &description, int trace);
  int ExecuteMULU(int opcode, std::string &description, int trace);
  int ExecuteNBCD(int opcode, std::string &description, int trace);
  int ExecuteNEG(int opcode, std::string &description, int trace);
  int ExecuteNEGX(int opcode, std::string &description, int trace);
  int ExecuteNOP(int opcode, std::string &description, int trace);
  int ExecuteNOT(int opcode, std::string &description, int trace);
  int ExecuteOR(int opcode, std::string &description, int trace);
  int ExecuteORI(int opcode, std::string &description, int trace);
  int ExecuteORItoCCR(int opcode, std::string &description, int trace);
  int ExecuteORItoSR(int opcode, std::string &description, int trace);
  int ExecutePEA(int opcode, std::string &description, int trace);
  int ExecuteRESET(int opcode, std::string &description, int trace);
  int ExecuteROL(int opcode, std::string &description, int trace);
  int ExecuteROR(int opcode, std::string &description, int trace);
  int ExecuteROXL(int opcode, std::string &description, int trace);
  int ExecuteROXR(int opcode, std::string &description, int trace);
  int ExecuteRTE(int opcode, std::string &description, int trace);
  // RTD: return and deallocate
  int ExecuteRTD(int opcode, std::string &description, int trace);
  int ExecuteRTR(int opcode, std::string &description, int trace);
  int ExecuteRTS(int opcode, std::string &description, int trace);
  int ExecuteSBCD(int opcode, std::string &description, int trace);
  int ExecuteSTOP(int opcode, std::string &description, int trace);
  int ExecuteSUB(int opcode, std::string &description, int trace);
  int ExecuteSUBA(int opcode, std::string &description, int trace);
  int ExecuteSUBI(int opcode, std::string &description, int trace);
  int ExecuteSUBQ(int opcode, std::string &description, int trace);
  int ExecuteSUBX(int opcode, std::string &description, int trace);
  int ExecuteSWAP(int opcode, std::string &description, int trace);
  int ExecuteScc(int opcode, std::string &description, int trace);
  int ExecuteTAS(int opcode, std::string &description, int trace);
  int ExecuteTRAP(int opcode, std::string &description, int trace);
  int ExecuteTRAPV(int opcode, std::string &description, int trace);
  int ExecuteTST(int opcode, std::string &description, int trace);
  int ExecuteUNLK(int opcode, std::string &description, int trace);
  int ExecuteInvalid(int opcode, std::string &description, int trace);

  int ExecuteBusError(int opcode, std::string &description, int trace);
  int ExecuteAddressError(int opcode, std::string &description, int trace);

  // Helpful routines for executing instructions
  int ComputeEffectiveAddress(unsigned long &address, int &in_register,
                              std::string &description, int mode_register, int size,
                              int trace);

  int Peek(unsigned long address, unsigned int &value, int size);
  int Poke(unsigned long address, unsigned int value, int size);

  unsigned int SignExtend(unsigned int value, int size);

  void SetConditionCodes(unsigned int src, unsigned int dest,
                         unsigned int result, int size, int operation,
                         int mask);

  void ClearConditionCodes(int mask);

  int CheckConditionCodes(int code, std::string &mnemonic, int trace);

  void SetRegister(int register_number, unsigned int value, int size);

  int ProcessException(int vector);

  long my_interrupt;
  BasicDevice *my_device;
};

#endif  // M68K_SIM68360_CPU32_H_
