///////////////////////////////////////////////////////////////////////////////
//
// Loader.cxx
//
//   Class to load object files
//
// Sim68000 "Motorola 68000 Simulator"
// Copyright (c) 1993
// By: Bradford W. Mott
// November 5,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: Loader.cxx,v 1.1 1996/08/06 11:53:58 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "AddressSpace.hxx"
#include "BasicCPU.hxx"
#include "Tools.hxx"
#include "Loader.hxx"

///////////////////////////////////////////////////////////////////////////////
// Loads the named file and answers an error message or the empty string
///////////////////////////////////////////////////////////////////////////////
string Loader::Load(const char* filename, int addressSpace)
{
  string error;

  // Open the file for reading
  #ifdef USE_STD
    fstream file(filename, ios::in);
  #else
    fstream file(filename, ios::in | ios::nocreate);
  #endif

  // Make sure the file was opened
  if(file.fail())
  {
    return "ERROR: Could not open file!!!";
  }

  // Try to load a motorola S-record file
  return LoadMotorolaSRecord(file, addressSpace);
}

///////////////////////////////////////////////////////////////////////////////
// Load in a Motorola S-Record file into an address space
///////////////////////////////////////////////////////////////////////////////
string Loader::LoadMotorolaSRecord(fstream& file, int addressSpace)
{
  unsigned long address;
  string line;
  int t, length, byte;
 
  while(!file.eof() && file.good())
  {
    file >> line;
    if(line.length() <= 0)
    {
      return("ERROR: Bad line in file!!!");
    }
    else if(line[0] != 'S')
    {
      return("ERROR: Incorrect file format!!!");
    }
    else if(line[1] == '1')
    {
      line.replace(0, 2, "");
      length = StringToInt(string(line, 0, 2));
      line.replace(0, 2, "");
      address = StringToInt(string(line, 0, 4));
      line.replace(0, 4, "");
      for(t = 0; t < length - 3; ++t)
      {
        byte = StringToInt(string(line, 0, 2));
        line.replace(0, 2, "");
        myCPU.addressSpace(addressSpace).Poke(address + t,
            (unsigned char)byte);
      }
    }
    else if(line[1] == '2')
    {
      line.replace(0, 2, "");
      length = StringToInt(string(line, 0, 2));
      line.replace(0, 2, "");
      address = StringToInt(string(line, 0, 6));
      line.replace(0, 6, "");
      for(t = 0; t < length - 4; ++t)
      {
        byte = StringToInt(string(line, 0, 2));
        line.replace(0, 2, "");
        myCPU.addressSpace(addressSpace).Poke(address + t,
            (unsigned char)byte);
      }
    }
    else if(line[1] == '3')
    {
      line.replace(0, 2, "");
      length = StringToInt(string(line, 0, 2));
      line.replace(0, 2, "");
      address = StringToInt(string(line, 0, 8));
      line.replace(0, 8, "");
      for(t = 0; t < length - 5; ++t)
      {
        byte = StringToInt(string(line, 0, 2));
        line.replace(0, 2, "");
        myCPU.addressSpace(addressSpace).Poke(address + t, 
            (unsigned char)byte);
      }
    }
    else if(line[1] == '7')
    {
      break;
    }
    else if(line[1] == '8')
    {
      break;
    }
    else if(line[1] == '9')
    {
      break;
    }
  }
  return "";
}  

