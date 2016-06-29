// Instantiates all of the objects and starts the user interface command parser.

#include <memory>

#include "Framework/Interface.hpp"
#include "M68k/sim68360/cpu32.hpp"
#include "M68k/devices/DeviceRegistry.hpp"
#include "M68k/loader/Loader.hpp"

int main() {
  auto processor = std::unique_ptr<BasicCPU>(new cpu32);
  auto loader = std::unique_ptr<BasicLoader>(new Loader(*processor));
  auto registry = std::unique_ptr<BasicDeviceRegistry>(new DeviceRegistry);

  Interface interface(*processor, *registry, *loader);
  interface.CommandLoop();

  return 0;
}
