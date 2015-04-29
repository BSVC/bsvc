//
// This is the user interface command class.  It handles all of the
// command's issue by the user interface.
//

#ifndef FRAMEWORK_INTERFACE_HPP_
#define FRAMEWORK_INTERFACE_HPP_

#include <iostream>

class BasicCPU;
class BasicDeviceRegistry;
class BasicLoader;
class BreakpointList;

class Interface {
public:
  Interface(BasicCPU &cpu, BasicDeviceRegistry &registry, BasicLoader &loader);

  // Command loop
  void CommandLoop();

private:
  // Structure for the interface's command table
  struct CommandTable {
    std::string name;
    void (Interface::*mfp)(const std::string &);
  };

  // Table of commands.
  static CommandTable ourCommandTable[];

  // Indicates the number of commands in the command table.
  const unsigned int myNumberOfCommands;

  // Reference to the CPU I'm managing.
  BasicCPU &myCPU;

  // Reference to the device registry.
  BasicDeviceRegistry &myDeviceRegistry;

  // Reference to the loader.
  BasicLoader &myLoader;

  // Reference to the input stream used to get information from the UI.
  std::istream &myInputStream;

  // Reference to the output stream used to send information to the UI.
  std::ostream &myOutputStream;

  // Breakpoint list to manage the breakpoints.
  BreakpointList &myBreakpointList;

  // Execute the given command.
  void ExecuteCommand(const std::string &command);

  // Member funtion for each of the commands.
  void AddBreakpoint(const std::string &args);
  void AttachDevice(const std::string &args);
  void ClearStatistics(const std::string &args);
  void DeleteBreakpoint(const std::string &args);
  void DetachDevice(const std::string &args);
  void FillMemoryBlock(const std::string &args);
  void ListAttachedDevices(const std::string &args);
  void ListBreakpoints(const std::string &args);
  void ListDevices(const std::string &args);
  void ListDeviceScript(const std::string &args);
  void ListExecutionTraceRecord(const std::string &args);
  void ListDefaultExecutionTraceEntries(const std::string &args);
  void ListGranularity(const std::string &args);
  void ListMemory(const std::string &args);
  void ListMaximumAddress(const std::string &args);
  void ListNumberOfAddressSpaces(const std::string &args);
  void ListRegisters(const std::string &args);
  void ListRegisterValue(const std::string &args);
  void ListRegisterDescription(const std::string &args);
  void ListStatistics(const std::string &args);
  void LoadProgram(const std::string &args);
  void ProgramCounterValue(const std::string &args);
  void Reset(const std::string &args);
  void Run(const std::string &args);
  void SetRegister(const std::string &args);
  void SetMemory(const std::string &args);
  void Step(const std::string &args);
};

#endif  // FRAMEWORK_INTERFACE_HPP_
