#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>

#include "Framework/Tools.hpp"

// Convert the hex string to an unsigned integer
unsigned int StringToInt(const std::string &hex) {
  return static_cast<unsigned int>(std::strtoul(hex.c_str(), NULL, 16));
}

// Convert the value to a hex string padded with zeros
std::string IntToString(unsigned int value, unsigned int width) {
  std::ostringstream oss;
  oss << std::setw(width) << std::setfill('0') << std::hex << value;
  auto s = oss.str();
  return s.substr(s.length() - width);
}
