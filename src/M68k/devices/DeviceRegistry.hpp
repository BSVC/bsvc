//
// Keeps a list of all of the availible devices and allocates them
// on demand. Dervied from the BasicDeviceRegistry.
//

#ifndef M68K_DEVICES_DEVICEREGISTRY_HPP_
#define M68K_DEVICES_DEVICEREGISTRY_HPP_

#include "Framework/BasicDeviceRegistry.hpp"

class DeviceRegistry : public BasicDeviceRegistry {
public:
  DeviceRegistry();

  // Creates a device with the given name.  Returns true iff successful.
  bool Create(const std::string &name, const std::string &args,
              BasicCPU &cpu, BasicDevice *&device);

private:
  static const DeviceInformation ourDeviceInfo[];
};

#endif  // M68K_DEVICES_DEVICEREGISTRY_HPP_
