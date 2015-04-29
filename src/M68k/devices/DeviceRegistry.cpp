#include <string>

#include "M68k/devices/DeviceRegistry.hpp"
#include "M68k/devices/Gdbsock.hpp"
#include "M68k/devices/M68681.hpp"
#include "M68k/devices/RAM.hpp"
#include "M68k/devices/Timer.hpp"

// Array of device information (name, description, tcl script).
const DeviceInformation DeviceRegistry::ourDeviceInfo[] = {
    {
     "RAM", "Random Access Memory",
#include "M68k/devices/RAM.scr"
    },
    {
     "GdbSocket", "Socket for connecting gdb",
#include "M68k/devices/Gdbsock.scr"
    },
    {
     "M68681", "Motorola 68681 Dual UART",
#include "M68k/devices/M68681.scr"
    },
    {
     "Timer", "Timer",
#include "M68k/devices/Timer.scr"
    }
};

DeviceRegistry::DeviceRegistry()
    : BasicDeviceRegistry(ourDeviceInfo,
                          sizeof(ourDeviceInfo) / sizeof(DeviceInformation)) {}

bool DeviceRegistry::Create(const std::string &name, const std::string &args,
                            BasicCPU &cpu, BasicDevice *&device) {
  device = nullptr;
  if (name == "RAM")
    device = new RAM(args, cpu);
  else if (name == "GdbSocket")
    device = new GdbSocket(args, cpu);
  else if (name == "M68681")
    device = new M68681(args, cpu);
  else if (name == "Timer")
    device = new Timer(args, cpu);

  // If the device does not exist or it's error message is not
  // empty then return an error.
  if (device == nullptr || !device->ErrorMessage().empty()) {
    delete device;
    return false;
  }
  return true;
}
