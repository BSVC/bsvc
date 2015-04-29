//
// Maintains a list of devices and provides methods to
// peek and poke into them.
//

#ifndef FRAMEWORK_ADDRESSSPACE_HPP_
#define FRAMEWORK_ADDRESSSPACE_HPP_

#include <string>
#include <vector>

#include "Framework/Types.hpp"

class BasicDevice;

// Size Constants
enum {
  BYTE,
  WORD,
  LONG,
};

class AddressSpace {
public:
  // Used to retrieve information about attached devices.
  struct DeviceInformation {
    std::string name;
    std::string arguments;
    size_t index;
  };

public:
  AddressSpace(Address maximumAddress);
  virtual ~AddressSpace();

  // Returns the maximum address of the address space.
  Address MaximumAddress() const { return myMaximumAddress; }

  // Attaches the given device. Returns true iff successful.
  bool AttachDevice(BasicDevice *);

  // Detaches and destroys the indexed device.  Returns true iff successful.
  bool DetachDevice(size_t index);

  // Resets all the attached devices.
  void Reset();

  // Returns the number of attached devices
  size_t NumberOfAttachedDevices() const;

  // Gets information about the indexed device.  Returns true iff successful.
  bool GetDeviceInformation(size_t index,
                            AddressSpace::DeviceInformation &info) const;

  // Peeks the given location.  Returns true iff successful.
  virtual bool Peek(Address addr, Byte &c);

  // Pokes the given location.  Returns true iff successful.
  virtual bool Poke(Address addr, Byte c);

  // Peeks the given location.  Returns true iff successful.
  virtual bool Peek(Address addr, unsigned long &d, int size);

  // Pokes the given location.  Returns true iff successful.
  virtual bool Poke(Address addr, unsigned long d, int size);

private:
  BasicDevice *FindCachedDevice(Address address,
                                std::vector<BasicDevice *> &cache);
  BasicDevice *FindReadDevice(Address address);
  BasicDevice *FindWriteDevice(Address address);

  // Attached devices.
  std::vector<BasicDevice *> devices;

  // Maximum address for this address space in CPU words (not bytes).
  const Address myMaximumAddress;

  // Device caches.
  std::vector<BasicDevice *> rcache;
  std::vector<BasicDevice *> wcache;
};

#endif  // FRAMEWORK_ADDRESSSPACE_HPP_
