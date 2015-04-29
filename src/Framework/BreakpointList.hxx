///////////////////////////////////////////////////////////////////////////////
//
// BreakpointList.hxx 
//
//   This class manages a set of breakpoints
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// November 23,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: BreakpointList.hxx,v 1.1 1996/08/02 14:51:29 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef BREAKPOINTLIST_HXX
#define BREAKPOINTLIST_HXX

#include <set>

#ifdef USE_STD
  using namespace std;
#endif

class BreakpointList {
  private:
    // My set of breakpoints 
    set<unsigned long, less<unsigned long> > myBreakpoints;

  public:
    // Constructor
    BreakpointList();

    // Add a break point to the list
    void Add(unsigned long address);

    // Delete a break point from the list. Answers true iff successful
    bool Delete(unsigned long address);

    // Answers the number of breakpoints
    unsigned int NumberOfBreakpoints() const;

    // Get the breakpoint with the given index. Answers true iff successful
    bool GetBreakpoint(unsigned int index, unsigned long& address) const;

    // Answers true iff the given address is a breakpoint 
    bool Check(unsigned long address) const;
};
#endif

