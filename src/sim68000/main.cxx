///////////////////////////////////////////////////////////////////////////////
//
// main.cxx 
//
//   The main procedure has to instantiate all of the objects and startup the
// user interface command parser
//
// Sim68000 "Motorola 68000 Simulator"
// Copyright (c) 1993
// By: Bradford W. Mott
// Novemeber 13,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: main.cxx,v 1.1 1996/08/02 14:58:12 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "Interface.hxx"
#include "cpu/m68000.hxx"
#include "devices/DeviceRegistry.hxx"
#include "loader/Loader.hxx"

main()
{
  BasicCPU& processor = *(new m68000);
  BasicLoader& loader = *(new Loader(processor));
  BasicDeviceRegistry& registry = *(new DeviceRegistry);

  Interface interface(processor, registry, loader);
  interface.CommandLoop();

  delete &registry;
  delete &loader;
  delete &processor;
}

