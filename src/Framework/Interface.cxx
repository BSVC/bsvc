///////////////////////////////////////////////////////////////////////////////
//
// Interface.cxx
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
// $Id: Interface.cxx,v 1.1 1996/08/07 19:16:36 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>

  #undef min
  #undef max

  #include <iostream>
  #include <fstream>
  #include <strstream>
  #include <string>
#else
  #include <sys/time.h>
  #include <sys/types.h>
  #include <unistd.h>

  #include <iostream.h>
  #include <strstream.h>
  #include <string>
#endif

#ifdef USE_STD
  using namespace std;
#endif

#include "Interface.hxx"
#include "BasicCPU.hxx"
#include "AddressSpace.hxx"
#include "BasicDeviceRegistry.hxx"
#include "BasicLoader.hxx"
#include "BreakpointList.hxx"
#include "StatInfo.hxx"
#include "RegInfo.hxx"
#include "Tools.hxx"

///////////////////////////////////////////////////////////////////////////////
// Simulator's main loop, gets a command from the UI, parses and executes it
///////////////////////////////////////////////////////////////////////////////
void Interface::CommandLoop()
{
  // Notify the UI that this is a BSVC simulator
  myOutputStream << "BSVC Simulator" << endl << flush;

  for(;;)
  {
    char command[2048];

    // Prompt the user interface
    myOutputStream << "Ready!" << endl << flush;

    // Get the next command from the UI
    do
    {
#ifdef WIN32
      gets(command);
#else
      myInputStream.getline(command, 2048);
#endif
    } while(string(command) == "NOP");

    
    if(string(command) == "Exit")
    {
      break;
    }
    else
    {
      // For some version of iostream we have to have a little padding :-(
      strcat(command, " ");

      ExecuteCommand(command);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Parse and execute the given command
///////////////////////////////////////////////////////////////////////////////
void Interface::ExecuteCommand(char* command)
{
  // Decide which command to call
  for(unsigned int t = 0; t < myNumberOfCommands; ++t)
  {
    if(string(command).find(ourCommandTable[t].name) == 0)
    {
      char* args = command + string(ourCommandTable[t].name).length() + 1;

      (this->*ourCommandTable[t].mfp)(args);
      return;
    }
  }
  myOutputStream << "ERROR: Unknown command!" << endl << flush;
}

///////////////////////////////////////////////////////////////////////////////
// The user interface command table
///////////////////////////////////////////////////////////////////////////////
Interface::CommandTable Interface::ourCommandTable[] = {
  {"AddBreakpoint",                    &Interface::AddBreakpoint},
  {"AttachDevice",                     &Interface::AttachDevice},
  {"ClearStatistics",                  &Interface::ClearStatistics},
  {"DetachDevice",                     &Interface::DetachDevice},
  {"DeleteBreakpoint",                 &Interface::DeleteBreakpoint},
  {"FillMemoryBlock",                  &Interface::FillMemoryBlock},
  {"ListAttachedDevices",              &Interface::ListAttachedDevices},
  {"ListBreakpoints",                  &Interface::ListBreakpoints},
  {"ListDevices",                      &Interface::ListDevices},
  {"ListDeviceScript",                 &Interface::ListDeviceScript},
  {"ListExecutionTraceRecord",         &Interface::ListExecutionTraceRecord},
  {"ListDefaultExecutionTraceEntries", &Interface::ListDefaultExecutionTraceEntries},
  {"ListGranularity",                  &Interface::ListGranularity},
  {"ListMemory",                       &Interface::ListMemory},
  {"ListMaximumAddress",               &Interface::ListMaximumAddress},
  {"ListNumberOfAddressSpaces",        &Interface::ListNumberOfAddressSpaces},
  {"ListRegisters",                    &Interface::ListRegisters},
  {"ListRegisterValue",                &Interface::ListRegisterValue},
  {"ListRegisterDescription",          &Interface::ListRegisterDescription},
  {"ListStatistics",                   &Interface::ListStatistics},
  {"LoadProgram",                      &Interface::LoadProgram},
  {"ProgramCounterValue",              &Interface::ProgramCounterValue},
  {"Reset",                            &Interface::Reset},
  {"Run",                              &Interface::Run},
  {"SetMemory",                        &Interface::SetMemory},
  {"SetRegister",                      &Interface::SetRegister},
  {"Step",                             &Interface::Step}
};

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
Interface::Interface(BasicCPU& cpu, BasicDeviceRegistry& registry,
    BasicLoader& loader) 
    : myNumberOfCommands(sizeof(ourCommandTable) / sizeof(CommandTable)),
      myCPU(cpu),
      myDeviceRegistry(registry),
      myLoader(loader),
      myInputStream(cin),
      myOutputStream(cout),
      myBreakpointList(*new BreakpointList)
{
}

///////////////////////////////////////////////////////////////////////////////
// Print the value of the program counter
///////////////////////////////////////////////////////////////////////////////
void Interface::ProgramCounterValue(char*)
{
  myOutputStream << hex << myCPU.ValueOfProgramCounter() << endl << flush;
}

///////////////////////////////////////////////////////////////////////////////
// List the processor's execution trace record
///////////////////////////////////////////////////////////////////////////////
void Interface::ListExecutionTraceRecord(char*)
{
  myOutputStream << myCPU.ExecutionTraceRecord() << endl << flush;
}

///////////////////////////////////////////////////////////////////////////////
// List the processor's default execution trace entries to display
///////////////////////////////////////////////////////////////////////////////
void Interface::ListDefaultExecutionTraceEntries(char*)
{
  myOutputStream << myCPU.DefaultExecutionTraceEntries() << endl << flush;
}

///////////////////////////////////////////////////////////////////////////////
// Clear the cpu's statistics
///////////////////////////////////////////////////////////////////////////////
void Interface::ClearStatistics(char*)
{
  myCPU.ClearStatistics();
}

///////////////////////////////////////////////////////////////////////////////
// List the cpu's statistics
///////////////////////////////////////////////////////////////////////////////
void Interface::ListStatistics(char*)
{
  StatisticalInformationList list(myCPU);

  for(unsigned int t = 0; t < list.NumberOfElements(); ++t)
  {
    StatisticInformation info;
    list.Element(t, info);
    myOutputStream << info.Statistic() << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// List the CPU's registers
///////////////////////////////////////////////////////////////////////////////
void Interface::ListRegisters(char*)
{
  RegisterInformationList list(myCPU);
  unsigned int t, width;

  // Find the longest register name
  for(width = 0, t = 0; t < list.NumberOfElements(); ++t)
  {
    RegisterInformation info;
    list.Element(t, info);

    if(width < (info.Name()).length())
      width = (info.Name()).length();
  }

  // Send the register list to the user interface
  for(t = 0; t < list.NumberOfElements(); ++t)
  {
    RegisterInformation info;
    list.Element(t, info);

    myOutputStream << info.Name();
    for(unsigned int i = 0; i < width - (info.Name()).length(); ++i)
      myOutputStream << ' ';
    myOutputStream << " = " << info.HexValue() << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// List a register's description
///////////////////////////////////////////////////////////////////////////////
void Interface::ListRegisterDescription(char* args)
{
  RegisterInformationList list(myCPU);
  istrstream in(args);
  string name;

  in >> name;

  // Make sure we were able to read the arguments 
  if(in)
  {
    for(unsigned int t = 0; t < list.NumberOfElements(); ++t)
    {
      RegisterInformation info;
  
      list.Element(t, info);
      if(name == info.Name())
      {
        myOutputStream << info.Description() << endl << flush;
        return;
      }
    }
    myOutputStream << "ERROR: Invalid register name!" << endl << flush;
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Set one of the cpu's registers to the given value
///////////////////////////////////////////////////////////////////////////////
void Interface::SetRegister(char* args)
{
  istrstream in(args);
  RegisterInformationList list(myCPU);
  string name;
  string value;

  // Read the register name and value
  in >> name >> value;

  // Make sure we were able to read the arguments 
  if(in)
  {
    for(unsigned int t = 0; t < list.NumberOfElements(); ++t)
    {
      RegisterInformation info;

      list.Element(t, info);
      if(name == info.Name())
      {
        myCPU.SetRegister(name, value);
        return;
      }
    }
    myOutputStream << "ERROR: Invalid register name!" << endl << flush;
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// List the value of one of the cpu's registers
///////////////////////////////////////////////////////////////////////////////
void Interface::ListRegisterValue(char* args)
{
  istrstream in(args);
  RegisterInformationList list(myCPU);
  string name;

  // Read the register name
  in >> name;

  // Make sure we were able to read the arguments 
  if(in)
  {
    for(unsigned int t = 0; t < list.NumberOfElements(); ++t)
    {
      RegisterInformation info;

      list.Element(t, info);
      if(name == info.Name())
      {
        myOutputStream << info.HexValue() << endl << flush;
        return;
      }
    }
    myOutputStream << "ERROR: Invalid register name!" << endl << flush;
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Detach a device from the simulator
///////////////////////////////////////////////////////////////////////////////
void Interface::DetachDevice(char* args)
{
  istrstream in(args);
  unsigned int addressSpace; 
  unsigned int deviceIndex; 

  in >> addressSpace >> deviceIndex;

  // Make sure we were able to read the arguments
  if(in)
  {
    if(addressSpace >= myCPU.NumberOfAddressSpaces())
    {
      myOutputStream << "ERROR: Invalid address space!" << endl << flush;
      return;
    }

    if(!(myCPU.addressSpace(addressSpace).DetachDevice(deviceIndex)))
    {
      myOutputStream << "ERROR: Couldn't detach device!" << endl << flush;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attach a device to the simulator
///////////////////////////////////////////////////////////////////////////////
void Interface::AttachDevice(char* args)
{
  istrstream in(args);
  char c;
  string name;
  string deviceArgs;
  unsigned int addressSpace; 

  in >> addressSpace >> name >> c;
  in.unsetf(ios::skipws);

  if(c != '{')
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
    return;
  }

  in >> c;
  while((in) && (c != '}'))
  {
    deviceArgs += c;
    in >> c;
  }

  // Make sure we were able to read the arguments
  if(in)
  {
    if(addressSpace >= myCPU.NumberOfAddressSpaces())
    {
      myOutputStream << "ERROR: Invalid address space!" << endl << flush;
      return;
    }
    
    BasicDevice *device;
    if(myDeviceRegistry.Create(name, deviceArgs, myCPU, device))
      myCPU.addressSpace(addressSpace).AttachDevice(device);
    else
      myOutputStream << "ERROR: Couldn't create the device!" << endl << flush;
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Add a breakpoint
///////////////////////////////////////////////////////////////////////////////
void Interface::AddBreakpoint(char* args)
{
  istrstream in(args);
  unsigned long address; 

  in >> hex >> address; 

  // Make sure we were able to read the arguments
  if(in)
  {
    myBreakpointList.Add(address);
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Delete a breakpoint
///////////////////////////////////////////////////////////////////////////////
void Interface::DeleteBreakpoint(char* args)
{
  istrstream in(args);
  unsigned long address; 

  in >> hex >> address; 

  // Make sure we were able to read the arguments
  if(in)
  {
    if(!myBreakpointList.Delete(address))
      myOutputStream << "ERROR: Couldn't delete breakpoint!" << endl << flush;
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// File the memory block with the given value
///////////////////////////////////////////////////////////////////////////////
void Interface::FillMemoryBlock(char* args)
{
  istrstream in(args);
  unsigned int addressSpace;
  unsigned long address;
  unsigned long length;
  string value;

  in >> addressSpace >> hex >> address >> hex >> length >> value;

  // Make sure we were able to read the arguments
  if(in)
  {
    if(addressSpace >= myCPU.NumberOfAddressSpaces())
    {
      myOutputStream << "ERROR: Invalid address space!" << endl << flush;
      return;
    }

    if(address > myCPU.addressSpace(addressSpace).MaximumAddress())
    {
      myOutputStream << "ERROR: Invalid address!" << endl << flush;
      return;
    }

    for(unsigned int i = 0; i < length; ++i)
    {
      unsigned long addr = (address + i) * myCPU.Granularity();
      for(unsigned int t = 0; t < myCPU.Granularity(); ++t)
      {
        myCPU.addressSpace(addressSpace).Poke(addr + t,
            StringToInt(string(value, t * 2, 2)));
      }
    }
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// List the breakpoints
///////////////////////////////////////////////////////////////////////////////
void Interface::ListBreakpoints(char*)
{
  for(unsigned int t = 0; t < myBreakpointList.NumberOfBreakpoints(); ++t)
  {
    unsigned long address;
    myBreakpointList.GetBreakpoint(t, address);
    myOutputStream << IntToString(address, 8) << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// List the devices attached to the simulator
///////////////////////////////////////////////////////////////////////////////
void Interface::ListAttachedDevices(char* args)
{
  istrstream in(args);
  unsigned int addressSpace;

  in >> addressSpace;

  // Make sure we were able to read the arguments
  if(in)
  {
    if(addressSpace >= myCPU.NumberOfAddressSpaces())
    {
      myOutputStream << "ERROR: Invalid address space!" << endl << flush;
      return;
    }

    for(unsigned int t = 0; 
        t < myCPU.addressSpace(addressSpace).NumberOfAttachedDevices();
        ++t)
    {
      AddressSpace::DeviceInformation info;
      myCPU.addressSpace(addressSpace).GetDeviceInformation(t, info);
      myOutputStream << info.name << " {" << info.arguments << "}" << endl
          << flush;
    }
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}
 
///////////////////////////////////////////////////////////////////////////////
// List the devices in the simulator
///////////////////////////////////////////////////////////////////////////////
void Interface::ListDevices(char*)
{
  for(unsigned int t = 0; t < myDeviceRegistry.NumberOfDevices(); ++t)
  {
    DeviceInformation info;
    myDeviceRegistry.Information(t, info);
    myOutputStream << info.name << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// List the UI script for the named device
///////////////////////////////////////////////////////////////////////////////
void Interface::ListDeviceScript(char* args)
{
  istrstream in(args);
  string name;

  in >> name;

  // Make sure we were able to read the arguments
  if(in)
  {
    for(unsigned int t = 0; t < myDeviceRegistry.NumberOfDevices(); ++t)
    {
      DeviceInformation info;
      myDeviceRegistry.Information(t, info);

      if(name == info.name)
      {
        myOutputStream << info.script << endl << flush;
        return;
      }
    }
    myOutputStream << "ERROR: Invalid device name!" << endl << flush;
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// List a memory block
///////////////////////////////////////////////////////////////////////////////
void Interface::ListMemory(char* args)
{ 
  istrstream in(args);
  unsigned int addressSpace;
  unsigned long address;
  unsigned int length;
  unsigned int wordsPerLine;
  string line;

  in >> addressSpace >> hex >> address >> length >> wordsPerLine;

  // Make sure we were able to read the arguments
  if(in)
  {
    if(addressSpace >= myCPU.NumberOfAddressSpaces())
    {
      myOutputStream << "ERROR: Invalid address space!" << endl << flush;
      return;
    }

    unsigned int numberOfWords = 0;
    for(unsigned int t = 0; t < length; ++t)
    {
      for(unsigned int s = 0; s < myCPU.Granularity(); ++s)
      {
        unsigned char value;
        if(myCPU.addressSpace(addressSpace).Peek((address + t) * 
            myCPU.Granularity() + s, value))
        {
          line += IntToString(value, 2);
        }
        else
        {
          line += "xx";
        }
      }
      ++numberOfWords;
      if(numberOfWords >= wordsPerLine)
      {
        myOutputStream << line << endl << flush;
        numberOfWords = 0;
        line = "";
      }
      else
      { 
        line += " ";
      }
    } 
    if(line != "")
      myOutputStream << line << endl << flush;
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Perform a Step of instructions
///////////////////////////////////////////////////////////////////////////////
void Interface::Step(char* args)
{
  istrstream in(args);
  int numberOfSteps;

  in >> numberOfSteps;

  // Make sure we were able to read the arguments 
  if(in)
  {
    for(int t = 0; t < numberOfSteps; ++t)
    {
      string traceRecord;
      const char* message;
      
      if((message = myCPU.ExecuteInstruction(traceRecord, true)) != 0)
      {
        myOutputStream << "{SimulatorMessage {" << message << "}}"
            << endl << flush;
        break;
      }
      myOutputStream << traceRecord << endl << flush;
    }
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Preform the Reset command 
///////////////////////////////////////////////////////////////////////////////
void Interface::Reset(char*)
{
  // Reset the CPU (which should also reset the devices)
  myCPU.Reset();
}

///////////////////////////////////////////////////////////////////////////////
// Perform the Run command 
///////////////////////////////////////////////////////////////////////////////
void Interface::Run(char* args)
{
  istrstream in(args);
  unsigned int i;
  char name[1024];
  char c;
  
  // The filename here is used to get around the problem with nonblocking
  // I/O in Tcl for windows.  When we're finished running we should
  // write something in this file.
  in >> c;
  in.unsetf(ios::skipws);

  if(c != '{')
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
    return;
  }

  in >> c;
  i = 0;
  while((in) && (c != '}'))
  {
    name[i++] = c;
    in >> c;
  }
  name[i] = 0;

 
  // Run until something stops us!!!
  for(unsigned int steps = 0; ; ++steps)
  {
    string traceRecord;
    const char* message = myCPU.ExecuteInstruction(traceRecord, false);
    if(message != 0)
    {
      if(*message == '.')
      {
        myOutputStream << (message + 1) << flush;
      }
      else
      {
        myOutputStream << "Execution stopped: " << message << endl << flush;
        break;
      }
    }
    else if(myBreakpointList.Check(myCPU.ValueOfProgramCounter()))
    {
      myOutputStream << "Execution stopped at a breakpoint!" << endl << flush;
      break;
    }
#ifdef WIN32
    // Poll for input every 1024 steps
    else if((steps & 0x03ff) == 0)
    {
      DWORD count;

      // Get STDIN handle
      HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);

      // See if there's any input available
      if(PeekNamedPipe(handle, (LPVOID) NULL, (DWORD)0, (LPDWORD) NULL,
          &count, (LPDWORD) NULL))
      {
        // Stop running if any input is available
        if(count != 0)
        {
          // Read the "StopRunning" Command
          char dummy[1024];

#ifdef WIN32
          gets(dummy);
#else
          myInputStream.getline(dummy, 1024);
#endif

          myOutputStream << "Execution Interrupted!" << endl << flush;
          break;
        }
      }
    }
#else
    // Poll for input every 1024 steps
    else if((steps & 0x03FF) == 0)
    {
      fd_set rfds;
      struct timeval tv;
      int retval;

      // We're going to poll STDIN to see if any input is available
      FD_ZERO(&rfds);
      FD_SET(0, &rfds);

      // Don't wait at all
      tv.tv_sec = 0;
      tv.tv_usec = 0;

      #ifdef _HPUX_SOURCE
        retval = select(1, (int*)&rfds, NULL, NULL, &tv);
      #else
        retval = select(1, &rfds, NULL, NULL, &tv);
      #endif

      // Stop running if data is ready to be read
      if(retval == 1)
      {
        // Read the "StopRunning" Command
        char dummy[1024];
        myInputStream.getline(dummy, 1024);

        myOutputStream << "Execution Interrupted!" << endl << flush;
        break;
      }
    }
#endif
  }

#ifdef WIN32
  // Let the UI know we're not running any longer
  ofstream out(name);
  out << "Stop running dude!" << endl;
  out.close();
#endif
}

///////////////////////////////////////////////////////////////////////////////
// List the Maximum Address allow by the give address space
///////////////////////////////////////////////////////////////////////////////
void Interface::ListMaximumAddress(char* args)
{
  istrstream in(args);
  unsigned int addressSpace;

  in >> addressSpace;

  // Make sure we were able to read the arguments
  if(in)
  {
    if(addressSpace >= myCPU.NumberOfAddressSpaces())
    {
      myOutputStream << "ERROR: Invalid address space!" << endl << flush;
      return;
    }

    myOutputStream << hex 
        << myCPU.addressSpace(addressSpace).MaximumAddress()
        << endl << flush;
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// List the number of address spaces for the CPU
///////////////////////////////////////////////////////////////////////////////
void Interface::ListNumberOfAddressSpaces(char*)
{
  myOutputStream << dec << myCPU.NumberOfAddressSpaces() << endl << flush;
}

///////////////////////////////////////////////////////////////////////////////
// List the granularity of the cpu
///////////////////////////////////////////////////////////////////////////////
void Interface::ListGranularity(char*)
{
  myOutputStream << dec << myCPU.Granularity() << endl << flush;
}

///////////////////////////////////////////////////////////////////////////////
// Set a memory location to the given value
///////////////////////////////////////////////////////////////////////////////
void Interface::SetMemory(char* args)
{
  istrstream in(args);
  unsigned int addressSpace;
  unsigned long address;
  string value;

  in >> addressSpace >> hex >> address >> value;

  // Make sure we were able to read the arguments
  if(in)
  {
    if(addressSpace >= myCPU.NumberOfAddressSpaces())
    {
      myOutputStream << "ERROR: Invalid address space!" << endl << flush;
      return;
    }

    if(address > myCPU.addressSpace(addressSpace).MaximumAddress())
    {
      myOutputStream << "ERROR: Invalid address!" << endl << flush;
      return;
    }

    address *= myCPU.Granularity();
    for(unsigned int t = 0; t < myCPU.Granularity(); ++t)
    {
      myCPU.addressSpace(addressSpace).Poke(address + t, 
          StringToInt(string(value, t * 2, 2)));
    }
  }
  else
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Load the named program into the address space
///////////////////////////////////////////////////////////////////////////////
void Interface::LoadProgram(char* args)
{
  istrstream in(args);
  unsigned int addressSpace;
  unsigned int i;
  char name[1024];
  char c;

  in >> addressSpace >> c;
  in.unsetf(ios::skipws);

  if(c != '{')
  {
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
    return;
  }

  in >> c;
  i = 0;
  while((in) && (c != '}'))
  {
    name[i++] = c;
    in >> c;
  }
  name[i] = 0;

  // Make sure we were able to read the arguments
  if(in)
    myOutputStream << myLoader.Load(name, addressSpace) << endl << flush;
  else
    myOutputStream << "ERROR: Invalid arguments!" << endl << flush;
}


