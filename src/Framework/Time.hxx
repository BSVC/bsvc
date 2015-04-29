///////////////////////////////////////////////////////////////////////////////
//
// Time.hxx
//
//   The Time object provides access to the system's time. 
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// August 12,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: Time.hxx,v 1.1 1996/08/02 14:53:40 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef TIME_HXX
#define TIME_HXX

class Time {
  public:
    Time()
    {}

    // Answers the current system time in seconds
    long seconds();
};
#endif

