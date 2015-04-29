#include <fstream>

#include "Framework/Types.hpp"
#include "Framework/AddressSpace.hpp"
#include "Framework/BasicCPU.hpp"
#include "Framework/Tools.hpp"
#include "M68k/loader/Loader.hpp"

// Loads the named file and answers an error message or the empty string
std::string Loader::Load(const std::string &filename, int addressSpace) {
  std::ifstream file(filename);
  if (file.fail()) {
    return "ERROR: Could not open file!!!";
  }
  return LoadMotorolaSRecord(file, addressSpace);
}

std::string Cut(size_t n, std::string &line) {
  std::string cut = line.substr(0, n);
  line.erase(0, n);
  return cut;
}

// Load in a Motorola S-Record file into an address space
std::string Loader::LoadMotorolaSRecord(std::ifstream &file, int addressSpace) {
  Address address;
  int length;
  Byte byte;
  std::string line;

  while (!file.eof() && file.good()) {
    file >> line;
    if (line.length() <= 0) {
      return "ERROR: Bad line in file!!!";
    }
    if (Cut(1, line) != "S") {
      return "ERROR: Incorrect file format!!!";
    }
    std::string line1 = Cut(1, line);
    if (line1 == "1") {
      length = StringToInt(Cut(2, line));
      address = StringToInt(Cut(4, line));
      for (int k = 0; k < length - 3; ++k) {
        byte = StringToInt(Cut(2, line));
        myCPU.addressSpace(addressSpace).Poke(address + k, byte);
      }
    } else if (line1 == "2") {
      length = StringToInt(Cut(2, line));
      address = StringToInt(Cut(6, line));
      for (int k = 0; k < length - 4; ++k) {
        byte = StringToInt(Cut(2, line));
        myCPU.addressSpace(addressSpace).Poke(address + k, byte);
      }
    } else if (line1 == "3") {
      length = StringToInt(Cut(2, line));
      address = StringToInt(Cut(8, line));
      for (int k = 0; k < length - 5; ++k) {
        byte = StringToInt(Cut(2, line));
        myCPU.addressSpace(addressSpace).Poke(address + k, byte);
      }
    } else if (line1 == "7" || line1 == "8" || line1 == "9") {
      break;
    }
  }

  return "";
}
