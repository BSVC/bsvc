//
// Simulates an ECB timer for the M68000 CPU. Thus giving
// the Simulator the ablility to simulate a timer interrupt.
// Programmers can implement time slicing with this timer.
//
// Contains Register and Offsets for the ECB timer.
//

#ifndef M68K_DEVICES_TIMER_HPP_
#define M68K_DEVICES_TIMER_HPP_

#include <string>

#include "Framework/Types.hpp"
#include "Framework/BasicDevice.hpp"

// Contants.
#define TCR 0
#define TIVR 2
#define NULR 4
#define CPRH 6
#define CPRM 8
#define CPRL 10   // 0xA  in Hex
#define NULTR 12  // 0xC  in hex
#define CNTRH 14
#define CNTRM 16
#define CNTRL 18
#define TSR 20    // 0x14 in hex

#define TIMER_EVENT 100

class Timer : public BasicDevice {
public:
  Timer(const std::string &args, BasicCPU &cpu);
  ~Timer() { };

  // Returns true iff the address maps into the device.
  bool CheckMapped(Address address) const override;

  // Returns the lowest address used by the device.
  Address LowestAddress() const override { return baseAddress; }

  // Returns the highest address used by the device.
  Address HighestAddress() const override {
    return (baseAddress + (15 * sizeof(char)));
  }

  // Gets a byte from the device.
  Byte Peek(Address address) override;

  // Puts a byte into the device.
  void Poke(Address address, Byte c) override;

  // Resets the device.
  void Reset() override;

  // Sends an interrupt request (IRQ) to the CPU.
  void InterruptRequest(int level) override;

  // Processes an interrupt when called by the CPU.
  int InterruptAcknowledge(unsigned int level) override;

  void EventCallback(int data, void *ptr) override;

protected:
  // Copies CPR register to CNTR.
  void copyCPRtoCNTR();

  // Decrements the CNTR and checks for 0.
  unsigned int decCNTR();

  // Interrupt pending flag.
  bool myInterruptPending;

  // Base location.
  Address baseAddress;

  // IRQ level set by the setup file.
  unsigned int TIMER_IRQ;

  // Time between event calls.
  unsigned int TIMER_CPU_CYCLE;

  // First time the caller is set.
  bool firstTime;

  // Timer value.
  Byte timerValue[22];
};

#endif  // M68K_DEVICES_TIMER_HPP_
