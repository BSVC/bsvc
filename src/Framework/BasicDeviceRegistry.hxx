///////////////////////////////////////////////////////////////////////////////
//
// BasicDeviceRegistry.hxx
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
// $Id: BasicDeviceRegistry.hxx,v 1.1 1996/08/02 14:50:41 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef BASICDEVICEREGISTRY_HXX
#define BASICDEVICEREGISTRY_HXX

#include <string>

#ifdef USE_STD
  using namespace std;
#endif

class BasicCPU;
class BasicDevice;

// Device information structure
struct DeviceInformation {
  const char* name;           // The name of the device ("RAM","m6850",etc)
  const char* description;    // A short description of the device
  const char* script;         // UI script to get the device attachment args
};

class BasicDeviceRegistry {
  public:
    // Constructor
    BasicDeviceRegistry(const DeviceInformation* devices, unsigned int number)
        : myDevices(devices),
          myNumberOfDevices(number)
    {}

    // Destructor
    virtual ~BasicDeviceRegistry()
    {}

    // Answers the number of devices
    unsigned int NumberOfDevices() { return myNumberOfDevices; }

    // Get device information for the given index. Answers true iff successful
    bool Information(unsigned int index, DeviceInformation& information);

    // Create a device with the given name. Answers true iff successful
    virtual bool Create(const string& name, const string& args, 
        BasicCPU& cpu, BasicDevice*& device) = 0; 

  private:
    // Array of devices in the simulator
    const DeviceInformation* myDevices;

    // Number of devices in the simulator
    const unsigned int myNumberOfDevices; 
};
#endif

