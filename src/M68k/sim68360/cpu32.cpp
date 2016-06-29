#include "Framework/AddressSpace.hpp"
#include "Framework/BasicDevice.hpp"
#include "Framework/RegInfo.hpp"
#include "Framework/Tools.hpp"
#include "M68k/sim68360/cpu32.hpp"

// Array of information about each register
cpu32::RegisterData cpu32::ourRegisterData[] = {
    {"D0", 0xffffffff, "Data Register 0"},
    {"D1", 0xffffffff, "Data Register 1"},
    {"D2", 0xffffffff, "Data Register 2"},
    {"D3", 0xffffffff, "Data Register 3"},
    {"D4", 0xffffffff, "Data Register 4"},
    {"D5", 0xffffffff, "Data Register 5"},
    {"D6", 0xffffffff, "Data Register 6"},
    {"D7", 0xffffffff, "Data Register 7"},
    {"A0", 0xffffffff, "Address Register 0"},
    {"A1", 0xffffffff, "Address Register 1"},
    {"A2", 0xffffffff, "Address Register 2"},
    {"A3", 0xffffffff, "Address Register 3"},
    {"A4", 0xffffffff, "Address Register 4"},
    {"A5", 0xffffffff, "Address Register 5"},
    {"A6", 0xffffffff, "Address Register 6"},
    {"A7", 0xffffffff, "Address Register 7 (User Stack Pointer)"},
    {"A7'", 0xffffffff, "Address Register 7 (Supervisor Stack Pointer)"},
    {"PC", 0xffffffff, "Program Counter"},
    {"SR", 0x0000ffff, "Status Register: T-S--III---XNZVC\n  T: Trace Mode\n  "
                       "S: Supervisory Mode\n  I: Interrupt Mask Level\n  X: "
                       "Extend\n  N: Negative\n  Z: Zero\n  V: Overflow\n  C: "
                       "Carry"},
    {"VBR", 0xffffffff, "Vector Base Register"},
    {"SFC", 0x00000007, "Address space identification"},
    {"DFC", 0x00000007, "Address space identification"}};

cpu32::cpu32()
    : BasicCPU("68360", 1, *(new std::vector<AddressSpace *>),
               "{InstructionAddress 8} {Mnemonic 35}",
               "InstructionAddress Mnemonic"),
      myNumberOfRegisters(22), C_FLAG(0x0001), // SR flags
      V_FLAG(0x0002), Z_FLAG(0x0004), N_FLAG(0x0008), X_FLAG(0x0010),
      I0_FLAG(0x0100), I1_FLAG(0x0200), I2_FLAG(0x0400), S_FLAG(0x2000),
      T_FLAG(0x8000), D0_INDEX(0), // Register Indices
      A0_INDEX(8), USP_INDEX(15), SSP_INDEX(16), PC_INDEX(17), SR_INDEX(18),
      VBR_INDEX(19), SFC_INDEX(20), DFC_INDEX(21),
      EXECUTE_OK(0), // Execution return codes
      EXECUTE_PRIVILEGED_OK(1), EXECUTE_BUS_ERROR(2), EXECUTE_ADDRESS_ERROR(3),
      EXECUTE_ILLEGAL_INSTRUCTION(4), NORMAL_STATE(0), // Processor states
      HALT_STATE(1), STOP_STATE(2), BREAK_STATE(3) {
  // Create my single address space object
  myAddressSpaces.push_back(new AddressSpace(0x0fffffff));

  // Build the decode cache if necessary
  if (ourDecodeCacheTable == 0) {
    // Allocate memory for the decode cache table
    ourDecodeCacheTable = new ExecutionPointer[65536];

    // Set all cache entries to invalid (NULL)
    for (int t = 0; t < 65536; ++t)
      ourDecodeCacheTable[t] = 0;
  }

  // Allocate array for register values
  register_value = new Register[myNumberOfRegisters];

  // Clear the registers
  for (int t = 0; t < myNumberOfRegisters; ++t)
    register_value[t] = 0;

  // Reset the system
  Reset();
}

cpu32::~cpu32() {
  // Free the register value array
  delete[] register_value;

  // Free the address space object
  delete myAddressSpaces[0];
}

void cpu32::Reset() {
  Register pc, ssp;

  // Reset all of the device's attached to the processor
  myAddressSpaces[0]->Reset();

  // Reset the VBR
  SetRegister(VBR_INDEX, 0x0, LONG);

  // Reset DFC and SFC
  SetRegister(DFC_INDEX, 5, LONG);
  SetRegister(SFC_INDEX, 5, LONG);

  // Set the Status register to its reset value
  register_value[SR_INDEX] = S_FLAG | I0_FLAG | I1_FLAG | I2_FLAG;

  // Fetch the Supervisor Stack Pointer from location $00000000
  if (Peek(0x00000000, ssp, LONG) != EXECUTE_OK)
    SetRegister(SSP_INDEX, 0, LONG);
  else
    SetRegister(SSP_INDEX, ssp, LONG);

  // Fetch the Program Counter from location $00000004
  if (Peek(0x00000004, pc, LONG) != EXECUTE_OK)
    SetRegister(PC_INDEX, 0, LONG);
  else
    SetRegister(PC_INDEX, pc, LONG);

  // Put the processor in normal instrution execution mode
  myState = NORMAL_STATE;
}

Address cpu32::ValueOfProgramCounter() {
  return register_value[PC_INDEX];
}

void cpu32::BuildStatisticalInformationList(StatisticalInformationList &) {
  // We're not keeping up with any stupid statistics :-}
}

void cpu32::SetRegister(const std::string &name, const std::string &hexValue) {
  for (int t = 0; t < myNumberOfRegisters; ++t) {
    if (name == ourRegisterData[t].name) {
      register_value[t] = StringToInt(hexValue) & ourRegisterData[t].mask;
      break;
    }
  }
}

void cpu32::BuildRegisterInformationList(RegisterInformationList &lst) {
  for (unsigned int t = 0; t < (unsigned int)myNumberOfRegisters; ++t) {
    std::string value;

    if (t != SR_INDEX)
      value = IntToString(register_value[t] & ourRegisterData[t].mask, 8);
    else
      value = IntToString(register_value[t] & ourRegisterData[t].mask, 4);

    lst.Append(ourRegisterData[t].name, value, ourRegisterData[t].description);
  }
}

std::string cpu32::ExecuteInstruction(std::string &traceRecord, bool tracing) {
  unsigned int opcode;
  int status;

  // Add instruction address to the trace record
  if (tracing) {
    traceRecord = "{InstructionAddress ";
    traceRecord += IntToString(register_value[PC_INDEX], 8);
    traceRecord += "} ";
  }

  // Make sure the CPU hasn't been halted
  if (myState != HALT_STATE) {
    bool serviceFlag;

    // Service any pending interrupts
    status = ServiceInterrupts(serviceFlag);

    // Only execute an instruction if we didn't service an interrupt
    if ((!serviceFlag) && (status == EXECUTE_OK)) {
      // Make sure the CPU isn't stopped waiting for exceptions
      if (myState != STOP_STATE) {
        // Fetch the next instruction
        status = Peek(register_value[PC_INDEX], opcode, WORD);
        if (status == EXECUTE_OK) {
          register_value[PC_INDEX] += 2;

          // Execute the instruction
          ExecutionPointer executeMethod = DecodeInstruction(opcode);
          status = (this->*executeMethod)(opcode, traceRecord, tracing);

          // If the last instruction was not priviledged then check for trace
          if ((status == EXECUTE_OK) && (register_value[SR_INDEX] & T_FLAG))
            status = ProcessException(9);
        }
      } else {
        if (tracing)
          traceRecord += "{Mnemonic {CPU is stopped}} ";
      }
    }

    if (status == EXECUTE_BUS_ERROR) {
      if (ExecuteBusError(opcode, traceRecord, tracing) != EXECUTE_OK) {
        // Oh, no the cpu has fallen and it can't get up!
        myState = HALT_STATE;
        if (tracing)
          traceRecord += "{Mnemonic {Double Bus/Address Error CPU halted}} ";
      }
    } else if (status == EXECUTE_ADDRESS_ERROR) {
      if (ExecuteAddressError(opcode, traceRecord, tracing) != EXECUTE_OK) {
        // Now, where's that reset button???
        myState = HALT_STATE;
        if (tracing)
          traceRecord += "{Mnemonic {Double Bus/Address Error CPU halted}} ";
      }
    }
  } else {
    if (tracing)
      traceRecord += "{Mnemonic {CPU has halted}} ";
  }

  // Check the event list - only if not in step by step execution
  if (!tracing)
    myEventHandler.Check();

  // Signal if the processor is in a wierd state
  if (myState == HALT_STATE) {
    return "CPU has halted";
  } else if (myState == BREAK_STATE) {
    myState = NORMAL_STATE;
    if (tracing)
      return "";
    else
      return "BREAK instruction";
  }

  return "";
}

void cpu32::InterruptRequest(BasicDevice *device, int level) {
  // The 68000 has seven levels of interrupts
  if (level > 7)
    level = 7;
  else if (level < 1)
    level = 1;

  pending_interrupts.push(PendingInterrupt(level, device));
}

int cpu32::ServiceInterrupts(bool &serviceFlag) {
  serviceFlag = false;

  // If there are no pending interupts, return normally.
  if (pending_interrupts.empty()) {
    return EXECUTE_OK;
  }

  const PendingInterrupt &interrupt = pending_interrupts.top();

  // Also return normally if any of the currently pending interrupts
  // are masked.  Note that a check against the top of the queue is
  // sufficient, as the top entry has the highest level.
  const int interrupt_mask = (register_value[SR_INDEX] & 0x0700) >> 8;
  if (interrupt.level < interrupt_mask && interrupt.level != 7) {
    return EXECUTE_OK;
  }

  // Put the processor into normal state if it's stopped
  if (myState == STOP_STATE)
    myState = NORMAL_STATE;

  // Save a copy of the current SR so it can be stacked for entry
  // to the interrupt service subroutine
  Register tmp_sr = register_value[SR_INDEX];

  // Set the Interrupt Mask in SR
  register_value[SR_INDEX] &= 0x0000f8ff;
  register_value[SR_INDEX] |= (interrupt.level << 8);

  // Change to Supervisor mode and clear the Trace mode
  register_value[SR_INDEX] |= S_FLAG;
  register_value[SR_INDEX] &= ~T_FLAG;

  // Interrupt has occured so push the PC and the SR
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 4, LONG);
  int status = Poke(register_value[SSP_INDEX], register_value[PC_INDEX], LONG);
  if (status != EXECUTE_OK)
    return status;

  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 2, LONG);
  status = Poke(register_value[SSP_INDEX], tmp_sr, WORD);
  if (status != EXECUTE_OK)
    return status;

  // Get the vector number
  long vector = interrupt.device->InterruptAcknowledge(interrupt.level);
  if (vector == AUTOVECTOR_INTERRUPT)
    vector = 24 + interrupt.level;
  else if (vector == SPURIOUS_INTERRUPT)
    vector = 24;

  // Get the interrupt service routine's address
  Address service_address;
  status = Peek(vector * 4, service_address, LONG);
  if (status != EXECUTE_OK)
    return (status);

  // Change the program counter to the service routine's address
  SetRegister(PC_INDEX, service_address, LONG);

  // Indicate that an interrupt was serviced and remove it from
  // the queue of pending interrupts
  serviceFlag = true;
  pending_interrupts.pop();

  return EXECUTE_OK;
}
