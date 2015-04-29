///////////////////////////////////////////////////////////////////////////////
//
// RAM.hxx 
//
// Random Access Memory Device
//
// Sim68000 "Motorola 68000 Simulator"
// Copyright (c) 1993
// By: Bradford W. Mott
// July 26,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: RAM.hxx,v 1.1 1996/08/02 15:02:57 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef RAM_HXX
#define RAM_HXX

#include <string>

#ifdef USE_STD
  using namespace std;
#endif

#include "BasicDevice.hxx"

class RAM : public BasicDevice {
  public:
    // Constructor
    RAM(const string& args, BasicCPU& cpu);

    // Destructor
    ~RAM();

    // Answers true iff the address maps into the device
    bool CheckMapped(unsigned long addr) const;

    // Answers the lowest address used by the device
    unsigned long LowestAddress() const
    { return myBaseAddress; }

    // Answers the highest address used by the device
    unsigned long HighestAddress() const
    { return myBaseAddress + mySize - 1; }

    // Get a byte from the device
    unsigned char Peek(unsigned long address)
    { return myBuffer[address - myBaseAddress]; }

    // Put a byte into the device
    virtual void Poke(unsigned long address, unsigned char c) 
    { myBuffer[address - myBaseAddress] = c; }

    // RAM never has Events
    void EventCallback(long, void*)
    {}

  protected:
    // Buffer to hold the RAM's contents
    unsigned char* myBuffer;
 
  private:
    // Starting address of the RAM device
    unsigned long myBaseAddress;

    // Size of the RAM device in bytes
    unsigned long mySize;
};
#endif

