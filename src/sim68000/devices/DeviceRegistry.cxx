///////////////////////////////////////////////////////////////////////////////
//
// DeviceRegistry.cxx
//
//   This class keeps up with a list of all of the availible devices and
// allocates them.  It's derived from the BasicDeviceRegistry
//
// Sim68000 "Motorola 68000 Simulator"
// Copyright (c) 1993
// By: Bradford W. Mott
// October 30,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: DeviceRegistry.cxx,v 1.1 1996/08/02 15:00:34 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
  #include <iostream>
#else
  #include <iostream.h>
#endif

#ifdef USE_STD
  using namespace std;
#endif

#include "DeviceRegistry.hxx"
#include "RAM.hxx"
#include "Timer.hxx"

#ifndef WIN32
  #include "M68681.hxx"
#endif

///////////////////////////////////////////////////////////////////////////////
// Array of device information (name, description, tcl script)
///////////////////////////////////////////////////////////////////////////////
const DeviceInformation DeviceRegistry::ourDeviceInfo[] = {
  {"RAM",
   "Random Access Memory",          
   #include "RAM.scr"
  }
#ifndef WIN32
 ,{"M68681",
   "Motorola 68681 Dual UART",
   #include "M68681.scr"
  }
#endif
 ,{"Timer",
   "Timer",          
   #include "Timer.scr"
  }
};

///////////////////////////////////////////////////////////////////////////////
// The Constructor
///////////////////////////////////////////////////////////////////////////////
DeviceRegistry::DeviceRegistry()
    : BasicDeviceRegistry(ourDeviceInfo,
          sizeof(ourDeviceInfo) / sizeof(DeviceInformation))
{}

///////////////////////////////////////////////////////////////////////////////
// Create a device with the given name. Answers true iff successful
///////////////////////////////////////////////////////////////////////////////
bool DeviceRegistry::Create(const string& name, const string& args,
    BasicCPU& cpu, BasicDevice*& device)
{
  if(name == "RAM")
    device = new RAM(args, cpu);
  else if(name == "Timer")
    device = new Timer(args, cpu);
#ifndef WIN32
  else if(name == "M68681")
    device = new M68681(args, cpu);
#endif
  else
    return false;

  // If the device's error message is not empty then return error
  if(device->ErrorMessage() != "")
  {
    delete device;
    return false;
  }
  else
  {
    return true;
  }
}

