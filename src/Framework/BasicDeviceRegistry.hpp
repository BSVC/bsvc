//
// This abstract base class is used to derive a class that maintains a list
// of all the device in the simulator and allows them to be created.
//

#ifndef FRAMEWORK_BASICDEVICEREGISTRY_HPP_
#define FRAMEWORK_BASICDEVICEREGISTRY_HPP_

#include <string>

class BasicCPU;
class BasicDevice;

// Device information structure.
struct DeviceInformation {
  std::string name;         // The name of the device ("RAM", "M68681", etc).
  std::string description;  // A short description of the device.
  std::string script;       // UI script to get the device attachment args.
};

class BasicDeviceRegistry {
public:
  BasicDeviceRegistry(const DeviceInformation *devices, unsigned int number)
      : myDevices(devices), myNumberOfDevices(number) { }
  virtual ~BasicDeviceRegistry() { }

  // Returns the number of devices
  unsigned int NumberOfDevices() { return myNumberOfDevices; }

  // Gets device information. Returns true iff successful.
  bool Information(size_t index, DeviceInformation &information);

  // Creates a device. Returns true iff successful.
  virtual bool Create(const std::string &name,
                      const std::string &args,
                      BasicCPU &cpu,
                      BasicDevice *&device) = 0;

private:
  // Array of devices in the simulator.
  const DeviceInformation *myDevices;

  // Number of devices in the simulator.
  const unsigned int myNumberOfDevices;
};

#endif  // FRAMEWORK_BASICDEVICEREGISTRY_HPP_
