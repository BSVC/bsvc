#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

#include "Framework/Interface.hpp"
#include "Framework/BasicCPU.hpp"
#include "Framework/AddressSpace.hpp"
#include "Framework/BasicDeviceRegistry.hpp"
#include "Framework/BasicLoader.hpp"
#include "Framework/BreakpointList.hpp"
#include "Framework/StatInfo.hpp"
#include "Framework/RegInfo.hpp"
#include "Framework/Tools.hpp"

// Simulator's main loop: gets a command from the UI, parses and executes it.
void Interface::CommandLoop() {
  myOutputStream << "BSVC Simulator" << std::endl;
  for (;;) {
    std::string command;
    myOutputStream << "Ready!" << std::endl;
    do {
      std::getline(myInputStream, command);
    } while (command == "NOP");
    if (command == "Exit") {
      break;
    }
    // For some version of iostream we have to have a little padding :-(
    command += " ";
    ExecuteCommand(command);
  }
}

// Parse and execute the given command
void Interface::ExecuteCommand(const std::string &command) {
  for (size_t t = 0; t < myNumberOfCommands; ++t) {
    if (command.find(ourCommandTable[t].name) == 0) {
      std::string args = command.substr(ourCommandTable[t].name.length() + 1);
      (this->*ourCommandTable[t].mfp)(args);
      return;
    }
  }
  myOutputStream << "ERROR: Unknown command!" << std::endl;
}

// The user interface command table
Interface::CommandTable Interface::ourCommandTable[] = {
    {"AddBreakpoint", &Interface::AddBreakpoint},
    {"AttachDevice", &Interface::AttachDevice},
    {"ClearStatistics", &Interface::ClearStatistics},
    {"DetachDevice", &Interface::DetachDevice},
    {"DeleteBreakpoint", &Interface::DeleteBreakpoint},
    {"FillMemoryBlock", &Interface::FillMemoryBlock},
    {"ListAttachedDevices", &Interface::ListAttachedDevices},
    {"ListBreakpoints", &Interface::ListBreakpoints},
    {"ListDevices", &Interface::ListDevices},
    {"ListDeviceScript", &Interface::ListDeviceScript},
    {"ListExecutionTraceRecord", &Interface::ListExecutionTraceRecord},
    {"ListDefaultExecutionTraceEntries",
     &Interface::ListDefaultExecutionTraceEntries},
    {"ListGranularity", &Interface::ListGranularity},
    {"ListMemory", &Interface::ListMemory},
    {"ListMaximumAddress", &Interface::ListMaximumAddress},
    {"ListNumberOfAddressSpaces", &Interface::ListNumberOfAddressSpaces},
    {"ListRegisters", &Interface::ListRegisters},
    {"ListRegisterValue", &Interface::ListRegisterValue},
    {"ListRegisterDescription", &Interface::ListRegisterDescription},
    {"ListStatistics", &Interface::ListStatistics},
    {"LoadProgram", &Interface::LoadProgram},
    {"ProgramCounterValue", &Interface::ProgramCounterValue},
    {"Reset", &Interface::Reset},
    {"Run", &Interface::Run},
    {"SetMemory", &Interface::SetMemory},
    {"SetRegister", &Interface::SetRegister},
    {"Step", &Interface::Step}};

Interface::Interface(BasicCPU &cpu, BasicDeviceRegistry &registry,
                     BasicLoader &loader)
    : myNumberOfCommands(sizeof(ourCommandTable) / sizeof(CommandTable)),
      myCPU(cpu), myDeviceRegistry(registry), myLoader(loader),
      myInputStream(std::cin), myOutputStream(std::cout),
      myBreakpointList(*new BreakpointList) {}

// Prints the value of the program counter.
void Interface::ProgramCounterValue(const std::string &) {
  myOutputStream << std::hex << myCPU.ValueOfProgramCounter() << std::endl;
}

// Lists the processor's execution trace record.
void Interface::ListExecutionTraceRecord(const std::string &) {
  myOutputStream << myCPU.ExecutionTraceRecord() << std::endl;
}

// Lists the processor's default execution trace entries to display.
void Interface::ListDefaultExecutionTraceEntries(const std::string &) {
  myOutputStream << myCPU.DefaultExecutionTraceEntries() << std::endl;
}

// Clears the cpu's statistics.
void Interface::ClearStatistics(const std::string &) { myCPU.ClearStatistics(); }

// Lists the cpu's statistics.
void Interface::ListStatistics(const std::string &) {
  StatisticalInformationList list(myCPU);
  for (size_t t = 0; t < list.NumberOfElements(); ++t) {
    std::string info;
    list.Element(t, info);
    myOutputStream << info.Statistic() << std::endl;
  }
}

// Lists the CPU's registers.
void Interface::ListRegisters(const std::string &) {
  RegisterInformationList list(myCPU);
  size_t width = 0;

  // Find the longest register name.
  for (size_t k = 0; k < list.NumberOfElements(); ++k) {
    RegisterInformation info;
    list.Element(k, info);
    width = std::max(width, info.Name().length());
  }

  // Send the register list to the user interface.
  for (size_t k = 0; k < list.NumberOfElements(); ++k) {
    RegisterInformation info;
    list.Element(k, info);
    myOutputStream << std::setw(width) << std::left << std::setfill(' ')
                   << info.Name() << " = " << info.HexValue() << std::endl;
  }
}

// Lists a register's description.
void Interface::ListRegisterDescription(const std::string &args) {
  RegisterInformationList list(myCPU);
  std::istringstream in(args);
  std::string name;
  in >> name;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  for (size_t k = 0; k < list.NumberOfElements(); ++k) {
    RegisterInformation info;
    list.Element(k, info);
    if (name == info.Name()) {
      myOutputStream << info.Description() << std::endl;
      return;
    }
  }
  myOutputStream << "ERROR: Invalid register name!" << std::endl;
}

// Sets one of the cpu's registers to the given value.
void Interface::SetRegister(const std::string &args) {
  std::istringstream in(args);
  RegisterInformationList list(myCPU);
  std::string name;
  std::string value;

  // Read the register name and value
  in >> name >> value;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  for (size_t k = 0; k < list.NumberOfElements(); ++k) {
    RegisterInformation info;

    list.Element(k, info);
    if (name == info.Name()) {
      myCPU.SetRegister(name, value);
      return;
    }
  }
  myOutputStream << "ERROR: Invalid register name!" << std::endl;
}

// Lists the value of one of the cpu's registers.
void Interface::ListRegisterValue(const std::string &args) {
  std::istringstream in(args);
  RegisterInformationList list(myCPU);
  std::string name;

  // Read the register name
  in >> name;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  for (size_t k = 0; k < list.NumberOfElements(); ++k) {
    RegisterInformation info;

    list.Element(k, info);
    if (name == info.Name()) {
      myOutputStream << info.HexValue() << std::endl;
      return;
    }
  }
  myOutputStream << "ERROR: Invalid register name!" << std::endl;
}

// Detaches a device from the simulator.
void Interface::DetachDevice(const std::string &args) {
  std::istringstream in(args);
  size_t addressSpace;
  size_t deviceIndex;

  in >> addressSpace >> deviceIndex;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  if (myCPU.NumberOfAddressSpaces() <= addressSpace) {
    myOutputStream << "ERROR: Invalid address space!" << std::endl;
    return;
  }
  if (!myCPU.addressSpace(addressSpace).DetachDevice(deviceIndex)) {
    myOutputStream << "ERROR: Couldn't detach device!" << std::endl;
  }
}

// Attaches a device to the simulator.
void Interface::AttachDevice(const std::string &args) {
  std::istringstream in(args);
  std::string name;
  std::string deviceArgs;
  size_t addressSpace;
  char c;

  in >> addressSpace >> name >> c;
  in.unsetf(std::ios::skipws);

  if (c != '{') {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  std::getline(in, deviceArgs, '}');
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  if (addressSpace >= myCPU.NumberOfAddressSpaces()) {
    myOutputStream << "ERROR: Invalid address space!" << std::endl;
    return;
  }

  BasicDevice *device;
  if (!myDeviceRegistry.Create(name, deviceArgs, myCPU, device)) {
    myOutputStream << "ERROR: Couldn't create the device!" << std::endl;
    return;
  }
  myCPU.addressSpace(addressSpace).AttachDevice(device);
}

// Adds a breakpoint.
void Interface::AddBreakpoint(const std::string &args) {
  std::istringstream in(args);
  Address address;

  in >> std::hex >> address;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  myBreakpointList.Add(address);
}

// Deletes a breakpoint.
void Interface::DeleteBreakpoint(const std::string &args) {
  std::istringstream in(args);
  Address address;

  in >> std::hex >> address;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  if (!myBreakpointList.Delete(address)) {
    myOutputStream << "ERROR: Couldn't delete breakpoint!" << std::endl;
  }
}

// Fills the memory block with the given value.
void Interface::FillMemoryBlock(const std::string &args) {
  std::istringstream in(args);
  unsigned int addressSpace;
  Address address;
  unsigned long length;
  std::string value;

  in >> addressSpace >> std::hex >> address >> std::hex >> length >> value;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  if (addressSpace >= myCPU.NumberOfAddressSpaces()) {
    myOutputStream << "ERROR: Invalid address space!" << std::endl;
    return;
  }

  if (address > myCPU.addressSpace(addressSpace).MaximumAddress()) {
    myOutputStream << "ERROR: Invalid address!" << std::endl;
    return;
  }

  for (size_t i = 0; i < length; ++i) {
    Address addr = (address + i) * myCPU.Granularity();
    for (size_t t = 0; t < myCPU.Granularity(); ++t) {
      myCPU.addressSpace(addressSpace)
          .Poke(addr + t, StringToInt(std::string(value, t * 2, 2)));
    }
  }
}

// Lists breakpoints.
void Interface::ListBreakpoints(const std::string &) {
  for (size_t t = 0; t < myBreakpointList.NumberOfBreakpoints(); ++t) {
    Address address = 0;
    myBreakpointList.GetBreakpoint(t, address);
    myOutputStream << IntToString(address, 8) << std::endl;
  }
}

// Lists the devices attached to the simulator.
void Interface::ListAttachedDevices(const std::string &args) {
  std::istringstream in(args);
  unsigned int addressSpace;

  in >> addressSpace;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  if (addressSpace >= myCPU.NumberOfAddressSpaces()) {
    myOutputStream << "ERROR: Invalid address space!" << std::endl;
    return;
  }

  size_t n = myCPU.addressSpace(addressSpace).NumberOfAttachedDevices();
  for (size_t k = 0; k < n; ++k) {
    AddressSpace::DeviceInformation info;
    myCPU.addressSpace(addressSpace).GetDeviceInformation(k, info);
    myOutputStream << info.name << " {" << info.arguments << "}" << std::endl;
  }
}

// Lists devices known to the simulator.
void Interface::ListDevices(const std::string &) {
  for (size_t t = 0; t < myDeviceRegistry.NumberOfDevices(); ++t) {
    DeviceInformation info;
    myDeviceRegistry.Information(t, info);
    myOutputStream << info.name << std::endl;
  }
}

// Lists the UI script for the named device.
void Interface::ListDeviceScript(const std::string &args) {
  std::istringstream in(args);
  std::string name;

  in >> name;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  for (size_t t = 0; t < myDeviceRegistry.NumberOfDevices(); ++t) {
    DeviceInformation info;
    myDeviceRegistry.Information(t, info);
    if (name == info.name) {
      myOutputStream << info.script << std::endl;
      return;
    }
  }
  myOutputStream << "ERROR: Invalid device name!" << std::endl;
}

// Lists a memory block.
void Interface::ListMemory(const std::string &args) {
  std::istringstream in(args);
  unsigned int addressSpace;
  Address address;
  unsigned int length;
  unsigned int wordsPerLine;
  std::string line;

  in >> addressSpace >> std::hex >> address >> length >> wordsPerLine;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  if (addressSpace >= myCPU.NumberOfAddressSpaces()) {
    myOutputStream << "ERROR: Invalid address space!" << std::endl;
    return;
  }
  size_t numberOfWords = 0;
  for (size_t t = 0; t < length; ++t) {
    for (size_t s = 0; s < myCPU.Granularity(); ++s) {
      Byte value;
      if (myCPU.addressSpace(addressSpace)
              .Peek((address + t) * myCPU.Granularity() + s, value)) {
        line += IntToString(value, 2);
      } else {
        line += "xx";
      }
    }
    ++numberOfWords;
    if (numberOfWords >= wordsPerLine) {
      myOutputStream << line << std::endl;
      numberOfWords = 0;
      line = "";
    } else {
      line += " ";
    }
  }
  if (!line.empty())
    myOutputStream << line << std::endl;
}

// Steps through some number of instructions.
void Interface::Step(const std::string &args) {
  std::istringstream in(args);
  int numberOfSteps;

  in >> numberOfSteps;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  for (int t = 0; t < numberOfSteps; ++t) {
    std::string traceRecord;
    const std::string &message = myCPU.ExecuteInstruction(traceRecord, true);
    if (!message.empty()) {
      myOutputStream << "{SimulatorMessage {" << message << "}}" << std::endl;
      break;
    }
    myOutputStream << traceRecord << std::endl;
  }
}

// Resets the CPU (which should also reset the devices).
void Interface::Reset(const std::string &) {
  myCPU.Reset();
}

// Perform the Run command
void Interface::Run(const std::string &args) {
  std::istringstream in(args);
  std::string name;
  char c;

  // The filename here is used to get around the problem with nonblocking
  // I/O in Tcl for windows.  When we're finished running we should
  // write something in this file.
  in >> c;
  in.unsetf(std::ios::skipws);

  if (c != '{') {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }

  std::getline(in, name, '}');

  // Run until something stops us.
  for (size_t steps = 0;; ++steps) {
    std::string traceRecord;
    const std::string &message = myCPU.ExecuteInstruction(traceRecord, false);
    if (!message.empty()) {
      if (message[0] == '.') {
        myOutputStream << message.substr(1) << std::flush;
      } else {
        myOutputStream << "Execution stopped: " << message << std::endl;
        break;
      }
    } else if (myBreakpointList.Check(myCPU.ValueOfProgramCounter())) {
      myOutputStream << "Execution stopped at a breakpoint!" << std::endl;
      break;
    }
    // Poll for input every 1024 steps
    else if ((steps & 0x03FF) == 0) {
      fd_set rfds;
      struct timeval tv;
      int retval;

      // We're going to poll STDIN to see if any input is available
      FD_ZERO(&rfds);
      FD_SET(0, &rfds);

      // Don't wait at all
      tv.tv_sec = 0;
      tv.tv_usec = 0;

      retval = select(1, &rfds, NULL, NULL, &tv);

      // Stop running if data is ready to be read
      if (retval == 1) {
        // Read the "StopRunning" Command
        std::string dummy;
        std::getline(myInputStream, dummy);

        myOutputStream << "Execution Interrupted!" << std::endl;
        break;
      }
    }
  }
}

// Lists the Maximum Address allow by the give address space.
void Interface::ListMaximumAddress(const std::string &args) {
  std::istringstream in(args);
  size_t addressSpace;

  in >> addressSpace;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  if (addressSpace >= myCPU.NumberOfAddressSpaces()) {
    myOutputStream << "ERROR: Invalid address space!" << std::endl;
    return;
  }
  myOutputStream << std::hex << myCPU.addressSpace(addressSpace).MaximumAddress()
                 << std::endl;
}

// Lists the number of address spaces for the CPU.
void Interface::ListNumberOfAddressSpaces(const std::string &) {
  myOutputStream << std::dec << myCPU.NumberOfAddressSpaces() << std::endl;
}

// Lists the granularity of the cpu.
void Interface::ListGranularity(const std::string &) {
  myOutputStream << std::dec << myCPU.Granularity() << std::endl;
}

// Sets a memory location to the given value.
void Interface::SetMemory(const std::string &args) {
  std::istringstream in(args);
  std::string value;
  size_t addressSpace;
  Address address;

  in >> addressSpace >> std::hex >> address >> value;

  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  if (addressSpace >= myCPU.NumberOfAddressSpaces()) {
    myOutputStream << "ERROR: Invalid address space!" << std::endl;
    return;
  }
  if (address > myCPU.addressSpace(addressSpace).MaximumAddress()) {
    myOutputStream << "ERROR: Invalid address!" << std::endl;
    return;
  }
  address *= myCPU.Granularity();
  for (size_t t = 0; t < myCPU.Granularity(); ++t) {
    myCPU.addressSpace(addressSpace)
        .Poke(address + t, StringToInt(std::string(value, t * 2, 2)));
  }
}

// Loads the named program into the address space.
void Interface::LoadProgram(const std::string &args) {
  std::istringstream in(args);
  size_t addressSpace;
  std::string name;
  char c;

  in >> addressSpace >> c;
  in.unsetf(std::ios::skipws);

  if (c != '{') {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  std::getline(in, name, '}');
  // Make sure we were able to read the arguments
  if (!in) {
    myOutputStream << "ERROR: Invalid arguments!" << std::endl;
    return;
  }
  myOutputStream << myLoader.Load(name, addressSpace) << std::endl;
}
