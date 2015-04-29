///////////////////////////////////////////////////////////////////////////////
//
// RAM.cxx 
//
// The Random Access Memory Device
//
// Sim68000 "Motorola 68000 Simulator"
// Copyright (c) 1993
// By: Bradford W. Mott
// July 26,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: RAM.cxx,v 1.1 1996/08/02 15:02:49 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
  #include <iostream>
  #include <strstream>
#else
  #include <iostream.h>
  #include <strstream.h>
#endif

#ifdef USE_STD
  using namespace std;
#endif

#include "Tools.hxx"
#include "BasicCPU.hxx"
#include "RAM.hxx"

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
RAM::RAM(const string& args, BasicCPU& cpu)
    : BasicDevice("RAM", args, cpu)
{
  istrstream in((char*)args.data(), args.length());
  string keyword, equals;
  unsigned long base, size;

  // Scan "BaseAddress = nnnn"
  in >> keyword >> equals >> hex >> base;
  if((!in) || (keyword != "BaseAddress") || (equals != "="))
  {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }

  // Scan "Size = nnnn"
  in >> keyword >> equals >> hex >> size;
  if((!in) || (keyword != "Size") || (equals != "="))
  {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }
 
  myBaseAddress = base * cpu.Granularity();
  mySize = size * cpu.Granularity();

  if(mySize > 0)
    myBuffer = new unsigned char[mySize];
  else
    myBuffer = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
RAM::~RAM()
{
  delete[] myBuffer;
}

///////////////////////////////////////////////////////////////////////////////
// Checks to see if the address maps into this device
///////////////////////////////////////////////////////////////////////////////
bool RAM::CheckMapped(unsigned long address) const
{
  return (address >= myBaseAddress) && (address < myBaseAddress + mySize);
}

