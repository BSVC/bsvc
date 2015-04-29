///////////////////////////////////////////////////////////////////////////////
//
// AddressSpace.hxx
//
//   This class maintains a list of devices and provides methods to 
// peek and poke into them.  
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// June 27,1993
//
// Adapted to 68360 - DJ - 1996/11
// - moved 'size' constant from cpu32.hxx into this file, for
//   use by '32 bit capable' Peek and Poke methods
// - added Peek and Poke methods with size parameter
//
///////////////////////////////////////////////////////////////////////////////
// $Id: AddressSpace.hxx,v 1.1 1996/08/02 14:48:41 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef ADDRESSSPACE_HXX
#define ADDRESSSPACE_HXX

#include <list>
#include <string>

#ifdef USE_STD
  using namespace std;
#endif

class AddressSpace;
class BasicDevice;

#include "BasicDevice.hxx"

// Size Constants
#define BYTE   0
#define WORD   1
#define LONG   2


///////////////////////////////////////////////////////////////////////////////
// AddressSpace class declaration
///////////////////////////////////////////////////////////////////////////////
class AddressSpace {
  public:
    // Used to retrieve information about attached devices
    struct DeviceInformation {
      string name;
      string arguments;
      unsigned int index;
    };

  public:
    // Constructor
    AddressSpace(unsigned long maximumAddress);

    // Destructor
    virtual ~AddressSpace();

    // Answer the maximum address of the address space
    unsigned long MaximumAddress() const { return(myMaximumAddress); }

    // Attach the given device. Answers true iff successful
    bool AttachDevice(BasicDevice*);

    // Detach the indexed device and destroy it. Answers true iff successful
    bool DetachDevice(unsigned int index);

    // Reset all the attached devices
    void Reset();

    // Answers the number of attached devices
    unsigned int NumberOfAttachedDevices() const;

    // Get information about the indexed device. Answer true iff successful
    bool GetDeviceInformation(unsigned int index, 
        AddressSpace::DeviceInformation& info) const;

    // Peek the given location.  Answers true iff successful
    virtual bool Peek(unsigned long addr, unsigned char &c);

    // Poke the given location.  Answers true iff successful
    virtual bool Poke(unsigned long addr, unsigned char c);

   public: 
    // Peek the given location.  Answers true iff successful
    virtual bool Peek(unsigned long addr, unsigned long &d, int size);

    // Poke the given location.  Answers true iff successful
    virtual bool Poke(unsigned long addr, unsigned long d, int size);

  private:
    // List of attached devices
    list<BasicDevice*> myDevices; 

    // Maximum address for this address space (In CPU words not bytes!!)
    const unsigned long myMaximumAddress;
	
    // Device cache
    BasicDevice* read_device_cache1;
    BasicDevice* read_device_cache2;
    BasicDevice* read_device_cache3;
    BasicDevice* write_device_cache1;
    BasicDevice* write_device_cache2;
    BasicDevice* write_device_cache3;
	
    BasicDevice* FindReadDevice(unsigned long address);
    BasicDevice* FindWriteDevice(unsigned long address);
};
#endif

