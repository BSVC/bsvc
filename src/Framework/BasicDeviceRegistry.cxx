///////////////////////////////////////////////////////////////////////////////
//
// BasicDeviceRegistry.cxx
//
//   This abstract base class is used to derive a class that maintains a list
// of all the device in the simulator and allows them to be created.
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// October 30,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: BasicDeviceRegistry.cxx,v 1.1 1996/08/02 14:50:34 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "BasicDeviceRegistry.hxx"

///////////////////////////////////////////////////////////////////////////////
// Get device information for the given index. Answers true iff successful
///////////////////////////////////////////////////////////////////////////////
bool BasicDeviceRegistry::Information(unsigned int i, DeviceInformation &info)
{
  if(i < myNumberOfDevices)
  {
    info = myDevices[i];
    return true;
  }
  else
  {
    return false;
  } 
}

