#ifndef TOOLS_HXX
#define TOOLS_HXX

#include <string>

// Converts the hex string to an unsigned integer.
unsigned int StringToInt(const std::string &hex);

// Converts the value to a hex string padded with zeros.
// Width is a maximum, counted from the end of the string.
std::string IntToString(unsigned int value, unsigned int width);

#endif
