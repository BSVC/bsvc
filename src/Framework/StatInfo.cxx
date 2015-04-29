///////////////////////////////////////////////////////////////////////////////
//
// StatInfo.cxx
//
//   This class is used by BasicCPU (and derived classes) to manage a list of
// of statistics objects.
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// December 5,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: StatInfo.cxx,v 1.1 1996/08/02 14:53:21 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "BasicCPU.hxx"
#include "StatInfo.hxx"

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
StatisticalInformationList::StatisticalInformationList(BasicCPU& cpu)
{
  // Ask the CPU to fill me up with StatisticInformation objects
  cpu.BuildStatisticalInformationList(*this);
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
StatisticalInformationList::~StatisticalInformationList()
{
  // Keep freeing items until no more exist
  while(myList.size() != 0)
  {
    StatisticInformation* info = myList.back();
    delete info;
    myList.pop_back();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Append an element to the end of the list
///////////////////////////////////////////////////////////////////////////////
void StatisticalInformationList::Append(const string& statistic)
{
  StatisticInformation* i = new StatisticInformation(statistic);
  myList.push_back(i);
}

///////////////////////////////////////////////////////////////////////////////
// Get the element with the given index. Answer true iff successful
///////////////////////////////////////////////////////////////////////////////
bool StatisticalInformationList::Element(unsigned int index, 
    StatisticInformation& info)
{
  unsigned int t;
  list<StatisticInformation*>::iterator i;

  if(index < NumberOfElements())
  {
    for(t = 0, i = myList.begin(); t < index; ++t, ++i);
    info.Set((*i)->Statistic());
    return true;
  }
  else
  {
    return false;
  }
}

