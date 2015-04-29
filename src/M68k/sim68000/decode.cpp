// Motorola 68000 instruction decoding.

#include "M68k/sim68000/m68000.hpp"

// Table to decode 68000 opcodes.
m68000::DecodeEntry m68000::ourDecodeTable[] = {
#include "M68k/sim68000/DecodeTable.hpp"
};

// Used to cache opcodes once they've been decoded
m68000::ExecutionPointer *m68000::ourDecodeCacheTable = nullptr;

// Decode the given opcode
m68000::ExecutionPointer m68000::DecodeInstruction(int opcode) {
  // Check to see if this opcode needs to be decoded
  if (ourDecodeCacheTable[opcode] == 0) {
    // Decode the opcode using a linear search (slow :-{ )
    for (size_t s = 0; s < (sizeof(ourDecodeTable) / sizeof(DecodeEntry)); ++s) {
      if ((opcode & ourDecodeTable[s].mask) == ourDecodeTable[s].signature) {
        ourDecodeCacheTable[opcode] = ourDecodeTable[s].execute;
      }
    }
    // If not found then it's an invalid instruction
    if (ourDecodeCacheTable[opcode] == 0)
      ourDecodeCacheTable[opcode] = &m68000::ExecuteInvalid;
  }
  return ourDecodeCacheTable[opcode];
}
