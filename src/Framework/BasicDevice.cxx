///////////////////////////////////////////////////////////////////////////////
//
// BasicDevice.cxx
//
//   This should be the base class for all devices
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// November 26,1993
//
// Adapted to 68360 - DJ - 1996/11
// - Peek and Poke with size as parameter.
//
///////////////////////////////////////////////////////////////////////////////
// $Id: BasicDevice.cxx,v 1.1 1996/08/02 14:49:54 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "BasicDevice.hxx"
#include "BasicCPU.hxx"

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
BasicDevice::BasicDevice(const char* name, const string& args, BasicCPU& cpu)
    : EventBase(cpu.eventHandler()),
      myCPU(cpu),
      myName(name),
      myArguments(args),
      myInterruptPending(false)
{
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
BasicDevice::~BasicDevice()
{
}

///////////////////////////////////////////////////////////////////////////////
// Reset device - At the very least the interrupt pending flag has to be reset
///////////////////////////////////////////////////////////////////////////////
void BasicDevice::Reset()
{
  myInterruptPending = false;
}

///////////////////////////////////////////////////////////////////////////////
// This routine should set myInterruptPending flag and send a request
// to the CPU for an interrupt.
///////////////////////////////////////////////////////////////////////////////
void BasicDevice::InterruptRequest(int level)
{
  // If no interrupt is pending then request one
  if(!myInterruptPending)
  {
    myInterruptPending = true;
    myCPU.InterruptRequest(this, level);
  }
}
       
///////////////////////////////////////////////////////////////////////////////
// This routine is called by the CPU when it processes a requested interrupt.
// It should return the vector number associated with the interrupt or
// AUTOVECTOR_INTERRUPT if the device doesn't generate vectors.  This default
// routine only does autovector interrupts.
///////////////////////////////////////////////////////////////////////////////
long BasicDevice::InterruptAcknowledge(int)
{
  if(myInterruptPending)
  {
    myInterruptPending = false;
    return(AUTOVECTOR_INTERRUPT);
  }
  else
  {
    return(SPURIOUS_INTERRUPT);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Default Peek implementation, for devices not supporting 'size' Peek.
///////////////////////////////////////////////////////////////////////////////
bool BasicDevice::Peek(unsigned long address,unsigned long& data, int size)
{
  switch(size)
  {
    case BYTE:
      data = (unsigned long)Peek(address);
      break;

    case WORD:
      data = (unsigned long)(Peek(address)*0x100 + Peek(address+1));
      break;

    case LONG:
      data = (unsigned long)((Peek(address)<<24) + (Peek(address+1)<<16)
          + (Peek(address+2)<<8)+(Peek(address+3)));
      break;

    default:
      return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Default Poke implementation, for devices not supporting 'size' Poke.
///////////////////////////////////////////////////////////////////////////////
bool BasicDevice::Poke(unsigned long address,unsigned long data, int size)
{
  switch(size)
  {
    case BYTE:
      Poke(address,(unsigned char)data);
      break;

    case WORD:
      Poke(address+1,(unsigned char)data);
      Poke(address,(unsigned char)(data>>8));
      break;

    case LONG:
      Poke(address+3,(unsigned char)data);
      Poke(address+2,(unsigned char)(data>>8));
      Poke(address+1,(unsigned char)(data>>16));
      Poke(address,(unsigned char)(data>>24));
      break;

    default:
      return false;
  }	
  return true;
}

