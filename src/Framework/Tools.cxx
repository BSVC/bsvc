///////////////////////////////////////////////////////////////////////////////
//
// Tools.cxx 
//
//   This is a collection of useful routines 
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// November 6,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: Tools.cxx,v 1.1 1996/08/02 14:54:04 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "Tools.hxx"

///////////////////////////////////////////////////////////////////////////////
// Convert the hex string to an unsigned integer
///////////////////////////////////////////////////////////////////////////////
unsigned int StringToInt(const string& hex)
{
  static char digits[] = "0123456789abcdef";
  unsigned int result = 0;

  for(unsigned int t = 0; t < hex.length(); ++t)
  {
    unsigned int digit = 0;

    for(unsigned int s = 0; s < 16; ++s)
    {
      if(digits[s] == tolower(hex[t]))
      {
        digit = s;
        break;
      }
    }
    result = result * 16 + digit;
  }
  return(result);  
}

///////////////////////////////////////////////////////////////////////////////
// Convert the value to a hex string padded with zeros
///////////////////////////////////////////////////////////////////////////////
string IntToString(unsigned int value, unsigned int width)
{
  static char digits[] = "0123456789abcdef";
  string result;

  for(unsigned int t = 0; t < width; ++t)
    result += "0";

  for(int i = width - 1; (i >= 0) && (value > 0); --i)
  {
    result[i] = digits[value & 0x0f];
    value = value / 16;
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////////
// Convert the value to a decimal string with the given width
///////////////////////////////////////////////////////////////////////////////
string IntToDecimal(unsigned long value, unsigned int width)
{
  static char digits[] = "0123456789";
  string result;

  for(unsigned int t = 0; t < width; ++t)
    result += "0";

  for(int i = width - 1; (i >= 0) && (value > 0); --i)
  {
    result[i] = digits[value % 10];
    value = value / 10;
  }
  return result;
}

