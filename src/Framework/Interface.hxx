///////////////////////////////////////////////////////////////////////////////
//
// Interface.hxx
//
// This is the user interface command class.  It handles all of the
// command's issue by the user interface.
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// October 21,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: Interface.hxx,v 1.1 1996/08/02 14:52:26 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef INTERFACE_HXX
#define INTERFACE_HXX

#ifdef WIN32
  #include <iostream>
#else
  #include <iostream.h>
#endif

#ifdef USE_STD
  using namespace std;
#endif

class BasicCPU;
class BasicDeviceRegistry;
class BasicLoader;
class BreakpointList;

class Interface {
  public:
    // Constructor
    Interface(BasicCPU& cpu, BasicDeviceRegistry& registry, 
        BasicLoader& loader);

    // Command loop
    void CommandLoop();
 
  private:
    // Structure for the interface's command table
    struct CommandTable {
      const char* name;
      void (Interface::*mfp)(char*);
    };

  private:
    // Indicates the number of commands in the command table
    const unsigned int myNumberOfCommands;

    // Reference to the CPU I'm managing
    BasicCPU& myCPU;

    // Reference to the device registry
    BasicDeviceRegistry& myDeviceRegistry;

    // Reference to the loader
    BasicLoader& myLoader;

    // Reference to the input stream used to get information from the UI
    istream& myInputStream;

    // Reference to the output stream used to send information to the UI
    ostream& myOutputStream;

    // Breakpoint list to manage the breakpoints
    BreakpointList& myBreakpointList;

    // Execute the given command
    void ExecuteCommand(char* command);

    // Table of commands
    static CommandTable ourCommandTable[];

    // Member funtion for each of the commands
    void AddBreakpoint(char* args);
    void AttachDevice(char* args);
    void ClearStatistics(char* args);
    void DeleteBreakpoint(char* args);
    void DetachDevice(char* args);
    void FillMemoryBlock(char* args);
    void ListAttachedDevices(char* args);
    void ListBreakpoints(char* args);
    void ListDevices(char* args);
    void ListDeviceScript(char* args);
    void ListExecutionTraceRecord(char* args);
    void ListDefaultExecutionTraceEntries(char* args);
    void ListGranularity(char* args);
    void ListMemory(char* args);
    void ListMaximumAddress(char* args);
    void ListNumberOfAddressSpaces(char* args);
    void ListRegisters(char* args);
    void ListRegisterValue(char* args);
    void ListRegisterDescription(char* args);
    void ListStatistics(char* args);
    void LoadProgram(char* args);
    void ProgramCounterValue(char* args);
    void Reset(char* args);
    void Run(char* args);
    void SetRegister(char* args);
    void SetMemory(char* args);
    void Step(char* args);
};
#endif

