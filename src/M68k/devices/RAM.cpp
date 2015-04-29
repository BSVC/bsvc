#include <ios>
#include <sstream>

#include "Framework/Tools.hpp"
#include "Framework/BasicCPU.hpp"
#include "M68k/devices/RAM.hpp"

RAM::RAM(const std::string &args, BasicCPU &cpu)
    : BasicDevice("RAM", args, cpu) {
  std::istringstream in(args);
  std::string keyword, equals;
  Address base;
  size_t size;

  // Scan "BaseAddress = nnnn"
  in >> keyword >> equals >> std::hex >> base;
  if ((!in) || (keyword != "BaseAddress") || (equals != "=")) {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }

  // Scan "Size = nnnn"
  in >> keyword >> equals >> std::hex >> size;
  if ((!in) || (keyword != "Size") || (equals != "=")) {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }

  myBaseAddress = base * cpu.Granularity();
  mySize = size * cpu.Granularity();

  myBuffer = nullptr;
  if (mySize > 0)
    myBuffer = new unsigned char[mySize];
}

RAM::~RAM() { delete[] myBuffer; }

bool RAM::CheckMapped(Address address) const {
  return (address >= myBaseAddress) && (address < myBaseAddress + mySize);
}
