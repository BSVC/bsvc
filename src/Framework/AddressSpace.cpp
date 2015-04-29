#include <algorithm>

#include "Framework/AddressSpace.hpp"
#include "Framework/BasicCPU.hpp"
#include "Framework/BasicDevice.hpp"

AddressSpace::AddressSpace(Address maximumAddress)
    : myMaximumAddress(maximumAddress), rcache(3), wcache(3) { }

AddressSpace::~AddressSpace() {
  for (auto *device : devices) delete device;
}

// Attach the given device. Answers true iff successful
bool AddressSpace::AttachDevice(BasicDevice *device) {
  Address maxbytes = (MaximumAddress() + 1) * device->CPU().Granularity() - 1;

  // If highest address is <= maxbytes, and lowest address is <= highest,
  // then transitively lowest <= maxbytes so no need to check.
  if (device->HighestAddress() < device->LowestAddress() ||
      device->HighestAddress() > maxbytes) {
    delete device;
    return false;
  }
  devices.push_back(device);
  return true;
}

// Detach the indexed device and destroy it. Answers true iff successful
bool AddressSpace::DetachDevice(size_t index) {
  if (index >= devices.size()) {
    return false;
  }
  auto *device = devices[index];
  auto rit = std::find(rcache.rbegin(), rcache.rend(), device);
  if (rit != rcache.rend()) {
    *rit = nullptr;
    std::rotate(rcache.rbegin(), rit, rcache.rend());
  }
  auto wit = std::find(wcache.rbegin(), wcache.rend(), device);
  if (wit != wcache.rend()) {
    *wit = nullptr;
    std::rotate(wcache.rbegin(), wit, wcache.rend());
  }
  devices.erase(devices.begin() + index);
  delete device;
  return true;
}

// Reset all the attached devices
void AddressSpace::Reset() {
  for (auto *device : devices) {
    device->Reset();
  }
}

// Answers the number of attached devices
size_t AddressSpace::NumberOfAttachedDevices() const {
  return devices.size();
}

// Get information about the indexed device. Answer true iff successful
bool AddressSpace::GetDeviceInformation(
    size_t index, AddressSpace::DeviceInformation &info) const {
  if (index >= devices.size()) {
    return false;
  }
  auto *device = devices[index];
  info.name = device->Name();
  info.arguments = device->Arguments();
  info.index = index;
  return true;
}

BasicDevice *AddressSpace::FindCachedDevice(Address address,
                                            std::vector<BasicDevice *> &cache) {
  auto end = find(cache.begin(), cache.end(), nullptr);
  for (auto it = cache.begin(); it != end && *it != nullptr; ++it) {
    auto *device = *it;
    if (device->CheckMapped(address)) {
      std::rotate(cache.begin(), it, end);
      return device;
    }
  }
  auto it = std::find_if(devices.begin(), devices.end(),
                         [address](const BasicDevice *d) -> bool {
                             return d->CheckMapped(address);
                         });
  if (it == devices.end()) { return nullptr; }
  auto *device = *it;
  *cache.rbegin() = device;
  std::rotate(cache.rbegin(), cache.rbegin() + 1, cache.rend());
  return device;
}

// Find device for read operation (device cache).
BasicDevice *AddressSpace::FindReadDevice(Address address) {
  return FindCachedDevice(address, rcache);
}

// Find device for write operation (device cache).
BasicDevice *AddressSpace::FindWriteDevice(Address address) {
  return FindCachedDevice(address, wcache);
}

// Peek the given location.  Answers true iff successful
bool AddressSpace::Peek(Address addr, Byte &c) {
  BasicDevice *d = FindReadDevice(addr);

  // Did we find a device
  if (d == nullptr) {
    return false;  // Bus error.
  }

  // Get the byte
  c = d->Peek(addr);
  return true;
}

// Poke the given location.  Answers true iff successful
bool AddressSpace::Poke(Address addr, Byte c) {
  BasicDevice *d = FindWriteDevice(addr);

  // Did we find a device
  if (d == nullptr) {
    return false;  // Bus error.
  }

  // Put the byte
  d->Poke(addr, c);
  return true;
}


namespace {
bool IsMapped(const BasicDevice &device, Address address, int width) {
  for (int k = 0; k < width; ++k) {
    if (!device.CheckMapped(address + k)) {
      return false;
    }
  }
  return true;
}
}

// Peek a location in the address space with size parameter. Answers true iff
// successful.
bool AddressSpace::Peek(Address addr, unsigned long &data, int size) {
  Byte c = 0;
  int width = 1;
  if (size == WORD) width = 2;
  if (size == LONG) width = 4;

  if (size == BYTE) {
    if (!Peek(addr, c)) {
      return false;
    }
    data = c;
    return true;
  }

  BasicDevice *d = FindReadDevice(addr);
  if (d != nullptr && IsMapped(*d, addr, width)) {
    return !d->Peek(addr, data, size);
  }

  if (size == WORD) {
    // try per byte
    if (!Peek(addr, c))
      return false;
    data = c << 8;
    if (!Peek(addr + 1, c))
      return false;
    data += c;
    return true;
  }

  if (size == LONG) {
    // try per byte
    if (!Peek(addr, c))
      return false;
    data = c << 24;
    if (!Peek(addr + 1, c))
      return false;
    data += c << 16;
    if (!Peek(addr + 2, c))
      return false;
    data += c << 8;
    if (!Peek(addr + 3, c))
      return false;
    data += c;
    return true;
  }

  return false;
}

// Poke a location in the address space with size parameter. Answers true
// iff successful.
bool AddressSpace::Poke(Address addr, unsigned long data, int size) {
  Byte c = 0;
  int width = 1;
  if (size == WORD) width = 2;
  if (size == LONG) width = 4;

  if (size == BYTE) {
    if (!Poke(addr, c)) {
      return false;
    }
    data = c;
    return true;
  }

  BasicDevice *d = FindWriteDevice(addr);
  if (d != nullptr && IsMapped(*d, addr, width)) {
    return d->Poke(addr, data, size);
  }

  if (size == WORD) {
    if (!Poke(addr, (Byte)(data >> 8)))
      return false;
    return Poke(addr + 1, (Byte)data);
  }

  if (size == LONG) {
    // try per byte
    if (!Poke(addr, (Byte)(data >> 24)))
      return false;
    if (!Poke(addr + 1, (Byte)(data >> 16)))
      return false;
    if (!Poke(addr + 2, (Byte)(data >> 8)))
      return false;
    return Poke(addr + 3, (Byte)(data));
  }

  return false;
}
