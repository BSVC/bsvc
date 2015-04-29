// The base class for all devices

#ifndef FRAMEWORK_BASICDEVICE_HPP_
#define FRAMEWORK_BASICDEVICE_HPP_

#include <string>

#include "Framework/Types.hpp"
#include "Framework/Event.hpp"

class BasicCPU;

constexpr int AUTOVECTOR_INTERRUPT = -1;
constexpr int SPURIOUS_INTERRUPT = -2;

class BasicDevice : public EventBase {
public:
  BasicDevice(const std::string &name, const std::string &arguments, BasicCPU &cpu);
  ~BasicDevice() override;

  void ErrorMessage(const std::string &message) { myErrorMessage = message; }
  const std::string &ErrorMessage() const { return myErrorMessage; }
  const std::string &Name() const { return myName; }
  BasicCPU &CPU() const { return myCPU; }
  const std::string &Arguments() const { return myArguments; }

  // Returns true iff the address maps into the device.
  virtual bool CheckMapped(Address) const = 0;

  // Returns the lowest address used by the device.
  virtual Address LowestAddress() const = 0;

  // Returns the highest address used by the device.
  virtual Address HighestAddress() const = 0;

  // Gets a byte from the device.
  virtual Byte Peek(Address address) = 0;

  // Puts a byte into the device.
  virtual void Poke(Address address, Byte c) = 0;

  // Gets data from the device.
  virtual bool Peek(Address address, unsigned long &data, int size);

  // Puts data into the device.
  virtual bool Poke(Address address, unsigned long data, int size);

  // Resets the device.
  virtual void Reset();

  // Sends an interrupt request (IRQ) to the CPU.
  virtual void InterruptRequest(int level);

  // Called by the CPU when it processes an interrupt.
  virtual int InterruptAcknowledge(int level);

protected:
  // Reference to the CPU I belong to.
  BasicCPU &myCPU;

  // My name (i.e. RAM, ROM, etc).
  const std::string myName;

  // Arguments passed to constructor.
  const std::string myArguments;

  // Error that occured during construction.
  std::string myErrorMessage;

  // Interrupt pending flag.
  bool myInterruptPending;
};

#endif  // FRAMEWORK_BASICDEVICE_HPP_
