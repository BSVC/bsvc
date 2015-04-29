///////////////////////////////////////////////////////////////////////////////
//
// RegInfo.hxx
//
//   This class is used by BasicCPU (and derived classes) to manage a list of
// of register structures. 
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// October 25,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: RegInfo.hxx,v 1.1 1996/08/02 14:52:55 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef REGINFO_HXX
#define REGINFO_HXX

#include <string>
#include <list>

#ifdef USE_STD
  using namespace std;
#endif

class BasicCPU;

///////////////////////////////////////////////////////////////////////////////
// RegisterInformation class declaration
///////////////////////////////////////////////////////////////////////////////
class RegisterInformation {
  public:
    // Constructor
    RegisterInformation(const string& name, const string& hexValue,
        const string& description)
        : myName(name),
          myHexValue(hexValue),
          myDescription(description)
    {}

    // Default Construtor
    RegisterInformation()
    {}

    // Destructor
    ~RegisterInformation()
    {}

    // Set the name, hex value, and the description fields
    void Set(const string& name, const string& hexValue,
        const string& description)
    { myName = name; myHexValue = hexValue; myDescription = description; }

    string Name() const { return myName; }
    string HexValue() const { return myHexValue; }
    string Description() const { return myDescription; }

  private:
    string myName;           // The name given to the register ("D0", "PC", etc)
    string myHexValue;       // The value of the register in hexidecimal
    string myDescription;    // A short description of the register
};


///////////////////////////////////////////////////////////////////////////////
// RegisterInformationList class declaration
///////////////////////////////////////////////////////////////////////////////
class RegisterInformationList {
  public:
    // Constructor
    RegisterInformationList(BasicCPU& cpu);

    // Destructor
    ~RegisterInformationList();

    // Append an element to the end of the list
    void Append(const string& name, const string& hexValue, const string& desc);

    // Return the number of elements in the list
    unsigned int NumberOfElements() const { return myList.size(); }

    // Get the element with the given index. Answer true iff successful
    bool Element(unsigned int index, RegisterInformation& info);

  private:
    list<RegisterInformation*> myList;
};
#endif

