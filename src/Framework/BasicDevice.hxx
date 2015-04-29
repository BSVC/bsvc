///////////////////////////////////////////////////////////////////////////////
//
// BasicDevice.hxx
//
//   This should be the base class for all devices
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// July 26,1993
//
// Adapted to 68360 - DJ - 1996/11
// - Peek and Poke with size as parameter.
//
///////////////////////////////////////////////////////////////////////////////
// $Id: BasicDevice.hxx,v 1.1 1996/08/02 14:50:07 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef BASICDEVICE_HXX
#define BASICDEVICE_HXX

#include <string>

#ifdef USE_STD
  using namespace std;
#endif

class BasicDevice;
class BasicCPU;

#include "Event.hxx"

#define AUTOVECTOR_INTERRUPT  -1
#define SPURIOUS_INTERRUPT    -2

class BasicDevice : public EventBase {
  public:
    // Constructor
    BasicDevice(const char* name, const string& arguments, BasicCPU& cpu);

    // Destructor
    virtual ~BasicDevice();

    // Change my startup error message
    void ErrorMessage(const string& message)
    { myErrorMessage = message; }

    // Answer my startup error message
    string ErrorMessage() const
    { return(myErrorMessage); }

    // Answer my name
    const char* Name() const { return(myName); }

    // Answer the CPU I belong too
    BasicCPU& CPU() const { return(myCPU); }

    // Answer my initialization arguments
    string Arguments() const { return (myArguments); }

    // Answers true iff the address maps into the device
    virtual bool CheckMapped(unsigned long) const = 0;

    // Answers the lowest address used by the device
    virtual unsigned long LowestAddress() const = 0;

    // Answers the highest address used by the device
    virtual unsigned long HighestAddress() const = 0;

    // Get a byte from the device
    virtual unsigned char Peek(unsigned long address) = 0;

    // Put a byte into the device
    virtual void Poke(unsigned long address, unsigned char c) = 0;

    // Get data from the device
    virtual bool Peek(unsigned long address, unsigned long& data, int size);

    // Put data into the device
    virtual bool Poke(unsigned long address, unsigned long data, int size);

    // Reset the device
    virtual void Reset();

    // This routine sends an interrupt request (IRQ) to the CPU
    virtual void InterruptRequest(int level);
       
    // This routine is called by the CPU when it processes an interrupt
    virtual long InterruptAcknowledge(int level);

  protected:
    // Reference to the CPU I belong too 
    BasicCPU& myCPU;

    // My name (i.e. RAM, ROM, etc.)
    const char* myName;

    // Arguments passed to constructor
    string myArguments;

    // Error that occured during construction
    string myErrorMessage;

    // Interrupt pending flag
    bool myInterruptPending;
};
#endif

