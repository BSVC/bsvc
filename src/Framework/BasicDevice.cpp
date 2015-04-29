#include "Framework/BasicDevice.hpp"
#include "Framework/BasicCPU.hpp"

BasicDevice::BasicDevice(const std::string &name, const std::string &args, BasicCPU &cpu)
    : EventBase(cpu.eventHandler()), myCPU(cpu), myName(name),
      myArguments(args), myInterruptPending(false) {}

BasicDevice::~BasicDevice() {}

// Reset device - At the very least the interrupt pending flag has to be reset
void BasicDevice::Reset() { myInterruptPending = false; }

// This routine should set myInterruptPending flag and send a request
// to the CPU for an interrupt.
void BasicDevice::InterruptRequest(int level) {
  // If no interrupt is pending then request one
  if (!myInterruptPending) {
    myInterruptPending = true;
    myCPU.InterruptRequest(this, level);
  }
}

// This routine is called by the CPU when it processes a requested interrupt.
// It should return the vector number associated with the interrupt or
// AUTOVECTOR_INTERRUPT if the device doesn't generate vectors.  This default
// routine only does autovector interrupts.
int BasicDevice::InterruptAcknowledge(int) {
  if (myInterruptPending) {
    myInterruptPending = false;
    return (AUTOVECTOR_INTERRUPT);
  } else {
    return (SPURIOUS_INTERRUPT);
  }
}

// Default Peek implementation, for devices not supporting 'size' Peek.
bool BasicDevice::Peek(Address address, unsigned long &data, int size) {
  switch (size) {
  case BYTE:
    data = (unsigned long)Peek(address);
    break;

  case WORD:
    data = (unsigned long)(Peek(address) * 0x100 + Peek(address + 1));
    break;

  case LONG:
    data = (unsigned long)((Peek(address) << 24) + (Peek(address + 1) << 16) +
                           (Peek(address + 2) << 8) + (Peek(address + 3)));
    break;

  default:
    return false;
  }
  return true;
}

// Default Poke implementation, for devices not supporting 'size' Poke.
bool BasicDevice::Poke(Address address, unsigned long data, int size) {
  switch (size) {
  case BYTE:
    Poke(address, (Byte)data);
    break;
  case WORD:
    Poke(address + 1, (Byte)data);
    Poke(address, (Byte)(data >> 8));
    break;
  case LONG:
    Poke(address + 3, (Byte)data);
    Poke(address + 2, (Byte)(data >> 8));
    Poke(address + 1, (Byte)(data >> 16));
    Poke(address + 0, (Byte)(data >> 24));
    break;
  default:
    return false;
  }
  return true;
}
