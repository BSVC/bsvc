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
// Adapted to 68360 - DJ - 1996/11
// - m68000 renamed cpu32
//
///////////////////////////////////////////////////////////////////////////////
// $Id: decode.cxx,v 1.1 1996/08/02 14:59:00 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "cpu32.hxx"

///////////////////////////////////////////////////////////////////////////////
// Table used to decode 68xxx opcodes
///////////////////////////////////////////////////////////////////////////////
cpu32::DecodeEntry cpu32::ourDecodeTable[] = {
  #include "cpu32DecodeTable.hxx"
};

///////////////////////////////////////////////////////////////////////////////
// Used to cache opcodes once they've been decoded
///////////////////////////////////////////////////////////////////////////////
cpu32::ExecutionPointer* cpu32::ourDecodeCacheTable = 0;

///////////////////////////////////////////////////////////////////////////////
// Decode the given opcode
///////////////////////////////////////////////////////////////////////////////
cpu32::ExecutionPointer cpu32::DecodeInstruction(int opcode)
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
      ourDecodeCacheTable[opcode] = &cpu32::ExecuteInvalid;
  }
  return ourDecodeCacheTable[opcode]; 
}


