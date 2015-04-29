///////////////////////////////////////////////////////////////////////////////
//
// DeviceRegistry.hxx
//
//   This class keeps up with a list of all of the availible devices and
// allocates them.  It's dervied from the BasicDeviceRegistry
//
// Sim68000 "Motorola 68000 Simulator"
// Copyright (c) 1993
// By: Bradford W. Mott
// October 30,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: DeviceRegistry.hxx,v 1.1 1996/08/02 15:00:41 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef DEVICEREGISTRY_HXX
#define DEVICEREGISTRY_HXX

#include "BasicDeviceRegistry.hxx"

///////////////////////////////////////////////////////////////////////////////
// The DeviceRegistry Class
///////////////////////////////////////////////////////////////////////////////
class DeviceRegistry : public BasicDeviceRegistry {
  public:
    // Constructor
    DeviceRegistry();

    // Create a device with the given name. Answers true iff successful
    bool Create(const string& name, const string& args, BasicCPU& cpu,
        BasicDevice*& device); 

  private:
    static const DeviceInformation ourDeviceInfo[];
};
#endif

