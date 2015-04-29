#include "M68k/sim68360/cpu32.hpp"

cpu32::DecodeEntry cpu32::ourDecodeTable[] = {
#include "M68k/sim68360/DecodeTable.hpp"
};

cpu32::ExecutionPointer *cpu32::ourDecodeCacheTable = nullptr;

cpu32::ExecutionPointer cpu32::DecodeInstruction(int opcode) {
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
      ourDecodeCacheTable[opcode] = &cpu32::ExecuteInvalid;
  }
  return ourDecodeCacheTable[opcode];
}
