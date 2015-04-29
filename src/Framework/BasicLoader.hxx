///////////////////////////////////////////////////////////////////////////////
//
// BasicLoader.hxx
//
//   This abstract base class provides methods to load object files into the
// the simulator.
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// November 5,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: BasicLoader.hxx,v 1.1 1996/08/02 14:51:02 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef BASICLOADER_HXX
#define BASICLOADER_HXX

#include <string>

#ifdef USE_STD
  using namespace std;
#endif

class BasicCPU;

///////////////////////////////////////////////////////////////////////////////
// BasicLoader class declaration
///////////////////////////////////////////////////////////////////////////////
class BasicLoader {
  public:
    BasicLoader(BasicCPU& c)
        : myCPU(c)
    {}

    virtual ~BasicLoader()
    {}

    // Answer my CPU
    BasicCPU& CPU() { return myCPU; }

    // Loads the named file and answers an error message or the empty string
    virtual string Load(const char *filename, int addressSpace) = 0;

  protected:
    BasicCPU& myCPU;
};
#endif

