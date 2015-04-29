///////////////////////////////////////////////////////////////////////////////
//
// m68000.cxx 
//
// Motorola 68000 microprocessor class
//
// Sim68000 "Motorola 68000 Simulator"
// Copyright (c) 1993
// By: Bradford W. Mott
// October 31,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: m68000.cxx,v 1.1 1996/08/02 14:59:18 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "Tools.hxx"
#include "m68000.hxx"
#include "BasicDevice.hxx"
#include "RegInfo.hxx"
#include "AddressSpace.hxx"

// Array of information about each register
m68000::RegisterData m68000::ourRegisterData[] = {
  {"D0",  0xffffffff,  "Data Register 0"},
  {"D1",  0xffffffff,  "Data Register 1"},
  {"D2",  0xffffffff,  "Data Register 2"},
  {"D3",  0xffffffff,  "Data Register 3"},
  {"D4",  0xffffffff,  "Data Register 4"},
  {"D5",  0xffffffff,  "Data Register 5"},
  {"D6",  0xffffffff,  "Data Register 6"},
  {"D7",  0xffffffff,  "Data Register 7"},
  {"A0",  0xffffffff,  "Address Register 0"},
  {"A1",  0xffffffff,  "Address Register 1"},
  {"A2",  0xffffffff,  "Address Register 2"},
  {"A3",  0xffffffff,  "Address Register 3"},
  {"A4",  0xffffffff,  "Address Register 4"},
  {"A5",  0xffffffff,  "Address Register 5"},
  {"A6",  0xffffffff,  "Address Register 6"},
  {"A7",  0xffffffff,  "Address Register 7 (User Stack Pointer)"},
  {"A7'" ,0xffffffff,  "Address Register 7 (Supervisor Stack Pointer)"},
  {"PC",  0xffffffff,  "Program Counter"},
  {"SR",  0x0000ffff,  "Status Register: T-S--III---XNZVC\n  T: Trace Mode\n  S: Supervisory Mode\n  I: Interrupt Mask Level\n  X: Extend\n  N: Negative\n  Z: Zero\n  V: Overflow\n  C: Carry"}
}; 

///////////////////////////////////////////////////////////////////////////////
// The m68000 Class constructor
///////////////////////////////////////////////////////////////////////////////
m68000::m68000()
    : BasicCPU("68000", 1, *(new vector<AddressSpace*>),
          "{InstructionAddress 8} {Mnemonic 35}",
          "InstructionAddress Mnemonic"),     
      myNumberOfRegisters(19),
      C_FLAG(0x0001),                           // SR flags    
      V_FLAG(0x0002),
      Z_FLAG(0x0004),
      N_FLAG(0x0008),
      X_FLAG(0x0010),
      I0_FLAG(0x0100),
      I1_FLAG(0x0200),
      I2_FLAG(0x0400),
      S_FLAG(0x2000),
      T_FLAG(0x8000),
      D0_INDEX(0),                              // Register Indices
      A0_INDEX(8),
      USP_INDEX(15),
      SSP_INDEX(16),
      PC_INDEX(17),
      SR_INDEX(18),
      EXECUTE_OK(0),                            // Execution return codes
      EXECUTE_PRIVILEGED_OK(1),
      EXECUTE_BUS_ERROR(2),
      EXECUTE_ADDRESS_ERROR(3),
      EXECUTE_ILLEGAL_INSTRUCTION(4),
      NORMAL_STATE(0),                          // Processor states
      HALT_STATE(1),
      STOP_STATE(2),
      BREAK_STATE(3)
{
  // Create my single address space object
  myAddressSpaces.push_back(new AddressSpace(0x00ffffff));

  // Build the decode cache if necessary
  if(ourDecodeCacheTable == 0)
  {
    // Allocate memory for the decode cache table
    ourDecodeCacheTable = new ExecutionPointer[65536];

    // Set all cache entries to invalid (NULL)
    for(int t = 0;t < 65536; ++t)
      ourDecodeCacheTable[t] = 0;
  }

  // Allocate array for register values
  register_value = new unsigned long[myNumberOfRegisters];

  // Clear the registers
  for(int t = 0; t < myNumberOfRegisters; ++t)
    register_value[t] = 0;

  // Reset the system
  Reset();

  my_interrupt = -1;
}

///////////////////////////////////////////////////////////////////////////////
// The m68000 Class destructor
///////////////////////////////////////////////////////////////////////////////
m68000::~m68000()
{
  // Free the register value array
  delete[] register_value;

  // Free the address space object
  delete myAddressSpaces[0];

  // Free the address space vector
  delete &myAddressSpaces;
}

///////////////////////////////////////////////////////////////////////////////
// Preform a system reset
///////////////////////////////////////////////////////////////////////////////
void m68000::Reset()
{
  unsigned int pc, ssp;

  // Reset all of the device's attached to the processor
  myAddressSpaces[0]->Reset();

  // Set the Status register to its reset value
  register_value[SR_INDEX] = S_FLAG | I0_FLAG | I1_FLAG | I2_FLAG;

  // Fetch the Supervisor Stack Pointer from location $00000000
  if(Peek(0x00000000, ssp, LONG) != EXECUTE_OK)
    SetRegister(SSP_INDEX, 0, LONG);
  else
    SetRegister(SSP_INDEX, ssp, LONG);

  // Fetch the Program Counter from location $00000004
  if(Peek(0x00000004, pc, LONG) != EXECUTE_OK)
    SetRegister(PC_INDEX, 0, LONG);
  else
    SetRegister(PC_INDEX, pc, LONG);

  // Put the processor in normal instrution execution mode
  myState = NORMAL_STATE;
}

///////////////////////////////////////////////////////////////////////////////
// Answer the value in the program counter
///////////////////////////////////////////////////////////////////////////////
unsigned long m68000::ValueOfProgramCounter()
{
  return register_value[PC_INDEX];
}

///////////////////////////////////////////////////////////////////////////////
// Build the statistics list for the StatisticalInformationList object
///////////////////////////////////////////////////////////////////////////////
void m68000::BuildStatisticalInformationList(StatisticalInformationList&)
{
  // We're not keeping up with any stupid statistics :-}
}

///////////////////////////////////////////////////////////////////////////////
// Set the named register to the given value
///////////////////////////////////////////////////////////////////////////////
void m68000::SetRegister(const string& name, const string& hexValue)
{
  for(int t = 0; t < myNumberOfRegisters; ++t) 
  {
    if(name == ourRegisterData[t].name)
    {
      register_value[t] = StringToInt(hexValue) & ourRegisterData[t].mask;
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Append all of the CPU's registers to the RegisterInformationList object
///////////////////////////////////////////////////////////////////////////////
void m68000::BuildRegisterInformationList(RegisterInformationList& lst)
{
  for(unsigned int t = 0; t < (unsigned int)myNumberOfRegisters; ++t)
  {
    string value;

    if(t != SR_INDEX)
      value = IntToString(register_value[t] & ourRegisterData[t].mask, 8);
    else
      value = IntToString(register_value[t] & ourRegisterData[t].mask, 4);

    lst.Append(ourRegisterData[t].name, value, ourRegisterData[t].description);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Execute the next instruction
///////////////////////////////////////////////////////////////////////////////
const char* m68000::ExecuteInstruction(string& traceRecord, bool tracing)
{
  unsigned int opcode;
  int status;

  // Add instruction address to the trace record
  if(tracing)
  {
    traceRecord = "{InstructionAddress ";
    traceRecord += IntToString(register_value[PC_INDEX], 8);
    traceRecord += "} ";
  }

  // Make sure the CPU hasn't been halted
  if(myState != HALT_STATE)
  {
    bool serviceFlag;

    // Service any pending interrupts
    status = ServiceInterrupts(serviceFlag);

    // Only execute an instruction if we didn't service an interrupt
    if((!serviceFlag) && (status == EXECUTE_OK))
    { 
      // Make sure the CPU isn't stopped waiting for exceptions
      if(myState != STOP_STATE)
      {
        // Fetch the next instruction
        status = Peek(register_value[PC_INDEX], opcode, WORD);
        if(status == EXECUTE_OK)
        {
          register_value[PC_INDEX] += 2;
  
          // Execute the instruction
          ExecutionPointer executeMethod = DecodeInstruction(opcode); 
          status = (this->*executeMethod)(opcode, traceRecord, tracing);

          // If the last instruction was not priviledged then check for trace
          if((status == EXECUTE_OK) && (register_value[SR_INDEX] & T_FLAG))
            status = ProcessException(9);
        }
      }
      else
      {
        if(tracing)
          traceRecord += "{Mnemonic {CPU is stopped}} ";
      }
    }

    if(status == EXECUTE_BUS_ERROR)
    {
      if(ExecuteBusError(opcode, traceRecord, tracing) != EXECUTE_OK)
      {
        // Oh, no the cpu has fallen and it can't get up!
        myState = HALT_STATE;
        if(tracing)
          traceRecord += "{Mnemonic {Double Bus/Address Error CPU halted}} ";
      }
    }
    else if(status == EXECUTE_ADDRESS_ERROR)
    {
      if(ExecuteAddressError(opcode, traceRecord, tracing) != EXECUTE_OK)
      {
        // Now, where's that reset button???
        myState = HALT_STATE;
        if(tracing)
          traceRecord += "{Mnemonic {Double Bus/Address Error CPU halted}} ";
      }
    }
  }
  else
  {
    if(tracing)
      traceRecord += "{Mnemonic {CPU has halted}} ";
  }

  // Check the event list
  myEventHandler.Check();

  // Signal if the processor is in a wierd state
  if(myState == HALT_STATE)
  {
    return "CPU has halted";
  }
  else if(myState == BREAK_STATE)
  {
    myState = NORMAL_STATE;
    if(tracing)
      return 0;
    else
      return "BREAK instruction";
  }
  else
  {
    return 0;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Handle an interrupt request from a device
///////////////////////////////////////////////////////////////////////////////
void m68000::InterruptRequest(BasicDevice* device, int level)
{
  // The 68000 has seven levels of interrupts
  if(level > 7)
    level = 7;
  else if(level < 1)
    level = 1;

  // Get the interrupt mask
  int interruptMask = (register_value[SR_INDEX] & 0x0700) >> 8;

  if((level > interruptMask) || (level == 7))
  {
    my_interrupt = level;
    my_device = device;
  }
  else
  {
    device->InterruptAcknowledge(1); 
  }
}

///////////////////////////////////////////////////////////////////////////////
// Service pending interrupts, serviceFlag set true iff something serviced
///////////////////////////////////////////////////////////////////////////////
int m68000::ServiceInterrupts(bool& serviceFlag)
{
  if(my_interrupt != -1)
  {
    // Indicate that I had to service an interrupt
    serviceFlag = true;

    int level = my_interrupt;
    int status;

    my_interrupt = -1;

    // Put the processor into normal state if it's stopped
    if(myState == STOP_STATE)
      myState = NORMAL_STATE;

    // Copy the SR to a temp
    unsigned long tmp_sr = register_value[SR_INDEX];

    // Set the Interrupt Mask in SR
    register_value[SR_INDEX] &= 0x0000f8ff;
    register_value[SR_INDEX] |= (level << 8);

    // Change to Supervisor mode and clear the Trace mode
    register_value[SR_INDEX] |= S_FLAG;
    register_value[SR_INDEX] &= ~T_FLAG;

    // Interrupt has occured so push the PC and the SR
    SetRegister(SSP_INDEX, register_value[SSP_INDEX]-4, LONG);
    status = Poke(register_value[SSP_INDEX], register_value[PC_INDEX], LONG);
    if(status != EXECUTE_OK)
      return status;

    SetRegister(SSP_INDEX, register_value[SSP_INDEX]-2, LONG);
    status = Poke(register_value[SSP_INDEX], tmp_sr, WORD);
    if(status != EXECUTE_OK)
      return status;

    // Get the vector number
    long vector = my_device->InterruptAcknowledge(level);
    if(vector == AUTOVECTOR_INTERRUPT)
      vector = 24 + level;
    else if(vector == SPURIOUS_INTERRUPT)
      vector = 24;

    // Get the interrupt service routine's address
    unsigned int service_address;
    status = Peek(vector * 4, service_address , LONG);
    if(status != EXECUTE_OK)
      return(status); 

    // Change the program counter to the service routine's address
    SetRegister(PC_INDEX, service_address, LONG);
  }
  else
  {
    // Indicate that I didn't have to service an interrupt
    serviceFlag = false;
  }
  return EXECUTE_OK;
}

