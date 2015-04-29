///////////////////////////////////////////////////////////////////////////////
//
// StatInfo.hxx
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
// $Id: StatInfo.hxx,v 1.1 1996/08/02 14:53:12 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef STATINFO_HXX
#define STATINFO_HXX

#include <string>
#include <list>

#ifdef USE_STD
  using namespace std;
#endif

class BasicCPU;

///////////////////////////////////////////////////////////////////////////////
// The Statistic Information Class
///////////////////////////////////////////////////////////////////////////////
class StatisticInformation {
  public:
    // Constructor
    StatisticInformation(const string& statistic)
        : myStatistic(statistic)
    {}

    // Default constructor
    StatisticInformation()
    {}

    // Destructor
    ~StatisticInformation()
    {}

    // Set the statistic fields
    void Set(const string& statistic)
    { myStatistic = statistic; }

    // Answer my statistic
    string Statistic() const { return myStatistic; }

  private:
    string myStatistic;
};


///////////////////////////////////////////////////////////////////////////////
// The Statistical Information List Class
///////////////////////////////////////////////////////////////////////////////
class StatisticalInformationList {
  public:
    // Constructor
    StatisticalInformationList(BasicCPU& cpu);

    // Destructor
    ~StatisticalInformationList();

    // Append an element to the end of the list
    void Append(const string& statistic);

    // Answer the number of elements in the list
    unsigned int NumberOfElements() const { return myList.size(); }

    // Get the element with the given index. Answer true iff successful
    bool Element(unsigned int index, StatisticInformation& info);

  private:
    list<StatisticInformation*> myList;
};
#endif

