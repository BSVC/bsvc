///////////////////////////////////////////////////////////////////////////////
//
// Loader.hxx
//
// Load object files
//
// Sim68000 "Motorola 68000 Simulator"
// Copyright (c) 1993
// By: Bradford W. Mott
// November 5,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: Loader.hxx,v 1.1 1996/08/02 15:03:43 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef LOADER_HXX
#define LOADER_HXX

#ifdef WIN32
  #include <iostream>
  #include <fstream>
#else
  #include <iostream.h>
  #include <fstream.h>
#endif

#ifdef USE_STD
  using namespace std;
#endif

#include "BasicLoader.hxx"

///////////////////////////////////////////////////////////////////////////////
// The Loader Class
///////////////////////////////////////////////////////////////////////////////
class Loader : public BasicLoader {
  public:
    Loader(BasicCPU& c)
        : BasicLoader(c)
    {}

    // Loads the named file and answers an error message or the empty string
    string Load(const char *filename, int addressSpace);

  private:
    // Load in a Motorola S-Record file
    string LoadMotorolaSRecord(fstream& file, int addressSpace);
};
#endif

