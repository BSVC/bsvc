#include "Framework/BasicDeviceRegistry.hpp"

bool BasicDeviceRegistry::Information(size_t i, DeviceInformation &info) {
  if (i < myNumberOfDevices) {
    info = myDevices[i];
    return true;
  }
  return false;
}
