///////////////////////////////////////////////////////////////////////////////
//
// RegInfo.cxx
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
// $Id: RegInfo.cxx,v 1.1 1996/08/02 14:52:47 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "BasicCPU.hxx"
#include "RegInfo.hxx"

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
RegisterInformationList::RegisterInformationList(BasicCPU& cpu)
{
  // Ask CPU to fill me up with RegisterInformation objects
  cpu.BuildRegisterInformationList(*this);
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
RegisterInformationList::~RegisterInformationList()
{
  // Keep freeing items until no more exist
  while(myList.size() != 0)
  {
    RegisterInformation* info = myList.back();
    delete info;
    myList.pop_back();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Append an element to the end of the list
///////////////////////////////////////////////////////////////////////////////
void RegisterInformationList::Append(const string& name, 
    const string& hexValue, const string& desc)
{
  RegisterInformation* i = new RegisterInformation(name, hexValue, desc);
  myList.push_back(i);
}

///////////////////////////////////////////////////////////////////////////////
// Get the element with the given index. Answer true iff successful
///////////////////////////////////////////////////////////////////////////////
bool RegisterInformationList::Element(unsigned int index, 
    RegisterInformation& info)
{
  unsigned int t;
  list<RegisterInformation*>::iterator i;

  if(index < NumberOfElements())
  {
    for(t = 0, i = myList.begin(); t < index; ++t, ++i);
    info.Set((*i)->Name(), (*i)->HexValue(), (*i)->Description());
    return true;
  } 
  else
  {
    return false;
  }
}

