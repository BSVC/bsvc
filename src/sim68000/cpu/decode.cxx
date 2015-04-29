///////////////////////////////////////////////////////////////////////////////
//
// decode.cxx 
//
// The Motorola 68000 instruction decoding stuff
//
// Sim68000 "Motorola 68000 Simulator"
// Copyright (c) 1993
// By: Bradford W. Mott
// November 3,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: decode.cxx,v 1.1 1996/08/02 14:59:00 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "m68000.hxx"

///////////////////////////////////////////////////////////////////////////////
// Table used to decode 68000 opcodes
///////////////////////////////////////////////////////////////////////////////
m68000::DecodeEntry m68000::ourDecodeTable[] = {
  #include "m68000DecodeTable.hxx"
};

///////////////////////////////////////////////////////////////////////////////
// Used to cache opcodes once they've been decoded
///////////////////////////////////////////////////////////////////////////////
m68000::ExecutionPointer* m68000::ourDecodeCacheTable = 0;

///////////////////////////////////////////////////////////////////////////////
// Decode the given opcode
///////////////////////////////////////////////////////////////////////////////
m68000::ExecutionPointer m68000::DecodeInstruction(int opcode)
{
  // Check to see if this opcode needs to be decoded
  if(ourDecodeCacheTable[opcode] == 0)
  {
    // Decode the opcode using a linear search (slow :-{ )
    for(unsigned int s = 0; s < (sizeof(ourDecodeTable) / sizeof(DecodeEntry));
        ++s)
    {
      if((opcode & ourDecodeTable[s].mask) == ourDecodeTable[s].signature )
      {
        ourDecodeCacheTable[opcode] = ourDecodeTable[s].execute;
      }
    }

    // If not found then it's an invalid instruction
    if(ourDecodeCacheTable[opcode] == 0)
      ourDecodeCacheTable[opcode] = &m68000::ExecuteInvalid;
  }
  return ourDecodeCacheTable[opcode]; 
}


