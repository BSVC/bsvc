//
// Simulates a timer for the M68000 CPU, giving the Simulator
// the ablility to simulate a timer intrurpt.  Programmers can
// use this to implement time slicing.
//
// Contains Register and Offsets for the timer.
//
#include <ios>
#include <sstream>
#include <string>

#include "Framework/BasicCPU.hpp"
#include "M68k/devices/Timer.hpp"

Timer::Timer(const std::string &args, BasicCPU &cpu)
    : BasicDevice("Timer", args, cpu) {
  std::istringstream in(args);
  std::string keyword, equals;
  Address base;
  unsigned int irq;

  firstTime = true;
  // Scan "BaseAddress = nnnn"
  // This is important for the CPU to know what address space
  // is used by the device.
  in >> keyword >> equals >> std::hex >> base;
  if ((!in) || (keyword != "BaseAddress") || (equals != "=")) {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }

  in >> keyword >> equals >> std::hex >> irq;
  if ((!in) || (keyword != "IRQ") || (equals != "=")) {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }

  /*
    I've hardcoded the cycles setting to 2000 which should provide the
    125000 ticks per second for the PI/T

    unsigned int timer_cycle;
    in >> keyword >> equals >> std::hex >> timer_cycle;
    if((!in) || (keyword != "CycleSec") || (equals != "="))
    {
       ErrorMessage("Invalid initialization arguments!");
       return;
    }
    Timer::TIMER_CPU_CYCLE = timer_cycle;
  */

  Timer::TIMER_IRQ = irq;
  Timer::TIMER_CPU_CYCLE = 2000;
  baseAddress = base * cpu.Granularity();

  // This is needed to start the whole process.  The eventhandler
  // will always request an event and test to see if the user has
  // enabled the timer it not nothing happens but if so. The
  // CPR  register is copied to the CNTR and is decremented for every
  // Cycle.
  myCPU.eventHandler().Add(this, TIMER_EVENT, 0, TIMER_CPU_CYCLE);

  Reset();
}

// This routine should set myInterruptPending flag and send a request
// to the CPU for an interrupt.
void Timer::InterruptRequest(int level) {
  // If no interrupt is pending then request one
  if (!myInterruptPending) {
    myInterruptPending = true;
    myCPU.InterruptRequest(this, level);
  }
}

// This routine is called by the CPU when it processes a
// requested interrupt. It should return the vector number associated with
// the interrupt or AUTOVECTOR_INTERRUPT if the device doesn't generate
// vectors.  This default routine only does autovector interrupts.
// This function is called by the CPU.  The return value is usally the
// memory location of a exemption funcion. For more details on how to
// handle these functions one shold refer to the Motorola documentation.
//
// NOTE: Vector codes 5 and 7 are the only masks that will make the timer
// do somthing.  5 is a VECTOR and 7 is an AUTOVECTOR.  The code
// can be found in any manual.
int Timer::InterruptAcknowledge(unsigned int mask) {
  Byte cTCR = timerValue[TCR];

  // Interrupt was masked; keep pending.
  if (mask > TIMER_IRQ)
    return -1;

  if (!myInterruptPending) {
    return SPURIOUS_INTERRUPT;
  }
  switch (cTCR >> 5) {
  case 5:
    // By returning the address of the vector, the cpu will
    // call the vector that is located in the right mem location.
    myInterruptPending = false;
    return timerValue[TIVR];
  case 7:
    myInterruptPending = false;
    return AUTOVECTOR_INTERRUPT;
  }

  return SPURIOUS_INTERRUPT;
}

// Checks to see if address mapps to device.
bool Timer::CheckMapped(Address address) const {
  return (baseAddress <= address) && (address <= baseAddress + 22);
}

// Returns a byte from the device's address.
//
// PEEK is the protocal that is used by the device to
// inform the CPU what the registry settings are.
// The use need not update the memory location for the CPU
// All of this is done by the cpu and thus uses peek to
// read the data that the device is using for the  Memory
// (i.e. RAM)
Byte Timer::Peek(Address address) {
  switch ((address - baseAddress)) {
  case TCR:
    return timerValue[TCR];
  case TIVR:
    return timerValue[TIVR];
  case NULR:
    return timerValue[NULR];
  case CPRH:
    return timerValue[CPRH];
  case CPRM:
    return timerValue[CPRM];
  case CPRL:
    return timerValue[CPRL];
  case CNTRH:
    return timerValue[CNTRH];
  case CNTRM:
    return timerValue[CNTRM];
  case CNTRL:
    return timerValue[CNTRL];
  case TSR:
    return timerValue[TSR];
  }
  return 0; // Error not a valid address.
}

// This is the interface to the rest of the World.  Here
// the CPU or anyother devece writes to memory and updates the
// device.  Poke Changes the value of the registers.
// Notice that there is no way for the user to WRITE to the
// registers CNTR, this is because they are read only.
void Timer::Poke(Address address, Byte c) {
  switch ((address - baseAddress)) {
  case TCR:
    timerValue[TCR] = c;
    break;
  case TIVR:
    timerValue[TIVR] = c;
    break;
  case NULR:
    timerValue[NULR] = c;
    break;
  case CPRH:
    timerValue[CPRH] = c;
    break;
  case CPRM:
    timerValue[CPRM] = c;
    break;
  case CPRL:
    timerValue[CPRL] = c;
    break;
  case TSR:
    timerValue[TSR] = c;
    break;
  }
}

// Resets the timer and Initalizes the registers.
void Timer::Reset() {
  // Resets the interupt handler.
  myInterruptPending = false;
  firstTime = true;

  // Clears the timers.
  for (int i = 0; i < 22; i++)
    timerValue[i] = 0;
}

// Function that executes when interupt occures.
// Requires decCNTR copyCPRtoCNTR.
void Timer::EventCallback(int data, void *ptr) {
  Byte cTCR = timerValue[TCR];

  // Test to see if timer is enabled, if False(0) do nothing.
  // but dispatch an event.
  if ((cTCR & 1) == 0) {
    myCPU.eventHandler().Add(this, TIMER_EVENT, 0, TIMER_CPU_CYCLE);
    return;
  }

  // This test determines if the timer has started a count down.
  // If it hasn't, copy the CPR register values to the CNTR registers.
  if (firstTime == true) {
    firstTime = false;
    copyCPRtoCNTR();
  }

  // Decrements the CNTR if 0 then call interrupt.
  // else dispatch an event.
  if (decCNTR() != 0) {
    myCPU.eventHandler().Add(this, TIMER_EVENT, 0, TIMER_CPU_CYCLE);
    return;
  }

  // Since the timer is now done, set the first time to true,
  // so the next time the TCR is set to 1 it will copy the
  // CPR Register to CNTR.
  firstTime = true;

  // Protocal says that the TCR should flip to 0 and that
  // TSR bit switches to 1. That is done below.
  timerValue[TCR] = (timerValue[TCR] & 0xfe);
  timerValue[TSR] = (timerValue[TSR] | 0x01);

  // Dispatch an IRQ only if user has set this in the
  // TCR register. (5 and 7) are the only valid
  // Interrupts.
  if (((cTCR >> 5) & 5) || ((cTCR >> 5) & 7)) {
    InterruptRequest(TIMER_IRQ);
  }
  myCPU.eventHandler().Add(this, TIMER_EVENT, 0, TIMER_CPU_CYCLE);
}

// Copies the register value of CPR to CNTR.  Used by EventCallback
void Timer::copyCPRtoCNTR() {
  timerValue[CNTRH] = timerValue[CPRH];
  timerValue[CNTRM] = timerValue[CPRM];
  timerValue[CNTRL] = timerValue[CPRL];
}

// Decriments the counter and checks for CNTR==0
// Retuns the value of CNTR.
unsigned int Timer::decCNTR() {
  // Grab the values of the 24bit register to a value.
  unsigned int decValue = (timerValue[CNTRH] << 24) |
                          (timerValue[CNTRM] << 16) |
                          (timerValue[CNTRL] <<  0);
  if (decValue < 250) {
    decValue = 250;
  }
  decValue -= 250;

  // Put the value  back  the 24bit register to a value.
  // with the decremnt.
  timerValue[CNTRH] = (Byte)(decValue >> 24);
  timerValue[CNTRM] = (Byte)(decValue >> 16);
  timerValue[CNTRL] = (Byte)(decValue);

  return decValue;
}
