//
// Random Access Memory Device.
//

#ifndef M68K_DEVICES_RAM_HPP_
#define M68K_DEVICES_RAM_HPP_

#include <string>

#include "Framework/BasicDevice.hpp"

class RAM : public BasicDevice {
public:
  RAM(const std::string &args, BasicCPU &cpu);
  ~RAM();

  // Returns true iff the address maps into the device.
  bool CheckMapped(Address addr) const;

  // Returns the lowest address used by the device.
  Address LowestAddress() const { return myBaseAddress; }

  // Returns the highest address used by the device.
  Address HighestAddress() const { return myBaseAddress + mySize - 1; }

  // Gets a byte from memory.
  Byte Peek(Address address) {
    if (address < LowestAddress() || HighestAddress() < address) {
      return 0xFF;
    }
    return myBuffer[address - myBaseAddress];
  }

  // Puts a byte into memory.
  virtual void Poke(Address address, Byte c) {
    if (LowestAddress() <= address && address <=  HighestAddress()) {
      myBuffer[address - myBaseAddress] = c;
    }
  }

  // RAM never has Events
  void EventCallback(int, void *) { }

protected:
  // Buffer to hold the RAM's contents.
  Byte *myBuffer;

private:
  // Starting address of the RAM device
  Address myBaseAddress;

  // Size of the RAM device in bytes
  size_t mySize;
};

#endif  // M68K_DEVICES_RAM_HPP_
