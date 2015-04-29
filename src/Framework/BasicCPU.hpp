//
// Abstract base class for all microprocessors
//

#ifndef FRAMEWORK_BASICCPU_HPP_
#define FRAMEWORK_BASICCPU_HPP_

#include <string>
#include <vector>

#include "Framework/Types.hpp"
#include "Framework/AddressSpace.hpp"
#include "Framework/Event.hpp"

class BasicCPU;
class BasicDevice;
class RegisterInformationList;
class StatisticalInformationList;
class AddressSpace;

class BasicCPU {
public:
  BasicCPU(const std::string &name,
           int granularity,
           std::vector<AddressSpace *> &addressSpaces,
           const std::string &traceRecordFormat,
           const std::string &defaultTraceRecordEntries);
  virtual ~BasicCPU();

  // Returns name of the microprocessor.
  const std::string &Name() const { return myName; }

  // Returns the granularity of the microprocessor.
  unsigned int Granularity() const { return myGranularity; }

  // Returns a reference to my event handler.
  EventHandler &eventHandler() { return myEventHandler; };

  // Returns the number of address spaces used by the processor.
  size_t NumberOfAddressSpaces() const { return myAddressSpaces.size(); }

  // Returns the execution trace record format.
  const std::string &ExecutionTraceRecord() const { return myExecutionTraceRecord; }

  // Returns the default execution trace entries.
  const std::string &DefaultExecutionTraceEntries() const {
    return myDefaultExecutionTraceEntries;
  }

  // Returns the indexed address space object.
  AddressSpace &addressSpace(size_t addressSpace) {
    return *myAddressSpaces[addressSpace];
  }

  // Executes the next instruction. Returns an error message or the empty string.
  virtual std::string ExecuteInstruction(std::string &traceRecord, bool trace) = 0;

  // Handles an interrupt request from a device.
  virtual void InterruptRequest(BasicDevice *device, int level) = 0;

  // Performs a system reset.
  virtual void Reset() = 0;

  // Returns value of the program counter register.
  virtual Address ValueOfProgramCounter() = 0;

  // Sets named register to the given hexidecimal value.
  virtual void SetRegister(const std::string &name, const std::string &hexValue) = 0;

  // Clears the CPU's Statistics.
  virtual void ClearStatistics() = 0;

  // Appends all of the CPU's registers to the RegisterInformationList object.
  virtual void BuildRegisterInformationList(RegisterInformationList &) = 0;

  // Appends all of the CPU's stats to the StatisticalInformationList object.
  virtual void
  BuildStatisticalInformationList(StatisticalInformationList &) = 0;

protected:
  // Array of address space objects.
  std::vector<AddressSpace *> myAddressSpaces;

  // My event handler.
  EventHandler myEventHandler;

private:
  // My name.
  const std::string myName;

  // CPU address granularity in bytes.
  const unsigned int myGranularity;

  // Trace record format used by the ExecuteInstruction member function.
  std::string myExecutionTraceRecord;

  // Default fields of the trace record that should be displayed by UI.
  std::string myDefaultExecutionTraceEntries;
};

#endif
