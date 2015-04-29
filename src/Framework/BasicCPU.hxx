///////////////////////////////////////////////////////////////////////////////
//
// BasicCPU.hxx
//
//   This is the abstract base class for all microprocessors
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// June 27,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: BasicCPU.hxx,v 1.1 1996/08/02 14:49:30 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef BASICCPU_HXX
#define BASICCPU_HXX

#include <string>
#include <vector>

#ifdef USE_STD
  using namespace std;
#endif

#include "AddressSpace.hxx"

class BasicCPU;
class BasicDevice;
class RegisterInformationList;
class StatisticalInformationList;
class AddressSpace;

#include "Event.hxx"

class BasicCPU {
  public:
    // Constructor
    BasicCPU(const char* name, int granularity, 
        vector<AddressSpace*>& addressSpaces,
        const char* traceRecordFormat, 
        const char* defaultTraceRecordEntries);

    // Destructor
    virtual ~BasicCPU();

    // Answer name of the microprocessor
    const char* Name() const { return myName; }

    // Answer the granularity of the microprocessor
    unsigned int Granularity() const { return myGranularity; }

    // Answer a reference to my event handler
    EventHandler& eventHandler() { return myEventHandler; };

    // Answer the number of address spaces used by the processor
    unsigned int NumberOfAddressSpaces() const
    { return myAddressSpaces.size(); }

    // Answer the execution trace record format
    const char* ExecutionTraceRecord() const
    { return myExecutionTraceRecord; }

    // Answer the default execution trace entries
    const char* DefaultExecutionTraceEntries() const
    { return myDefaultExecutionTraceEntries; }

    // Answer the indexed address space object
    AddressSpace& addressSpace(unsigned int addressSpace)
    { return *myAddressSpaces[addressSpace]; }

    // Execute next instruction. Answers pointer to an error message or null
    virtual const char* ExecuteInstruction(string& traceRecord, bool trace) = 0;

    // Handle an interrupt request from a device
    virtual void InterruptRequest(BasicDevice* device, int level) = 0;

    // Perform a system reset
    virtual void Reset() = 0;

    // Answers value of the program counter register
    virtual unsigned long ValueOfProgramCounter() = 0;

    // Sets named register to the given hexidecimal value
    virtual void SetRegister(const string& name, const string& hexValue) = 0;

    // Clear the CPU's Statistics
    virtual void ClearStatistics() = 0;

    // Append all of the CPU's registers to the RegisterInformationList object
    virtual void BuildRegisterInformationList(RegisterInformationList&) = 0;

    // Append all of the CPU's stats to the StatisticalInformationList object
    virtual void BuildStatisticalInformationList(StatisticalInformationList&)=0;

  protected:
    // Pointer to array of address space objects
    vector<AddressSpace*>& myAddressSpaces;

    // My event handler
    EventHandler myEventHandler;

  private:
    // My name
    const char* myName;

    // My granularity in bytes
    const unsigned int myGranularity;

    // Trace record format used by the ExecuteInstruction member function
    const char* myExecutionTraceRecord;

    // Default fields of the trace record that should be displayed by UI
    const char* myDefaultExecutionTraceEntries;
};
#endif

