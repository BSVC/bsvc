#include <string>

#include "Framework/AddressSpace.hpp"
#include "Framework/Tools.hpp"
#include "M68k/sim68360/cpu32.hpp"

int cpu32::ComputeEffectiveAddress(unsigned long &address, int &in_register,
                                   std::string &description, int mode_register,
                                   int size, int trace) {
  unsigned long tmp = 0;
  unsigned int extend_word;
  int status;
  int scale = 1;
  unsigned int displacement;

  switch (mode_register >> 3) {
  case 0: // Data Register Direct
    address = (mode_register & 0x7) + D0_INDEX;
    if (trace) {
      description += ourRegisterData[address].name;
    }
    in_register = 1;
    return (EXECUTE_OK);
    break;

  case 1: // Address Register Direct
    address = (mode_register & 0x7);
    if ((address == 7) && (register_value[SR_INDEX] & S_FLAG))
      address = SSP_INDEX;
    else
      address += A0_INDEX;
    if (trace) {
      description += ourRegisterData[address].name;
    }
    in_register = 1;
    return (EXECUTE_OK);
    break;

  case 2: // Address Register Indirect
    tmp = (mode_register & 0x7);
    if ((tmp == 7) && (register_value[SR_INDEX] & S_FLAG))
      tmp = SSP_INDEX;
    else
      tmp += A0_INDEX;
    address = register_value[tmp];
    if (trace) {
      description += "(";
      description += ourRegisterData[tmp].name;
      description += ")";
    }
    in_register = 0;
    return (EXECUTE_OK);
    break;

  case 3: // Address Register Indirect with Post-Increment
    tmp = (mode_register & 0x7);
    if ((tmp == 7) && (register_value[SR_INDEX] & S_FLAG))
      tmp = SSP_INDEX;
    else
      tmp += A0_INDEX;
    address = register_value[tmp];
    switch (size) {
    case BYTE:
      // Stack pointers are always incremented by a multiple of two
      if ((tmp == SSP_INDEX) || (tmp == USP_INDEX))
        register_value[tmp] += 2;
      else
        register_value[tmp] += 1;
      break;

    case WORD:
      register_value[tmp] += 2;
      break;

    case LONG:
      register_value[tmp] += 4;
      break;
    }
    if (trace) {
      description += "(";
      description += ourRegisterData[tmp].name;
      description += ")+";
    }
    in_register = 0;
    return (EXECUTE_OK);
    break;

  case 4: // Address Register Indirect with Pre-Decrement
    tmp = (mode_register & 0x7);
    if ((tmp == 7) && (register_value[SR_INDEX] & S_FLAG))
      tmp = SSP_INDEX;
    else
      tmp += A0_INDEX;
    switch (size) {
    case BYTE:
      // Stack pointers are always decremented by a multiple of two
      if ((tmp == SSP_INDEX) || (tmp == USP_INDEX))
        register_value[tmp] -= 2;
      else
        register_value[tmp] -= 1;
      break;

    case WORD:
      register_value[tmp] -= 2;
      break;

    case LONG:
      register_value[tmp] -= 4;
      break;
    }
    address = register_value[tmp];
    if (trace) {
      description += "-(";
      description += ourRegisterData[tmp].name;
      description += ")";
    }
    in_register = 0;
    return (EXECUTE_OK);
    break;

  case 5: // Address Register Indirect with Displacement
    tmp = (mode_register & 0x7);
    if ((tmp == 7) && (register_value[SR_INDEX] & S_FLAG))
      tmp = SSP_INDEX;
    else
      tmp += A0_INDEX;
    address = register_value[tmp];

    if ((status = Peek(register_value[PC_INDEX], extend_word, WORD)) !=
        EXECUTE_OK) {
      register_value[PC_INDEX] += 2;
      return (status);
    } else {
      register_value[PC_INDEX] += 2;
    }

    address += SignExtend(extend_word, WORD);
    if (trace) {
      description += "($";
      description += IntToString(extend_word, 4);
      description += ",";
      description += ourRegisterData[tmp].name;
      description += ")";
    }
    in_register = 0;
    return (EXECUTE_OK);
    break;

  case 6: // Address Register Indirect with Index and n-bit Displacement
    if ((status = Peek(register_value[PC_INDEX], extend_word, WORD)) !=
        EXECUTE_OK) {
      register_value[PC_INDEX] += 2;
      return (status);
    } else {
      register_value[PC_INDEX] += 2;
    }

    // Only if NOT (full extension word AND ignore base register)
    if ((extend_word & 0x180) != 0x180) {
      tmp = (mode_register & 0x7);
      if ((tmp == 7) && (register_value[SR_INDEX] & S_FLAG))
        tmp = SSP_INDEX;
      else
        tmp += A0_INDEX;
      address = register_value[tmp];
    } else {
      address = 0L;
    }

    if (trace) {
      description += "($";
    }

    // either 8 bits or 16/32bits displacement
    if (extend_word & 0x100) {
      // Full  Extension Word Format
      // Process BS (Base Register Suppress) see above
      // IS (Index Suppress) ignored.
      if (extend_word & 0x40) {
        // index suppress
        //**CPU_DEBUG_PRINT0(this,"[CPU32] ERROR: Index suppress not
        //implemented.");
        // NOT IMPLEMENTED
        return (EXECUTE_ILLEGAL_INSTRUCTION);
      }

      // for CPU32, accept only no memory indirect operations (IIS=0)
      if (extend_word & 0x7) {
        //**CPU_DEBUG_PRINT0(this,"[CPU32] ERROR: bad I/IS");
        // NOT AVAILABLE TO CPU32
        return (EXECUTE_ILLEGAL_INSTRUCTION);
      }
      if ((extend_word & 0x30) == 0x20) {
        // word displacement
        if ((status = Peek(register_value[PC_INDEX], displacement, WORD)) !=
            EXECUTE_OK) {
          register_value[PC_INDEX] += 2;
          return (status);
        }
        register_value[PC_INDEX] += 2;
        displacement = SignExtend(displacement, WORD);
        if (trace) {
          description += IntToString(displacement, 4);
        }
      } else if ((extend_word & 0x30) == 0x30) {
        // long displacement
        if ((status = Peek(register_value[PC_INDEX], displacement, LONG)) !=
            EXECUTE_OK) {
          register_value[PC_INDEX] += 4;
          return (status);
        }
        register_value[PC_INDEX] += 4;
        if (trace) {
          description += IntToString(displacement, 8);
        }
      } else if ((extend_word & 0x30) == 0x10) {
        // suppress displacement
        displacement = 0;
        if (trace) {
          description += "0";
        }
      } else {
        //**CPU_DEBUG_PRINT0(this,"[CPU32] ERROR: bad BD Size");
        // NOT ALLOWED (bad BD size)
        return (EXECUTE_ILLEGAL_INSTRUCTION);
      }

    } else {
      // Brief Extension Word Format
      displacement = SignExtend(extend_word & 0xff, BYTE);
      if (trace) {
        description += IntToString(displacement, 2);
      }
    }

    address += displacement;
    if (trace) {
      if ((extend_word & 0x180) !=
          0x180) // 0x180 is full extension format + base reg. suppress
      {
        description += ",";
        description += ourRegisterData[tmp].name;
      }
      description += ",";
    }

    if (extend_word & 0x8000) // Get register number
    {
      tmp = ((extend_word >> 12) & 0x7);
      if ((tmp == 7) && (register_value[SR_INDEX] & S_FLAG))
        tmp = SSP_INDEX;
      else
        tmp += A0_INDEX;
    } else {
      tmp = ((extend_word >> 12) & 0x7) + D0_INDEX;
    }

    // if extend_word==0x600, scale is 8, ie 1*4*2.
    if (extend_word & 0x400) {
      scale *= 4;
    }
    if (extend_word & 0x200) {
      scale *= 2;
    }

    if (extend_word & 0x800)
      address += register_value[tmp] * scale;
    else
      address += SignExtend(register_value[tmp], WORD) * scale;

    if (trace) {
      description += ourRegisterData[tmp].name;
      if (extend_word & 0x800)
        description += ".L";
      else
        description += ".W";

      if ((extend_word & 0x600) == 0x600)
        description += "*8";
      else if (extend_word & 0x400)
        description += "*4";
      else if (extend_word & 0x200)
        description += "*2";
      description += ")";
    }
    in_register = 0;
    return (EXECUTE_OK);
    break;

  case 7:
    switch (mode_register & 7) {
    case 0: // Absolute Short Address
      if ((status = Peek(register_value[PC_INDEX], extend_word, WORD)) !=
          EXECUTE_OK) {
        register_value[PC_INDEX] += 2;
        return (status);
      } else {
        register_value[PC_INDEX] += 2;
      }
      address = SignExtend(extend_word, WORD);
      if (trace) {
        description += IntToString(extend_word, 4);
        description += ".W";
      }
      in_register = 0;
      return (EXECUTE_OK);
      break;

    case 1: // Absolute Long Address
      if ((status = Peek(register_value[PC_INDEX], extend_word, LONG)) !=
          EXECUTE_OK) {
        register_value[PC_INDEX] += 4;
        return (status);
      } else {
        register_value[PC_INDEX] += 4;
      }
      address = extend_word;
      if (trace) {
        description += IntToString(extend_word, 8);
        description += ".L";
      }
      in_register = 0;
      return (EXECUTE_OK);
      break;

    case 2: // Program Counter with Displacement
      address = register_value[PC_INDEX];
      if ((status = Peek(register_value[PC_INDEX], extend_word, WORD)) !=
          EXECUTE_OK) {
        register_value[PC_INDEX] += 2;
        return (status);
      } else {
        register_value[PC_INDEX] += 2;
      }
      address += SignExtend(extend_word, WORD);
      if (trace) {
        description += "($";
        description += IntToString(extend_word, 4);
        description += ",PC)";
      }
      in_register = 0;
      return (EXECUTE_OK);
      break;

    case 3: // Program Counter with Index and n-bits Displacement
      address = register_value[PC_INDEX];
      if ((status = Peek(register_value[PC_INDEX], extend_word, WORD)) !=
          EXECUTE_OK) {
        register_value[PC_INDEX] += 2;
        return (status);
      } else {
        register_value[PC_INDEX] += 2;
      }

      // Only if NOT (full extension word AND ignore base register)
      // 'PC suppress'
      if ((extend_word & 0x180) == 0x180) {
        address = 0L;
      }

      // either 8 bits or 16/32bits displacement
      if (extend_word & 0x100) {
        // Full  Extension Word Format
        // Process BS (Base Register Suppress) see above
        // IS (Index Suppress) ignored
        if (extend_word & 0x40) {
          // index suppress
          //**CPU_DEBUG_PRINT0(this,"[CPU32] ERROR: Index suppress not
          //implemented.");
          // NOT IMPLEMENTED
          return (EXECUTE_ILLEGAL_INSTRUCTION);
        }

        // for CPU32, accept only no memory indirect operations (IIS=0)
        if (extend_word & 0x7) {
          //**CPU_DEBUG_PRINT0(this,"[CPU32] ERROR: bad I/IS");
          // NOT AVAILABLE TO CPU32
          return (EXECUTE_ILLEGAL_INSTRUCTION);
        }
        if ((extend_word & 0x30) == 0x20) {
          // word displacement
          if ((status = Peek(register_value[PC_INDEX], displacement, WORD)) !=
              EXECUTE_OK) {
            register_value[PC_INDEX] += 2;
            return (status);
          }
          register_value[PC_INDEX] += 2;
          displacement = SignExtend(displacement, WORD);
        } else if ((extend_word & 0x30) == 0x30) {
          // long displacement
          if ((status = Peek(register_value[PC_INDEX], displacement, LONG)) !=
              EXECUTE_OK) {
            register_value[PC_INDEX] += 4;
            return (status);
          }
          register_value[PC_INDEX] += 4;
        } else if ((extend_word & 0x30) == 0x10) {
          // suppress displacement
          displacement = 0;
        } else {
          //**CPU_DEBUG_PRINT0(this,"[CPU32] ERROR: bad BD Size");
          // NOT ALLOWED (bad BD size)
          return (EXECUTE_ILLEGAL_INSTRUCTION);
        }
      } else {
        // Brief Extension Word Format
        displacement = SignExtend(extend_word & 0xff, BYTE);
      }

      address += displacement;
      if (trace) {
        description += "($";
        description += IntToString(displacement, 4);
        description += ",";
        if ((extend_word & 0x180) != 0x180) {
          // not 'PC suppress'
          description += "PC,";
        }
      }

      if (extend_word & 0x8000) // Get register number
      {
        tmp = ((extend_word >> 12) & 0x7);
        if ((tmp == 7) && (register_value[SR_INDEX] & S_FLAG))
          tmp = SSP_INDEX;
        else
          tmp += A0_INDEX;
      } else {
        tmp = ((extend_word >> 12) & 0x7) + D0_INDEX;
      }

      // if extend_word==0x600, scale is 8, ie 1*4*2.
      if (extend_word & 0x400) {
        scale *= 4;
      }
      if (extend_word & 0x200) {
        scale *= 2;
      }

      if (extend_word & 0x0800)
        address += register_value[tmp] * scale;
      else
        address += SignExtend(register_value[tmp], WORD) * scale;

      if (trace) {
        description += ourRegisterData[tmp].name;
        if (extend_word & 0x0800)
          description += ".L";
        else
          description += ".W";
        if ((extend_word & 0x600) == 0x600)
          description += "*8";
        else if (extend_word & 0x400)
          description += "*4";
        else if (extend_word & 0x200)
          description += "*2";
        description += ")";
      }
      in_register = 0;
      return (EXECUTE_OK);
      break;

    case 4: // Immediate Data
      address = register_value[PC_INDEX];

      if (size == BYTE)
        ++address;

      if ((size == BYTE) || (size == WORD))
        register_value[PC_INDEX] += 2;
      else
        register_value[PC_INDEX] += 4;

      if (trace) {
        // Fetch the immediate data
        if ((status = Peek(address, extend_word, size)) != EXECUTE_OK)
          return (status);

        description += "#$";
        if (size == BYTE)
          description += IntToString(extend_word, 2);
        else if (size == WORD)
          description += IntToString(extend_word, 4);
        else if (size == LONG)
          description += IntToString(extend_word, 8);
      }
      in_register = 0;
      return (EXECUTE_OK);
      break;
    }
  }
  return (EXECUTE_ILLEGAL_INSTRUCTION);
}

unsigned int cpu32::SignExtend(unsigned int value, int size) {
  switch (size) {
  case BYTE:
    if (value & 0x80)
      return ((value & 0xff) | 0xffffff00);
    else
      return (value & 0xff);
    break;

  case WORD:
    if (value & 0x8000)
      return ((value & 0xffff) | 0xffff0000);
    else
      return (value & 0xffff);
    break;

  default: // LONG
    return (value & 0xffffffff);
    break;
  }
}

int cpu32::Peek(unsigned long address, unsigned int &value, int size) {
  unsigned long data;

  // for CPU32, should use SFC to choose address space...
  if (myAddressSpaces[0]->Peek(address, data, size)) {
    value = (unsigned int)data;
    return (EXECUTE_OK);
  }
  return (EXECUTE_BUS_ERROR);
}

int cpu32::Poke(unsigned long address, unsigned int value, int size) {
  // for CPU32, should use DFC to choose address space...
  if (myAddressSpaces[0]->Poke(address, value, size)) {
    return (EXECUTE_OK);
  }
  return (EXECUTE_BUS_ERROR);
}

void cpu32::ClearConditionCodes(int mask) {
  if (mask & C_FLAG)
    register_value[SR_INDEX] &= ~C_FLAG;
  else if (mask & V_FLAG)
    register_value[SR_INDEX] &= ~V_FLAG;
  else if (mask & Z_FLAG)
    register_value[SR_INDEX] &= ~Z_FLAG;
  else if (mask & N_FLAG)
    register_value[SR_INDEX] &= ~N_FLAG;
  else if (mask & X_FLAG)
    register_value[SR_INDEX] &= ~X_FLAG;
  else if (mask & T_FLAG)
    register_value[SR_INDEX] &= ~T_FLAG;
  else if (mask & S_FLAG)
    register_value[SR_INDEX] &= ~S_FLAG;
}

void cpu32::SetConditionCodes(unsigned int src, unsigned int dest,
                              unsigned int result, int size, int operation,
                              int mask) {
  int S, D, R;

  switch (size) {
  case BYTE:
    S = (src >> 7) & 1;
    D = (dest >> 7) & 1;
    R = (result >> 7) & 1;
    result = result & 0xff;
    break;
  case WORD:
    S = (src >> 15) & 1;
    D = (dest >> 15) & 1;
    R = (result >> 15) & 1;
    result = result & 0xffff;
    break;
  case LONG:
    S = (src >> 31) & 1;
    D = (dest >> 31) & 1;
    R = (result >> 31) & 1;
    result = result & 0xffffffff;
    break;
  default:
    S = D = R = 0;
  }

  if (mask & C_FLAG) {
    if (operation == ADDITION) {
      if ((S && D) || (!R && D) || (S && !R))
        register_value[SR_INDEX] |= C_FLAG;
      else
        register_value[SR_INDEX] &= ~C_FLAG;
    } else if (operation == SUBTRACTION) {
      if ((S && !D) || (R && !D) || (S && R))
        register_value[SR_INDEX] |= C_FLAG;
      else
        register_value[SR_INDEX] &= ~C_FLAG;
    } else {
      register_value[SR_INDEX] &= ~C_FLAG;
    }
  }

  if (mask & V_FLAG) {
    if (operation == ADDITION) {
      if ((S && D && !R) || (!S && !D && R))
        register_value[SR_INDEX] |= V_FLAG;
      else
        register_value[SR_INDEX] &= ~V_FLAG;
    } else if (operation == SUBTRACTION) {
      if ((!S && D && !R) || (S && !D && R))
        register_value[SR_INDEX] |= V_FLAG;
      else
        register_value[SR_INDEX] &= ~V_FLAG;
    } else {
      register_value[SR_INDEX] &= ~V_FLAG;
    }
  }

  if (mask & Z_FLAG) {
    if (!result)
      register_value[SR_INDEX] |= Z_FLAG;
    else
      register_value[SR_INDEX] &= ~Z_FLAG;
  }

  if (mask & N_FLAG) {
    if (R)
      register_value[SR_INDEX] |= N_FLAG;
    else
      register_value[SR_INDEX] &= ~N_FLAG;
  }

  if (mask & X_FLAG) {
    if (operation == ADDITION) {
      if ((S && D) || (!R && D) || (S && !R))
        register_value[SR_INDEX] |= X_FLAG;
      else
        register_value[SR_INDEX] &= ~X_FLAG;
    } else if (operation == SUBTRACTION) {
      if ((S && !D) || (R && !D) || (S && R))
        register_value[SR_INDEX] |= X_FLAG;
      else
        register_value[SR_INDEX] &= ~X_FLAG;
    } else {
      register_value[SR_INDEX] &= ~X_FLAG;
    }
  }
}

void cpu32::SetRegister(int register_number, unsigned int value, int size) {
  switch (size) {
  case BYTE:
    register_value[register_number] &= 0xffffff00;
    register_value[register_number] |= (value & 0xff);
    break;
  case WORD:
    register_value[register_number] &= 0xffff0000;
    register_value[register_number] |= (value & 0xffff);
    break;
  case LONG:
    register_value[register_number] &= 0x00000000;
    register_value[register_number] |= (value & 0xffffffff);
    break;
  }
}

int cpu32::CheckConditionCodes(int code, std::string &mnemonic, int trace) {
  int branch = 0;
  unsigned long sr;

  sr = register_value[SR_INDEX];
  switch (code) {
  case 4:
    branch = !(sr & C_FLAG);
    if (trace)
      mnemonic += "CC";
    break;
  case 5:
    branch = (sr & C_FLAG);
    if (trace)
      mnemonic += "CS";
    break;
  case 7:
    branch = (sr & Z_FLAG);
    if (trace)
      mnemonic += "EQ";
    break;
  case 1:
    branch = 0;
    if (trace)
      mnemonic += "F";
    break;
  case 12:
    branch =
        ((sr & N_FLAG) && (sr & V_FLAG)) || (!(sr & N_FLAG) && !(sr & V_FLAG));
    if (trace)
      mnemonic += "GE";
    break;
  case 14:
    branch = ((sr & N_FLAG) && (sr & V_FLAG) && !(sr & Z_FLAG)) ||
             (!(sr & N_FLAG) && !(sr & V_FLAG) && !(sr & Z_FLAG));
    if (trace)
      mnemonic += "GT";
    break;
  case 2:
    branch = (!(sr & C_FLAG) && !(sr & Z_FLAG));
    if (trace)
      mnemonic += "HI";
    break;
  case 15:
    branch = (sr & Z_FLAG) || ((sr & N_FLAG) && !(sr & V_FLAG)) ||
             (!(sr & N_FLAG) && (sr & V_FLAG));
    if (trace)
      mnemonic += "LE";
    break;
  case 3:
    branch = (sr & C_FLAG) || (sr & Z_FLAG);
    if (trace)
      mnemonic += "LS";
    break;
  case 13:
    branch =
        ((sr & N_FLAG) && !(sr & V_FLAG)) || (!(sr & N_FLAG) && (sr & V_FLAG));
    if (trace)
      mnemonic += "LT";
    break;
  case 11:
    branch = (sr & N_FLAG);
    if (trace)
      mnemonic += "MI";
    break;
  case 6:
    branch = !(sr & Z_FLAG);
    if (trace)
      mnemonic += "NE";
    break;
  case 10:
    branch = !(sr & N_FLAG);
    if (trace)
      mnemonic += "PL";
    break;
  case 0:
    branch = 1;
    if (trace)
      mnemonic += "T";
    break;
  case 8:
    branch = !(sr & V_FLAG);
    if (trace)
      mnemonic += "VC";
    break;
  case 9:
    branch = (sr & V_FLAG);
    if (trace)
      mnemonic += "VS";
    break;
  }
  return (branch);
}

int cpu32::ExecuteABCD(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long src_address, dest_address;
  unsigned int result, src, dest, zw_resultat, zw_uebertrag; // T_M change
  std::string mnemonic;

  size = (opcode & 0x00c0) >> 6;

  if (trace) {
    mnemonic += "{Mnemonic {ABCD";
    if (size == BYTE)
      mnemonic += ".B ";
    else
      return (ExecuteInvalid(opcode, trace_record, trace));
  }

  // Get the addresses
  if (opcode & 8) {
    if ((status = ComputeEffectiveAddress(src_address, in_register_flag,
                                          mnemonic, 0x20 | (opcode & 7), size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }

    if (trace)
      mnemonic += ",";

    if ((status = ComputeEffectiveAddress(
             dest_address, in_register_flag, mnemonic,
             0x20 | ((opcode & 0x0e00) >> 9), size, trace)) != EXECUTE_OK) {
      return (status);
    }

    if ((status = Peek(src_address, src, size)) != EXECUTE_OK)
      return (status);
    if ((status = Peek(dest_address, dest, size)) != EXECUTE_OK)
      return (status);
  } else {
    src_address = D0_INDEX + (opcode & 0x0007);
    src = register_value[src_address];
    dest_address = D0_INDEX + ((opcode & 0x0e00) >> 9);
    dest = register_value[dest_address];
    if (trace) {
      mnemonic += ourRegisterData[src_address].name;
      mnemonic += ",";
      mnemonic += ourRegisterData[dest_address].name;
    }
  }

  if (register_value[SR_INDEX] & X_FLAG)
    zw_uebertrag = 1; // vorheriger Uebertrag beachten!!

  /////// now comes the bcd-addition !! /////////

  // 10er stelle abschneiden und 1er addieren
  result = (src & 0x0f) + (dest & 0x0f);
  if (result > 9) {
    result -= 10;
    zw_uebertrag = 1;
  } else
    zw_uebertrag = 0;

  zw_resultat = ((src & 0xf0) >> 4) + ((dest & 0xf0) >> 4) + zw_uebertrag;
  if (zw_resultat > 9) {
    zw_resultat -= 10;

    // trick, damit 'SetConditionCodes' das carry, und x-bit setzt !!
    src = 0xff;
    dest = 0xff;
  }

  // endgueltiges resultat !!
  result += zw_resultat << 4;

  // only BYTE data !!
  result = result & 0xff;

  SetConditionCodes(src, dest, result, size, ADDITION,
                    C_FLAG | V_FLAG | N_FLAG | X_FLAG);
  if (result)
    register_value[SR_INDEX] &= ~Z_FLAG;

  if (opcode & 8) {
    if ((status = Poke(dest_address, result, size)) != EXECUTE_OK)
      return (status);
  } else {
    SetRegister(dest_address, result, size);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return EXECUTE_OK;
}

int cpu32::ExecuteADD(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int result, ea_data;
  std::string mnemonic, ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, size)) != EXECUTE_OK)
    return (status);

  // Get the register number
  register_number = D0_INDEX + ((opcode & 0x0e00) >> 9);

  if (trace) {
    mnemonic += "{Mnemonic {ADD";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  if (opcode & 0x0100) // <Dn> + <ea> -> <ea>
  {
    if (trace) {
      mnemonic += ourRegisterData[register_number].name;
      mnemonic += ",";
      mnemonic += ea_description;
    }
    result = register_value[register_number] + ea_data;
    SetConditionCodes(register_value[register_number], ea_data, result, size,
                      ADDITION, C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
    if ((status = Poke(ea_address, result, size)) != EXECUTE_OK)
      return (status);
  } else // <ea> + <Dn> -> <Dn>
  {
    if (trace) {
      mnemonic += ea_description;
      mnemonic += ",";
      mnemonic += ourRegisterData[register_number].name;
    }
    result = ea_data + register_value[register_number];
    SetConditionCodes(ea_data, register_value[register_number], result, size,
                      ADDITION, C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
    SetRegister(register_number, result, size);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteADDA(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int result, ea_data;
  std::string mnemonic, ea_description;

  if (opcode & 0x0100)
    size = LONG;
  else
    size = WORD;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, size)) != EXECUTE_OK)
    return (status);

  if (size == WORD)
    ea_data = SignExtend(ea_data, WORD);

  // Get the register number
  register_number = A0_INDEX + ((opcode & 0x0e00) >> 9);

  // Adjust register_number if it's A7 and we're in supervisor mode
  if ((register_number == USP_INDEX) && (register_value[SR_INDEX] & S_FLAG))
    register_number = SSP_INDEX;

  result = ea_data + register_value[register_number];
  SetRegister(register_number, result, LONG);

  if (trace) {
    mnemonic += "{Mnemonic {ADDA";
    if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";

    mnemonic += ea_description;
    mnemonic += ",";
    mnemonic += ourRegisterData[register_number].name;
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteADDI(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register;
  unsigned long dest_addr, src_addr;
  unsigned int result, src, dest;
  std::string mnemonic;

  size = (opcode & 0x00c0) >> 6;

  if (trace) {
    mnemonic += "{Mnemonic {ADDI";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register, mnemonic, 0x3c,
                                        size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, size)) != EXECUTE_OK)
    return (status);

  if (trace)
    mnemonic += ",";

  // Get the destination data pointer
  if ((status = ComputeEffectiveAddress(dest_addr, in_register, mnemonic,
                                        opcode & 0x3f, size, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register) {
    dest = register_value[dest_addr];
    result = dest + src;
    SetConditionCodes(src, dest, result, size, ADDITION,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
    SetRegister(dest_addr, result, size);
  } else {
    if ((status = Peek(dest_addr, dest, size)) != EXECUTE_OK)
      return (status);
    result = dest + src;
    SetConditionCodes(src, dest, result, size, ADDITION,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
    if ((status = Poke(dest_addr, result, size)) != EXECUTE_OK)
      return (status);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteADDQ(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, immediate_data;
  unsigned int result, ea_data;
  std::string ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Get the immediate data out of the opcode
  if ((immediate_data = (opcode & 0x0e00) >> 9) == 0)
    immediate_data = 8;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, size)) != EXECUTE_OK)
    return (status);

  if (in_register_flag) {
    result = immediate_data + ea_data;
    if ((ea_address >= A0_INDEX) && (ea_address <= SSP_INDEX)) {
      SetRegister(ea_address, result, LONG);
    } else {
      SetConditionCodes(immediate_data, ea_data, result, size, ADDITION,
                        C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
      SetRegister(ea_address, result, size);
    }
  } else {
    result = immediate_data + ea_data;
    SetConditionCodes(immediate_data, ea_data, result, size, ADDITION,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
    if ((status = Poke(ea_address, result, size)) != EXECUTE_OK)
      return (status);
  }

  if (trace) {
    trace_record += "{Mnemonic {ADDQ";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += "#$";
    trace_record += IntToString(immediate_data, 1);
    trace_record += ",";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteADDX(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long src_address, dest_address;
  unsigned int result, src, dest;
  std::string mnemonic;

  size = (opcode & 0x00c0) >> 6;

  if (trace) {
    mnemonic += "{Mnemonic {ADDX";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  // Get the addresses
  if (opcode & 8) {
    if ((status = ComputeEffectiveAddress(src_address, in_register_flag,
                                          mnemonic, 0x20 | (opcode & 7), size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }

    if (trace)
      mnemonic += ",";

    if ((status = ComputeEffectiveAddress(
             dest_address, in_register_flag, mnemonic,
             0x20 | ((opcode & 0x0e00) >> 9), size, trace)) != EXECUTE_OK) {
      return (status);
    }

    if ((status = Peek(src_address, src, size)) != EXECUTE_OK)
      return (status);
    if ((status = Peek(dest_address, dest, size)) != EXECUTE_OK)
      return (status);
  } else {
    src_address = D0_INDEX + (opcode & 0x0007);
    src = register_value[src_address];
    dest_address = D0_INDEX + ((opcode & 0x0e00) >> 9);
    dest = register_value[dest_address];
    if (trace) {
      mnemonic += ourRegisterData[src_address].name;
      mnemonic += ",";
      mnemonic += ourRegisterData[dest_address].name;
    }
  }

  if (register_value[SR_INDEX] & X_FLAG)
    result = src + dest + 1;
  else
    result = src + dest;

  if (size == BYTE)
    result = result & 0xff;
  else if (size == WORD)
    result = result & 0xffff;
  else if (size == LONG)
    result = result & 0xffffffff;

  SetConditionCodes(src, dest, result, size, ADDITION,
                    C_FLAG | V_FLAG | N_FLAG | X_FLAG);
  if (result)
    register_value[SR_INDEX] &= ~Z_FLAG;

  if (opcode & 8) {
    if ((status = Poke(dest_address, result, size)) != EXECUTE_OK)
      return (status);
  } else {
    SetRegister(dest_address, result, size);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteAND(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int result, ea_data;
  std::string mnemonic, ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, size)) != EXECUTE_OK)
    return (status);

  // Get the register number
  register_number = D0_INDEX + ((opcode & 0x0e00) >> 9);

  if (trace) {
    mnemonic += "{Mnemonic {AND";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  if (opcode & 0x0100) // <Dn> & <ea> -> <ea>
  {
    if (trace) {
      mnemonic += ourRegisterData[register_number].name;
      mnemonic += ",";
      mnemonic += ea_description;
    }
    result = register_value[register_number] & ea_data;
    SetConditionCodes(register_value[register_number], ea_data, result, size,
                      OTHER, V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
    if ((status = Poke(ea_address, result, size)) != EXECUTE_OK)
      return (status);
  } else // <ea> & <Dn> -> <Dn>
  {
    if (trace) {
      mnemonic += ea_description;
      mnemonic += ",";
      mnemonic += ourRegisterData[register_number].name;
    }
    result = ea_data & register_value[register_number];
    SetConditionCodes(ea_data, register_value[register_number], result, size,
                      OTHER, V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
    SetRegister(register_number, result, size);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteANDI(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register;
  unsigned long dest_addr, src_addr;
  unsigned int result, src, dest;
  std::string mnemonic;

  size = (opcode & 0x00c0) >> 6;

  if (trace) {
    mnemonic += "{Mnemonic {ANDI";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register, mnemonic, 0x3c,
                                        size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, size)) != EXECUTE_OK)
    return (status);

  if (trace)
    mnemonic += ",";

  // Get the destination data pointer
  if ((status = ComputeEffectiveAddress(dest_addr, in_register, mnemonic,
                                        opcode & 0x3f, size, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register) {
    dest = register_value[dest_addr];
    result = dest & src;
    SetConditionCodes(src, dest, result, size, OTHER,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
    SetRegister(dest_addr, result, size);
  } else {
    if ((status = Peek(dest_addr, dest, size)) != EXECUTE_OK)
      return (status);
    result = dest & src;
    SetConditionCodes(src, dest, result, size, OTHER,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
    if ((status = Poke(dest_addr, result, size)) != EXECUTE_OK)
      return (status);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteANDItoCCR(int, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long src_addr;
  unsigned int src;
  std::string mnemonic;

  size = BYTE;

  if (trace)
    mnemonic += "{Mnemonic {ANDI.B ";

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register_flag, mnemonic,
                                        0x3c, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, size)) != EXECUTE_OK)
    return (status);

  SetRegister(SR_INDEX, (register_value[SR_INDEX] & src), BYTE);
  if (trace) {
    mnemonic += ",CCR}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteANDItoSR(int, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long src_addr;
  unsigned int src;
  std::string mnemonic;

  // Make sure we're in supervisor mode or trap
  if (!(register_value[SR_INDEX] & S_FLAG)) {
    SetRegister(PC_INDEX, register_value[PC_INDEX] - 2, LONG);
    if ((status = ProcessException(8)) != EXECUTE_OK)
      return (status);
    if (trace)
      ;
    trace_record += "{Mnemonic {Privilege Violation Exception}} ";
    return (EXECUTE_PRIVILEGED_OK);
  }

  size = WORD;

  if (trace)
    mnemonic += "{Mnemonic {ANDI.W ";

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register_flag, mnemonic,
                                        0x3c, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, size)) != EXECUTE_OK)
    return (status);

  SetRegister(SR_INDEX, register_value[SR_INDEX] & src, WORD);

  if (trace) {
    mnemonic += ",SR}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_PRIVILEGED_OK);
}

int cpu32::ExecuteASL(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag, shift_count;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Check to see if this is a memory or register shift
  if (size == 3) {
    size = WORD; // Memory always shifts a word

    // Get the address
    if ((status = ComputeEffectiveAddress(address, in_register_flag,
                                          ea_description, opcode & 0x3f, size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }

    // Fetch the data
    if ((status = Peek(address, data, size)) != EXECUTE_OK)
      return (status);

    // Shift the data to the left by one bit.
    data = data << 1;

    // Store the shifted data
    if ((status = Poke(address, data, size)) != EXECUTE_OK)
      return (status);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | X_FLAG | C_FLAG | V_FLAG);
    if (data & 0x00010000) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }
    if (!((data & 0x000180000) == 0x000180000) &&
        !((data & 0x000180000) == 0x00000000))
      register_value[SR_INDEX] |= V_FLAG;
  } else {
    // Compute the shift count
    if (opcode & 32) {
      shift_count = register_value[D0_INDEX + ((opcode & 0x0e00) >> 9)] & 0x3f;
      if (trace)
        ea_description =
            ourRegisterData[D0_INDEX + ((opcode & 0x0e00) >> 9)].name;
    } else {
      if ((shift_count = (opcode & 0x0e00) >> 9) == 0)
        shift_count = 8;

      if (trace) {
        ea_description = "#$";
        ea_description += IntToString(shift_count, 1);
      }
    }

    if (trace) {
      ea_description += ",";
      ea_description += ourRegisterData[D0_INDEX + (opcode & 7)].name;
    }

    unsigned int carry = 0, overflow = 0;
    unsigned int carry_mask, overflow_mask;

    // Setup masks
    if (size == BYTE) {
      carry_mask = 0x80;
      overflow_mask = 0xc0;
    } else if (size == WORD) {
      carry_mask = 0x8000;
      overflow_mask = 0xc000;
    } else {
      carry_mask = 0x80000000;
      overflow_mask = 0xc0000000;
    }

    // Perform the shift on the data
    data = register_value[D0_INDEX + (opcode & 7)];
    for (int t = 0; t < shift_count; ++t) {
      carry = data & carry_mask;
      if (!((data & overflow_mask) == overflow_mask) &&
          !((data & overflow_mask) == 0))
        overflow |= 1;

      data = data << 1;
    }

    SetRegister(D0_INDEX + (opcode & 7), data, size);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | X_FLAG | C_FLAG | V_FLAG);
    if (overflow)
      register_value[SR_INDEX] |= V_FLAG;
    if (carry) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }
  }

  if (trace) {
    trace_record += "{Mnemonic {ASL";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteASR(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag, shift_count;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Check to see if this is a memory or register shift
  if (size == 3) {
    size = WORD; // Memory always shifts a word

    // Get the address
    if ((status = ComputeEffectiveAddress(address, in_register_flag,
                                          ea_description, opcode & 0x3f, size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }

    // Fetch the data
    if ((status = Peek(address, data, size)) != EXECUTE_OK)
      return (status);

    // Set the condition codes
    SetConditionCodes(0, 0, data >> 1, size, OTHER,
                      N_FLAG | Z_FLAG | X_FLAG | C_FLAG | V_FLAG);
    if (data & 0x0001) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }

    // Shift the data to the right by one bit.
    data = data >> 1;

    // Replicate the old high order bit
    if (data & 0x4000)
      data |= 0x8000;
    else
      data &= ~0x8000;

    // Store the shifted data
    if ((status = Poke(address, data, size)) != EXECUTE_OK)
      return (status);
  } else {
    // Compute the shift count
    if (opcode & 32) {
      shift_count = register_value[D0_INDEX + ((opcode & 0x0e00) >> 9)] & 0x3f;
      if (trace)
        ea_description =
            ourRegisterData[D0_INDEX + ((opcode & 0x0e00) >> 9)].name;
    } else {
      if ((shift_count = (opcode & 0x0e00) >> 9) == 0)
        shift_count = 8;

      if (trace) {
        ea_description = "#$";
        ea_description += IntToString(shift_count, 1);
      }
    }

    if (trace) {
      ea_description += ",";
      ea_description += ourRegisterData[D0_INDEX + (opcode & 7)].name;
    }

    unsigned int carry = 0, replicate_mask, msb;

    // Setup masks
    if (size == BYTE) {
      replicate_mask = 0x40;
      msb = 0x80;
    } else if (size == WORD) {
      replicate_mask = 0x4000;
      msb = 0x8000;
    } else {
      replicate_mask = 0x40000000;
      msb = 0x80000000;
    }

    // Perform the shift on the data
    data = register_value[D0_INDEX + (opcode & 7)];
    for (int t = 0; t < shift_count; ++t) {
      carry = data & 0x00000001;
      data = data >> 1;

      // Replicate the old high order bit
      if (data & replicate_mask)
        data |= msb;
      else
        data &= ~msb;
    }

    SetRegister(D0_INDEX + (opcode & 7), data, size);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | X_FLAG | C_FLAG | V_FLAG);
    if (carry) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }
  }

  if (trace) {
    trace_record += "{Mnemonic {ASR";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteBit(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int ea_data, bit_number;
  std::string mnemonic, ea_description;

  // Get the bit number we're supposed to be checking
  if (opcode & 256) {
    register_number = D0_INDEX + ((opcode & 0x0e00) >> 9);
    bit_number = register_value[register_number];
    if (trace)
      ea_description = ourRegisterData[register_number].name;
  } else {
    unsigned long address;

    // Get the immediate data pointer
    if ((status = ComputeEffectiveAddress(address, in_register_flag,
                                          ea_description, 0x3c, WORD, trace)) !=
        EXECUTE_OK) {
      return (status);
    }

    // Fetch the immediate data
    if ((status = Peek(address, bit_number, WORD)) != EXECUTE_OK)
      return (status);
  }

  if (trace)
    ea_description += ",";

  // Determine the size of the operation (BYTE or LONG)
  if ((opcode & 0x38) == 0)
    size = LONG;
  else
    size = BYTE;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag) {
    ea_data = register_value[ea_address];
    bit_number = (1 << (bit_number & 0x1F));
  } else {
    if ((status = Peek(ea_address, ea_data, BYTE)) != EXECUTE_OK)
      return (status);
    bit_number = (1 << (bit_number & 0x07));
  }

  // Set the Zero Flag
  if (ea_data & bit_number)
    register_value[SR_INDEX] &= ~Z_FLAG;
  else
    register_value[SR_INDEX] |= Z_FLAG;

  switch ((opcode & 0x00c0) >> 6) {
  case 0: // BTST
    if (trace)
      mnemonic += "{Mnemonic {BTST";
    break;

  case 1: // BCHG
    if (trace)
      mnemonic += "{Mnemonic {BCHG";
    if (ea_data & bit_number)
      ea_data &= ~bit_number;
    else
      ea_data |= bit_number;
    break;

  case 2: // BCLR
    if (trace)
      mnemonic += "{Mnemonic {BCLR";
    ea_data &= ~bit_number;
    break;

  case 3: // BSET
    if (trace)
      mnemonic += "{Mnemonic {BSET";
    ea_data |= bit_number;
    break;
  }

  // If it's not BTST then write the result back
  if (((opcode & 0x00c0) >> 6) != 0) {
    if (in_register_flag)
      SetRegister(ea_address, ea_data, LONG);
    else if ((status = Poke(ea_address, ea_data, BYTE)) != EXECUTE_OK)
      return (status);
  }

  if (trace) {
    if (size == BYTE)
      mnemonic += ".B ";
    else
      mnemonic += ".L ";
    trace_record += mnemonic;
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteBRA(int opcode, std::string &trace_record, int trace) {
  unsigned int displacement;
  int status;
  std::string mnemonic;

  if (trace)
    mnemonic = "{Mnemonic {BRA";

  // Compute the displacement
  if ((displacement = opcode & 0xff) == 0xFF) {
    // Fetch the 32-bit displacement data
    status = Peek(register_value[PC_INDEX], displacement, LONG);
    if (status != EXECUTE_OK)
      return (status);

    displacement = SignExtend(displacement, LONG);
    if (trace) {
      mnemonic += ".L $";
      mnemonic += IntToString(displacement, 8);
      mnemonic += "}} ";
      trace_record += mnemonic;
    }
  } else if ((displacement = opcode & 0xff) == 0) {
    // Fetch the 16-bit displacement data
    status = Peek(register_value[PC_INDEX], displacement, WORD);
    if (status != EXECUTE_OK)
      return (status);

    displacement = SignExtend(displacement, WORD);
    if (trace) {
      mnemonic += ".W $";
      mnemonic += IntToString(displacement, 4);
      mnemonic += "}} ";
      trace_record += mnemonic;
    }
  } else {
    displacement = SignExtend(displacement, BYTE);
    if (trace) {
      mnemonic += ".B $";
      mnemonic += IntToString(displacement, 2);
      mnemonic += "}} ";
      trace_record += mnemonic;
    }
  }

  SetRegister(PC_INDEX, register_value[PC_INDEX] + displacement, LONG);
  return (EXECUTE_OK);
}

int cpu32::ExecuteBSR(int opcode, std::string &trace_record, int trace) {
  unsigned int displacement;
  int status;
  unsigned long addr;
  std::string mnemonic;

  if (trace)
    mnemonic = "{Mnemonic {BSR";

  // Compute the displacement
  if ((displacement = opcode & 0xff) == 0xFF) {
    // Fetch the 32-bit displacement data
    status = Peek(register_value[PC_INDEX], displacement, LONG);
    if (status != EXECUTE_OK)
      return (status);

    displacement = SignExtend(displacement, LONG);
    if (trace) {
      mnemonic += ".L $";
      mnemonic += IntToString(displacement, 8);
      mnemonic += "}} ";
      trace_record += mnemonic;
    }
  } else if ((displacement = opcode & 0xff) == 0) {
    // Fetch the 16-bit displacement data
    status = Peek(register_value[PC_INDEX], displacement, WORD);
    if (status != EXECUTE_OK)
      return (status);

    displacement = SignExtend(displacement, WORD);
    if (trace) {
      mnemonic += ".W $";
      mnemonic += IntToString(displacement, 4);
      mnemonic += "}} ";
    }
  } else {
    displacement = SignExtend(displacement, BYTE);
    if (trace) {
      mnemonic += ".B $";
      mnemonic += IntToString(displacement, 2);
      mnemonic += "}}  ";
    }
  }

  // Push the PC onto the stack
  if (register_value[SR_INDEX] & S_FLAG) {
    SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 4, LONG);
    addr = register_value[SSP_INDEX];
  } else {
    SetRegister(USP_INDEX, register_value[USP_INDEX] - 4, LONG);
    addr = register_value[USP_INDEX];
  }

  if ((opcode & 0xff) == 0xff) {
    if ((status = Poke(addr, register_value[PC_INDEX] + 4, LONG)) != EXECUTE_OK)
      return (status);
  } else if ((opcode & 0xff) == 0) {
    if ((status = Poke(addr, register_value[PC_INDEX] + 2, LONG)) != EXECUTE_OK)
      return (status);
  } else {
    if ((status = Poke(addr, register_value[PC_INDEX], LONG)) != EXECUTE_OK)
      return (status);
  }

  SetRegister(PC_INDEX, register_value[PC_INDEX] + displacement, LONG);

  if (trace)
    trace_record += mnemonic;

  return (EXECUTE_OK);
}

int cpu32::ExecuteBcc(int opcode, std::string &trace_record, int trace) {
  unsigned int displacement, branch;
  int status;
  std::string mnemonic;

  // Compute the displacement
  if ((opcode & 0xff) == 0xFF) {
    // Fetch the 32-bit displacement data
    status = Peek(register_value[PC_INDEX], displacement, LONG);
    if (status != EXECUTE_OK)
      return (status);

  } else if ((opcode & 0xff) == 0) {
    // Fetch the 16-bit displacement data
    status = Peek(register_value[PC_INDEX], displacement, WORD);
    if (status != EXECUTE_OK)
      return (status);
  } else {
    displacement = opcode & 0xff;
  }

  if (trace)
    mnemonic = "{Mnemonic {B";

  // See if the branch should occur
  branch = CheckConditionCodes((opcode & 0x0f00) >> 8, mnemonic, trace);

  if ((opcode & 0xff) == 0xff) {
    if (trace) {
      mnemonic += ".L $";
      mnemonic += IntToString(displacement, 8);
    }
    // displacement=SignExtend(displacement,LONG);
  } else if ((opcode & 0xff) == 0) {
    if (trace) {
      mnemonic += ".W $";
      mnemonic += IntToString(displacement, 4);
    }
    displacement = SignExtend(displacement, WORD);
  } else {
    if (trace) {
      mnemonic += ".B $";
      mnemonic += IntToString(displacement, 2);
    }
    displacement = SignExtend(displacement, BYTE);
  }

  if (branch)
    SetRegister(PC_INDEX, register_value[PC_INDEX] + displacement, LONG);
  else if ((opcode & 0xff) == 0xff)
    SetRegister(PC_INDEX, register_value[PC_INDEX] + 4, LONG);
  else if ((opcode & 0xff) == 0)
    SetRegister(PC_INDEX, register_value[PC_INDEX] + 2, LONG);

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteCHK(int opcode, std::string &trace_record, int trace) {
  return (ExecuteInvalid(opcode, trace_record, trace));
}

int cpu32::ExecuteCLR(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register;
  unsigned long address;
  std::string mnemonic;

  size = (opcode & 0x00c0) >> 6;

  if (trace) {
    mnemonic += "{Mnemonic {CLR";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  // Get the destination data pointer
  if ((status = ComputeEffectiveAddress(address, in_register, mnemonic,
                                        opcode & 0x3f, size, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register) {
    SetConditionCodes(0, 0, 0, size, OTHER, C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
    SetRegister(address, 0, size);
  } else {
    SetConditionCodes(0, 0, 0, size, OTHER, C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
    if ((status = Poke(address, 0, size)) != EXECUTE_OK)
      return (status);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteCMP(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int result, ea_data;
  std::string mnemonic, ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, size)) != EXECUTE_OK)
    return (status);

  // Get the register number
  register_number = D0_INDEX + ((opcode & 0x0e00) >> 9);

  result = register_value[register_number] - ea_data;
  SetConditionCodes(ea_data, register_value[register_number], result, size,
                    SUBTRACTION, C_FLAG | V_FLAG | Z_FLAG | N_FLAG);

  if (trace) {
    mnemonic += "{Mnemonic {CMP";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
    mnemonic += ea_description;
    mnemonic += ",";
    mnemonic += ourRegisterData[register_number].name;
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteCMPA(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int result, ea_data;
  std::string mnemonic, ea_description;

  if (opcode & 0x0100)
    size = LONG;
  else
    size = WORD;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, size)) != EXECUTE_OK)
    return (status);

  ea_data = SignExtend(ea_data, size);

  // Get the register number
  register_number = A0_INDEX + ((opcode & 0x0e00) >> 9);

  // Adjust register_number if it's A7 and we're in supervisor mode
  if ((register_number == USP_INDEX) && (register_value[SR_INDEX] & S_FLAG))
    register_number = SSP_INDEX;

  result = register_value[register_number] - ea_data;
  SetConditionCodes(ea_data, register_value[register_number], result, size,
                    SUBTRACTION, C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
  if (trace) {
    mnemonic += "{Mnemonic {CMPA";
    if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
    mnemonic += ea_description;
    mnemonic += ",";
    mnemonic += ourRegisterData[register_number].name;
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteCMPI(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register;
  unsigned long dest_addr, src_addr;
  unsigned int result, src, dest;
  std::string mnemonic;

  size = (opcode & 0x00c0) >> 6;

  if (trace) {
    mnemonic += "{Mnemonic {CMPI";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register, mnemonic, 0x3c,
                                        size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, size)) != EXECUTE_OK)
    return (status);

  if (trace)
    mnemonic += ",";

  // Get the destination data pointer
  if ((status = ComputeEffectiveAddress(dest_addr, in_register, mnemonic,
                                        opcode & 0x3f, size, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register) {
    dest = register_value[dest_addr];
    result = dest - src;
    SetConditionCodes(src, dest, result, size, SUBTRACTION,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
  } else {
    if ((status = Peek(dest_addr, dest, size)) != EXECUTE_OK)
      return (status);
    result = dest - src;
    SetConditionCodes(src, dest, result, size, SUBTRACTION,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteCMPM(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long src_address, dest_address;
  unsigned int result, src, dest;
  std::string mnemonic;

  size = (opcode & 0x00c0) >> 6;

  if (trace) {
    mnemonic += "{Mnemonic {CMPM";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  // Get the addresses
  if ((status = ComputeEffectiveAddress(src_address, in_register_flag, mnemonic,
                                        0x18 | (opcode & 7), size, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (trace)
    mnemonic += ",";

  if ((status = ComputeEffectiveAddress(
           dest_address, in_register_flag, mnemonic,
           0x18 | ((opcode & 0x0e00) >> 9), size, trace)) != EXECUTE_OK) {
    return (status);
  }

  if ((status = Peek(src_address, src, size)) != EXECUTE_OK)
    return (status);
  if ((status = Peek(dest_address, dest, size)) != EXECUTE_OK)
    return (status);

  result = dest - src;
  SetConditionCodes(src, dest, result, size, SUBTRACTION,
                    C_FLAG | V_FLAG | N_FLAG | Z_FLAG);

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteDBcc(int opcode, std::string &trace_record, int trace) {
  unsigned int displacement, register_number, condition_code;
  int status;
  std::string mnemonic;

  if (trace)
    mnemonic = "{Mnemonic {DB";

  // Fetch the 16-bit displacement data
  if ((status = Peek(register_value[PC_INDEX], displacement, WORD)) !=
      EXECUTE_OK)
    return (status);

  displacement = SignExtend(displacement, WORD);

  // Check the condition code
  condition_code = CheckConditionCodes((opcode & 0x0f00) >> 8, mnemonic, trace);

  // Get the register number that we are counting with
  register_number = D0_INDEX + (opcode & 7);

  if (trace) {
    mnemonic += ".W ";
    mnemonic += ourRegisterData[register_number].name;
    mnemonic += ",$";
    mnemonic += IntToString(displacement, 4);
  }

  // If condition code is not true then preform Decrement and Branch
  if (!condition_code) {
    SetRegister(register_number, register_value[register_number] - 1, WORD);
    if ((register_value[register_number] & 0xffff) == 0xffff)
      SetRegister(PC_INDEX, register_value[PC_INDEX] + 2, LONG);
    else
      SetRegister(PC_INDEX, register_value[PC_INDEX] + displacement, LONG);
  } else {
    SetRegister(PC_INDEX, register_value[PC_INDEX] + 2, LONG);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteDIVS(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag, positive_result;
  unsigned long ea_address, register_number;
  int ea_data, high_result, low_result, data;
  std::string ea_description;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, WORD, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address] & 0xffff;
  else {
    unsigned int tmp;

    if ((status = Peek(ea_address, tmp, WORD)) != EXECUTE_OK)
      return (status);

    ea_data = (int)tmp;
  }

  // Are they trying to divide by zero?
  if (ea_data == 0) {
    if ((status = ProcessException(5)) != EXECUTE_OK)
      return (status);
    if (trace)
      ;
    trace_record += "{Mnemonic {Divide by Zero Exception}} ";
    return (EXECUTE_OK);
  }

  // Get the register number
  register_number = D0_INDEX + ((opcode & 0x0e00) >> 9);
  data = register_value[register_number];

  // Determine the sign of the result
  if ((!(ea_data & 0x8000) && !(data & 0x8000000)) ||
      ((ea_data & 0x8000) && (data & 0x80000000)))
    positive_result = 1;
  else
    positive_result = 0;

  // Sign extend the ea_data and the register data
  ea_data = SignExtend(ea_data, WORD);
  data = SignExtend(data, LONG);

  high_result = data % ea_data;
  high_result = high_result << 16;
  low_result = data / ea_data;

  SetRegister(register_number, high_result | (low_result & 0xffff), LONG);
  SetConditionCodes(0, 0, low_result, WORD, OTHER, C_FLAG | Z_FLAG | N_FLAG);

  // Set the overflow flag
  if ((positive_result && (low_result & 0xffff8000)) ||
      (!positive_result && (~low_result & 0xffff8000)))
    register_value[SR_INDEX] |= V_FLAG;
  else
    register_value[SR_INDEX] &= ~V_FLAG;

  if (trace) {
    trace_record += "{Mnemonic {DIVS.W ";
    trace_record += ea_description;
    trace_record += ",";
    trace_record += ourRegisterData[register_number].name;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteDIVU(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int high_result, low_result, ea_data;
  std::string ea_description;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, WORD, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address] & 0xffff;
  else if ((status = Peek(ea_address, ea_data, WORD)) != EXECUTE_OK)
    return (status);

  // Are they trying to divide by zero?
  if (ea_data == 0) {
    if ((status = ProcessException(5)) != EXECUTE_OK)
      return (status);
    if (trace)
      ;
    trace_record += "{Mnemonic {Divide by Zero Exception}} ";
    return (EXECUTE_OK);
  }

  // Get the register number
  register_number = D0_INDEX + ((opcode & 0x0e00) >> 9);

  high_result = register_value[register_number] % ea_data;
  high_result = high_result << 16;
  low_result = register_value[register_number] / ea_data;

  SetRegister(register_number, high_result | (low_result & 0xffff), LONG);
  SetConditionCodes(0, 0, low_result, WORD, OTHER, C_FLAG | Z_FLAG | N_FLAG);

  // Set the overflow flag
  if (low_result & 0xffff0000)
    register_value[SR_INDEX] |= V_FLAG;
  else
    register_value[SR_INDEX] &= ~V_FLAG;

  if (trace) {
    trace_record += "{Mnemonic {DIVU.W ";
    trace_record += ea_description;
    trace_record += ",";
    trace_record += ourRegisterData[register_number].name;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteDIVL(int opcode, std::string &trace_record, int trace) {
  // Note: check update of condition codes!

  int status, in_register_flag;
  unsigned long ea_address;
  unsigned int ea_data;
  unsigned int data;
  std::string ea_description;
  unsigned long address;
  unsigned int opcode2;

  // Operation is DIVx.L <ea>,DI or DIVx.L <ea>,DI:DH
  unsigned long di_number, dh_number;

  // Get the second word
  address = register_value[PC_INDEX];
  register_value[PC_INDEX] += 2;

  if ((status = Peek(address, opcode2, WORD)) != EXECUTE_OK)
    return (status);

  // Check the second opcode
  if (opcode2 & 0x83F8) {
    //**CPU_DEBUG_PRINT1(this, "[DIVL] Bad Opcode 2 value", opcode2);
    // second operand is badly formed
    return (EXECUTE_ILLEGAL_INSTRUCTION);
  }

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, LONG, trace)) != EXECUTE_OK) {
    //**CPU_DEBUG_PRINT1(this, "[DIVL] Bad <ea_address>", (opcode&0x3F));
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, LONG)) != EXECUTE_OK)
    return (status);

  // Compute DI and DH
  dh_number = D0_INDEX + (opcode2 & 0x7);
  di_number = D0_INDEX + ((opcode2 & 0x7000) >> 12);
  data = register_value[di_number];

  //**CPU_DEBUG_PRINT4(this, "[DIVL] OPERATION IS:", hex, data, ea_data, dec);

  if (ea_data == 0) {
    if ((status = ProcessException(5)) != EXECUTE_OK)
      return (status);
    if (trace)
      trace_record += "{Mnemonic {Divide by Zero Exception}} ";
    return (EXECUTE_OK);
  }

  // 64 bits or 32 bits registers for result
  if (opcode2 & (1 << 10)) {
    // 64 bits division *** NOT IMPLEMENTED ***
    //**CPU_DEBUG_PRINT(this, "[DIVxL] 64 bits operation processed as 32
    //bits!");
    // return(EXECUTE_ILLEGAL_INSTRUCTION);

    // Check if signed or unsigned
    if (opcode2 & (1 << 11)) {
      // signed
      long d1, d2;
      long quo, mod;
      int sign = 1;

      d1 = (long)ea_data;
      d2 = (long)data;

      if (d1 < 0) {
        d1 = -d1;
        sign *= -1;
      }
      if (d2 < 0) {
        d2 = -d2;
        sign *= -1;
      }

      quo = sign * d2 / d1;
      mod = (long)data - quo * (long)ea_data;

      //**CPU_DEBUG_PRINT4(this, "[DIVSL] result is", hex, quo, mod, dec);

      SetConditionCodes(0, 0, quo, LONG, OTHER, C_FLAG | Z_FLAG | N_FLAG);
      SetRegister(dh_number, mod, LONG);
      SetRegister(di_number, quo, LONG);

      if (trace) {
        trace_record += "{Mnemonic {DIVSL.L ";
        trace_record += ea_description;
        trace_record += ",";
        trace_record += ourRegisterData[dh_number].name;
        trace_record += ":";
        trace_record += ourRegisterData[di_number].name;
        trace_record += "}} ";
      }
    } else {
      // unsigned
      unsigned long quo, mod;
      quo = data / ea_data;
      mod = data % ea_data;

      //**CPU_DEBUG_PRINT4(this, "[DIVUL] u-result is", hex, quo, mod, dec);
      SetConditionCodes(0, 0, quo, LONG, OTHER,
                        V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
      SetRegister(dh_number, mod, LONG);
      SetRegister(di_number, quo, LONG);

      if (trace) {
        trace_record += "{Mnemonic {DIVUL.L ";
        trace_record += ea_description;
        trace_record += ",";
        trace_record += ourRegisterData[dh_number].name;
        trace_record += ":";
        trace_record += ourRegisterData[di_number].name;
        trace_record += "}} ";
      }
    }
  } else {
    // 32 bits division
    // Check if signed or unsigned
    if (opcode2 & (1 << 11)) {
      // signed
      long d1, d2;
      long quo, mod;
      int sign = 1;

      d1 = (long)ea_data;
      d2 = (long)data;
      if (d1 < 0) {
        d1 = -d1;
        sign *= -1;
      }
      if (d2 < 0) {
        d2 = -d2;
        sign *= -1;
      }

      quo = sign * d2 / d1;
      mod = (long)data - quo * (long)ea_data;

      //**CPU_DEBUG_PRINT4(this, "[DIVL] result is", hex, quo, mod, dec);
      SetConditionCodes(0, 0, quo, LONG, OTHER, C_FLAG | Z_FLAG | N_FLAG);
      SetRegister(dh_number, mod, LONG);
      SetRegister(di_number, quo, LONG);

      if (trace) {
        trace_record += "{Mnemonic {DIVS.L ";
        trace_record += ea_description;
        trace_record += ",";
        trace_record += ourRegisterData[dh_number].name;
        trace_record += ":";
        trace_record += ourRegisterData[di_number].name;
        trace_record += "}} ";
      }
    } else {
      // unsigned
      unsigned long quo, mod;
      quo = data / ea_data;
      mod = data % ea_data;

      //**CPU_DEBUG_PRINT4(this, "[DIVL] u-result is", hex, quo, mod, dec);
      SetConditionCodes(0, 0, quo, LONG, OTHER, C_FLAG | Z_FLAG | N_FLAG);
      SetRegister(dh_number, mod, LONG);
      SetRegister(di_number, quo, LONG);

      if (trace) {
        trace_record += "{Mnemonic {DIVU.L ";
        trace_record += ea_description;
        trace_record += ",";
        trace_record += ourRegisterData[dh_number].name;
        trace_record += ":";
        trace_record += ourRegisterData[di_number].name;
        trace_record += "}} ";
      }
    }
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteEOR(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int result, ea_data;
  std::string mnemonic, ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, size)) != EXECUTE_OK)
    return (status);

  // Get the register number
  register_number = D0_INDEX + ((opcode & 0x0e00) >> 9);

  result = register_value[register_number] ^ ea_data;
  SetConditionCodes(register_value[register_number], ea_data, result, size,
                    OTHER, V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
  if (in_register_flag)
    SetRegister(ea_address, result, size);
  else if ((status = Poke(ea_address, result, size)) != EXECUTE_OK)
    return (status);

  if (trace) {
    mnemonic += "{Mnemonic {EOR";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
    mnemonic += ourRegisterData[register_number].name;
    mnemonic += ",";
    mnemonic += ea_description;
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteEORI(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register;
  unsigned long dest_addr, src_addr;
  unsigned int result, src, dest;
  std::string mnemonic;

  size = (opcode & 0x00c0) >> 6;

  if (trace) {
    mnemonic += "{Mnemonic {EORI";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register, mnemonic, 0x3c,
                                        size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, size)) != EXECUTE_OK)
    return (status);

  if (trace)
    mnemonic += ",";

  // Get the destination data pointer
  if ((status = ComputeEffectiveAddress(dest_addr, in_register, mnemonic,
                                        opcode & 0x3f, size, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register) {
    dest = register_value[dest_addr];
    result = dest ^ src;
    SetConditionCodes(src, dest, result, size, OTHER,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
    SetRegister(dest_addr, result, size);
  } else {
    if ((status = Peek(dest_addr, dest, size)) != EXECUTE_OK)
      return (status);
    result = dest ^ src;
    SetConditionCodes(src, dest, result, size, OTHER,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
    if ((status = Poke(dest_addr, result, size)) != EXECUTE_OK)
      return (status);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteEORItoCCR(int, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long src_addr;
  unsigned int src;
  std::string ea_description;

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register_flag,
                                        ea_description, 0x3c, BYTE, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, BYTE)) != EXECUTE_OK)
    return (status);

  SetRegister(SR_INDEX, register_value[SR_INDEX] ^ src, BYTE);
  if (trace) {
    trace_record += "{Mnemonic {EORI.B ";
    trace_record += ea_description;
    trace_record += ",CCR}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteEORItoSR(int, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long src_addr;
  unsigned int src;
  std::string ea_description;

  // Make sure we're in supervisor mode or trap
  if (!(register_value[SR_INDEX] & S_FLAG)) {
    SetRegister(PC_INDEX, register_value[PC_INDEX] - 2, LONG);
    if ((status = ProcessException(8)) != EXECUTE_OK)
      return (status);
    if (trace)
      ;
    trace_record += "{Mnemonic {Privilege Violation Exception}} ";
    return (EXECUTE_PRIVILEGED_OK);
  }

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register_flag,
                                        ea_description, 0x3c, WORD, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, WORD)) != EXECUTE_OK)
    return (status);

  SetRegister(SR_INDEX, register_value[SR_INDEX] ^ src, WORD);

  if (trace) {
    trace_record += "{Mnemonic {EORI.W ";
    trace_record += ea_description;
    trace_record += ",SR}} ";
  }
  return (EXECUTE_PRIVILEGED_OK);
}

int cpu32::ExecuteEXG(int opcode, std::string &trace_record, int trace) {
  unsigned long tmp;
  unsigned int src_register, dest_register;

  // Get the src and dest registers numbers
  switch ((opcode & 0x00f8) >> 3) {
  case 8: // Data Registers
    src_register = D0_INDEX + ((opcode & 0x0e00) >> 9);
    dest_register = D0_INDEX + (opcode & 0x0007);
    break;
  case 9: // Address Registers
    src_register = A0_INDEX + ((opcode & 0x0e00) >> 9);
    dest_register = A0_INDEX + (opcode & 0x0007);
    if (register_value[SR_INDEX] & S_FLAG) {
      if (src_register == USP_INDEX)
        src_register = SSP_INDEX;
      if (dest_register == USP_INDEX)
        dest_register = SSP_INDEX;
    }
    break;
  case 17: // Data register and Address register
    src_register = D0_INDEX + ((opcode & 0x0e00) >> 9);
    dest_register = A0_INDEX + (opcode & 0x0007);
    if (register_value[SR_INDEX] & S_FLAG) {
      if (dest_register == USP_INDEX)
        dest_register = SSP_INDEX;
    }
    break;
  default:
    src_register = dest_register = 0;
  }

  // Exchange the src and dest registers
  tmp = register_value[src_register];
  SetRegister(src_register, register_value[dest_register], LONG);
  SetRegister(dest_register, tmp, LONG);

  if (trace) {
    trace_record += "{Mnemonic {EXG.L ";
    trace_record += ourRegisterData[src_register].name;
    trace_record += ",";
    trace_record += ourRegisterData[dest_register].name;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteEXT(int opcode, std::string &trace_record, int trace) {
  unsigned int register_number;
  unsigned long data;

  if (trace)
    trace_record += "{Mnemonic {EXT";

  // Get the data register number
  register_number = D0_INDEX + (opcode & 0x0007);
  data = register_value[register_number];

  // Extend the data
  if (((opcode & 0x01c0) >> 6) == 2) {
    data = SignExtend(data, BYTE);
    SetRegister(register_number, data, WORD);
    SetConditionCodes(0, 0, data, WORD, OTHER,
                      V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
    if (trace)
      trace_record += ".W ";
  } else if (((opcode & 0x01c0) >> 6) == 3) {
    data = SignExtend(data, WORD);
    SetRegister(register_number, data, LONG);
    SetConditionCodes(0, 0, data, LONG, OTHER,
                      V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
    if (trace)
      trace_record += ".L ";
  } else if (((opcode & 0x01c0) >> 6) == 7) {
    data = SignExtend(data, BYTE);
    SetRegister(register_number, data, LONG);
    SetConditionCodes(0, 0, data, LONG, OTHER,
                      V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
    if (trace)
      trace_record += "B.L ";
  }

  if (trace) {
    trace_record += ourRegisterData[register_number].name;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteILLEGAL(int, std::string &trace_record, int trace) {
  int status;

  // Move the PC back to the start of the illegal instruction opcode
  SetRegister(PC_INDEX, register_value[PC_INDEX] - 2, LONG);

  // Process the illegal instruction exception
  if ((status = ProcessException(4)) != EXECUTE_OK)
    return (status);

  if (trace)
    trace_record += "{Mnemonic {ILLEGAL}} ";

  return (EXECUTE_OK);
}

int cpu32::ExecuteJMP(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long address;
  std::string ea_description;

  // Get the effective address
  if ((status =
           ComputeEffectiveAddress(address, in_register_flag, ea_description,
                                   opcode & 0x3f, LONG, trace)) != EXECUTE_OK) {
    return (status);
  }

  SetRegister(PC_INDEX, address, LONG);

  if (trace) {
    trace_record += "{Mnemonic {JMP ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteJSR(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long address, stack_address;
  std::string ea_description;

  // Get the effective address
  if ((status =
           ComputeEffectiveAddress(address, in_register_flag, ea_description,
                                   opcode & 0x3f, LONG, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Push the PC onto the stack
  if (register_value[SR_INDEX] & S_FLAG) {
    SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 4, LONG);
    stack_address = register_value[SSP_INDEX];
  } else {
    SetRegister(USP_INDEX, register_value[USP_INDEX] - 4, LONG);
    stack_address = register_value[USP_INDEX];
  }
  if ((status = Poke(stack_address, register_value[PC_INDEX], LONG)) !=
      EXECUTE_OK)
    return (status);

  SetRegister(PC_INDEX, address, LONG);

  if (trace) {
    trace_record += "{Mnemonic {JSR ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteLEA(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long address, register_number;
  std::string ea_description;

  // for EXTB instruction, if mode (bit 3 to 5) is 0 then
  // it is not a LEA but an EXTB!
  if ((opcode & 0x38) == 0) {
    return ExecuteEXT(opcode, trace_record, trace);
  }

  // Get the effective address
  if ((status =
           ComputeEffectiveAddress(address, in_register_flag, ea_description,
                                   opcode & 0x3f, LONG, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Get the address register number
  register_number = A0_INDEX + ((opcode & 0x0e00) >> 9);

  // Adjust register_number if it's A7 and we're in supervisor mode
  if ((register_number == USP_INDEX) && (register_value[SR_INDEX] & S_FLAG))
    register_number = SSP_INDEX;

  SetRegister(register_number, address, LONG);

  if (trace) {
    trace_record += "{Mnemonic {LEA.L ";
    trace_record += ea_description;
    trace_record += ",";
    trace_record += ourRegisterData[register_number].name;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteLINK(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag, stack_index;
  unsigned int register_number, displacement;
  unsigned long address;
  std::string ea_description;

  // Long operation
  if ((opcode & 0x8)) {
    //**CPU_DEBUG_PRINT2(this, "[cpu32] LINK: long version not implemented",
    //hex, opcode);
    // NOT IMPLEMENTED
    return EXECUTE_ILLEGAL_INSTRUCTION;
  }

  // Get the displacement data pointer
  if ((status = ComputeEffectiveAddress(address, in_register_flag,
                                        ea_description, 0x3c, WORD, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  // Fetch the displacement data
  if ((status = Peek(address, displacement, WORD)) != EXECUTE_OK)
    return (status);
  displacement = SignExtend(displacement, WORD);

  // Get the address register number
  register_number = A0_INDEX + (opcode & 0x0007);

  // Adjust register_number if it's A7 and we're in supervisor mode
  if ((register_number == USP_INDEX) && (register_value[SR_INDEX] & S_FLAG))
    register_number = SSP_INDEX;

  // Get the stack index
  if (register_value[SR_INDEX] & S_FLAG)
    stack_index = SSP_INDEX;
  else
    stack_index = USP_INDEX;

  // Push the address register onto the stack
  SetRegister(stack_index, register_value[stack_index] - 4, LONG);
  if ((status = Poke(register_value[stack_index],
                     register_value[register_number], LONG)) != EXECUTE_OK) {
    return (status);
  }

  // Move the stack pointer into the address register
  SetRegister(register_number, register_value[stack_index], LONG);

  // Add displacement to the stack pointer
  SetRegister(stack_index, register_value[stack_index] + displacement, LONG);

  if (trace) {
    trace_record += "{Mnemonic {LINK ";
    trace_record += ourRegisterData[register_number].name;
    trace_record += ",";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteLSL(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag, shift_count;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Check to see if this is a memory or register shift
  if (size == 3) {
    size = WORD; // Memory always shifts a word

    // Get the address
    if ((status = ComputeEffectiveAddress(address, in_register_flag,
                                          ea_description, opcode & 0x3f, size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }

    // Fetch the data
    if ((status = Peek(address, data, size)) != EXECUTE_OK)
      return (status);

    // Shift the data to the left by one bit.
    data = data << 1;

    // Store the shifted data
    if ((status = Poke(address, data, size)) != EXECUTE_OK)
      return (status);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | X_FLAG | C_FLAG | V_FLAG);
    if (data & 0x00010000) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }
  } else {
    // Compute the shift count
    if (opcode & 32) {
      shift_count = register_value[D0_INDEX + ((opcode & 0x0e00) >> 9)] & 0x3f;
      if (trace)
        ea_description =
            ourRegisterData[D0_INDEX + ((opcode & 0x0e00) >> 9)].name;
    } else {
      if ((shift_count = (opcode & 0x0e00) >> 9) == 0)
        shift_count = 8;

      if (trace) {
        ea_description = "#$";
        ea_description += IntToString(shift_count, 1);
      }
    }

    if (trace) {
      ea_description += ",";
      ea_description += ourRegisterData[D0_INDEX + (opcode & 7)].name;
    }

    unsigned int carry = 0;
    unsigned int carry_mask;

    // Setup masks
    if (size == BYTE)
      carry_mask = 0x80;
    else if (size == WORD)
      carry_mask = 0x8000;
    else
      carry_mask = 0x80000000;

    // Perform the shift on the data
    data = register_value[D0_INDEX + (opcode & 7)];
    for (int t = 0; t < shift_count; ++t) {
      carry = data & carry_mask;
      data = data << 1;
    }

    SetRegister(D0_INDEX + (opcode & 7), data, size);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | X_FLAG | C_FLAG | V_FLAG);
    if (carry) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }
  }

  if (trace) {
    trace_record += "{Mnemonic {LSL";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteLSR(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag, shift_count;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Check to see if this is a memory or register shift
  if (size == 3) {
    size = WORD; // Memory always shifts a word

    // Get the address
    if ((status = ComputeEffectiveAddress(address, in_register_flag,
                                          ea_description, opcode & 0x3f, size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }

    // Fetch the data
    if ((status = Peek(address, data, size)) != EXECUTE_OK)
      return (status);

    // Set the condition codes
    SetConditionCodes(0, 0, (data >> 1) & 0x7fff, size, OTHER,
                      N_FLAG | Z_FLAG | X_FLAG | C_FLAG | V_FLAG);
    if (data & 0x0001) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }

    // Shift the data to the right by one bit.
    data = (data >> 1) & 0x7fff;

    // Store the shifted data
    if ((status = Poke(address, data, size)) != EXECUTE_OK)
      return (status);
  } else {
    // Compute the shift count
    if (opcode & 32) {
      shift_count = register_value[D0_INDEX + ((opcode & 0x0e00) >> 9)] & 0x3f;
      if (trace)
        ea_description =
            ourRegisterData[D0_INDEX + ((opcode & 0x0e00) >> 9)].name;
    } else {
      if ((shift_count = (opcode & 0x0e00) >> 9) == 0)
        shift_count = 8;

      if (trace) {
        ea_description = "#$";
        ea_description += IntToString(shift_count, 1);
      }
    }

    if (trace) {
      ea_description += ",";
      ea_description += ourRegisterData[D0_INDEX + (opcode & 7)].name;
    }

    unsigned int carry = 0, clear_mask;

    // Setup masks
    if (size == BYTE)
      clear_mask = 0x7f;
    else if (size == WORD)
      clear_mask = 0x7fff;
    else
      clear_mask = 0x7fffffff;

    // Perform the shift on the data
    data = register_value[D0_INDEX + (opcode & 7)];
    for (int t = 0; t < shift_count; ++t) {
      carry = data & 0x00000001;
      data = (data >> 1) & clear_mask;
    }

    SetRegister(D0_INDEX + (opcode & 7), data, size);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | X_FLAG | C_FLAG | V_FLAG);
    if (carry) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }
  }

  if (trace) {
    trace_record += "{Mnemonic {LSR";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteMOVE(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag, size;
  unsigned long src_address, dest_address;
  unsigned int src;
  std::string ea_description;

  switch ((opcode & 0x3000) >> 12) {
  case 1:
    size = BYTE;
    break;
  case 3:
    size = WORD;
    break;
  case 2:
    size = LONG;
    break;
  default:
    size = 0;
  }

  // Get the source effective address
  if ((status = ComputeEffectiveAddress(src_address, in_register_flag,
                                        ea_description, opcode & 0x3f, size,
                                        trace)) != EXECUTE_OK) {
    return (status);
  }

  if (in_register_flag)
    src = register_value[src_address];
  else if ((status = Peek(src_address, src, size)) != EXECUTE_OK)
    return (status);

  if (trace)
    ea_description += ",";

  // Get the destination effective address
  if ((status = ComputeEffectiveAddress(
           dest_address, in_register_flag, ea_description,
           ((opcode & 0x01c0) >> 3) | ((opcode & 0x0e00) >> 9), size, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register_flag)
    SetRegister(dest_address, src, size);
  else if ((status = Poke(dest_address, src, size)) != EXECUTE_OK)
    return (status);

  SetConditionCodes(0, 0, src, size, OTHER, N_FLAG | Z_FLAG | V_FLAG | C_FLAG);
  if (trace) {
    trace_record += "{Mnemonic {MOVE";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteMOVEA(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag, size;
  unsigned long src_address, dest_address;
  unsigned int src;
  std::string ea_description;

  switch ((opcode & 0x3000) >> 12) {
  case 3:
    size = WORD;
    break;
  case 2:
    size = LONG;
    break;
  default:
    size = 0;
  }

  // Get the source effective address
  if ((status = ComputeEffectiveAddress(src_address, in_register_flag,
                                        ea_description, opcode & 0x3f, size,
                                        trace)) != EXECUTE_OK) {
    return (status);
  }

  if (in_register_flag)
    src = register_value[src_address];
  else if ((status = Peek(src_address, src, size)) != EXECUTE_OK)
    return (status);

  if (size == WORD)
    src = SignExtend(src, WORD);

  if (trace)
    ea_description += ",";

  // Get the destination effective address
  if ((status = ComputeEffectiveAddress(
           dest_address, in_register_flag, ea_description,
           ((opcode & 0x01c0) >> 3) | ((opcode & 0x0e00) >> 9), LONG, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register_flag)
    SetRegister(dest_address, src, LONG);
  else if ((status = Poke(dest_address, src, LONG)) != EXECUTE_OK)
    return (status);

  if (trace) {
    trace_record += "{Mnemonic {MOVEA";
    if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteMOVEC(int opcode, std::string &trace_record, int trace) {
  int status;
  unsigned long address;
  unsigned int opcode2;
  int general_index; // index of the general purpose register
  int special_index; // index of the special register

  // Make sure we are in supervisor mode or trap
  if (!(register_value[SR_INDEX] & S_FLAG)) {
    SetRegister(PC_INDEX, register_value[PC_INDEX] - 2, LONG);
    if ((status = ProcessException(8)) != EXECUTE_OK)
      return (status);
    if (trace)
      ;
    trace_record += "{Mnemonic {Privilege Violation Exception}} ";
    return (EXECUTE_PRIVILEGED_OK);
  }

  // Get the second word
  address = register_value[PC_INDEX];
  register_value[PC_INDEX] += 2;

  if ((status = Peek(address, opcode2, WORD)) != EXECUTE_OK)
    return (status);

  general_index = ((opcode2 & 0x7000) >> 12);
  if (opcode2 & 0x8000) {
    general_index += A0_INDEX;
  } else {
    general_index += D0_INDEX;
  }
  switch (opcode2 & 0xFFF) {
  case 0x0:
    special_index = SFC_INDEX;
    break;
  case 0x1:
    special_index = DFC_INDEX;
    break;
  case 0x800:
    special_index = USP_INDEX;
    break;
  case 0x801:
    special_index = VBR_INDEX;
    break;
  default:
    return (EXECUTE_ILLEGAL_INSTRUCTION);
    break;
  }

  if (opcode & 1) // bit 0 gives the direction of the transfert
  {
    register_value[special_index] = register_value[general_index];
    if (trace) {
      trace_record += "{Mnemonic {MOVEC ";
      trace_record += ourRegisterData[general_index].name;
      trace_record += ",";
      trace_record += ourRegisterData[special_index].name;
      trace_record += "}} ";
    }

  } else {
    register_value[general_index] = register_value[special_index];
    if (trace) {
      trace_record += "{Mnemonic {MOVEC ";
      trace_record += ourRegisterData[special_index].name;
      trace_record += ",";
      trace_record += ourRegisterData[general_index].name;
      trace_record += "}} ";
    }
  }

  return (EXECUTE_OK);
}

int cpu32::ExecuteMOVES(int opcode, std::string &trace_record, int trace) {
  int status;
  int size;
  int in_register_flag;
  unsigned long address;
  unsigned int opcode2;
  unsigned int src;
  int general_index; // index of the general purpose register
  std::string ea_description;

  // Make sure we are in supervisor mode or trap
  if (!(register_value[SR_INDEX] & S_FLAG)) {
    SetRegister(PC_INDEX, register_value[PC_INDEX] - 2, LONG);
    if ((status = ProcessException(8)) != EXECUTE_OK)
      return (status);
    if (trace)
      ;
    trace_record += "{Mnemonic {Privilege Violation Exception}} ";
    return (EXECUTE_PRIVILEGED_OK);
  }

  switch ((opcode >> 6) & 0x3) {
  case 1:
    size = BYTE;
    break;
  case 3:
    size = WORD;
    break;
  case 2:
    size = LONG;
    break;
  default:
    size = 0;
  }

  // Get the second word
  if ((status = Peek(register_value[PC_INDEX], opcode2, WORD)) != EXECUTE_OK)
    return (status);
  register_value[PC_INDEX] += 2;

  // Get the effective address (either source or destination
  // depending on the transfert direction)
  if ((status =
           ComputeEffectiveAddress(address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  general_index = ((opcode2 & 0x7000) >> 12);
  if (opcode2 & 0x8000) {
    general_index += A0_INDEX;
  } else {
    general_index += D0_INDEX;
  }

  if (opcode2 & (1 << 11)) // bit 11 gives the direction of the transfert
  {
    // from general register to ea
    if (in_register_flag)
      register_value[address] = register_value[general_index];
    else if ((status = Poke(address, register_value[general_index], size)) !=
             EXECUTE_OK)
      return (status);
    if (trace) {
      trace_record += "{Mnemonic {MOVES";
      if (size == BYTE)
        trace_record += ".B ";
      else if (size == WORD)
        trace_record += ".W ";
      else if (size == LONG)
        trace_record += ".L ";
      trace_record += ourRegisterData[general_index].name;
      trace_record += ",";
      trace_record += ea_description;
      trace_record += "}} ";
    }
  } else {
    // from ea to general_register
    if (in_register_flag) {
      register_value[general_index] = register_value[address];
    } else {
      if ((status = Peek(address, src, size)) != EXECUTE_OK)
        return (status);
      register_value[general_index] = src;
    }

    if (trace) {
      trace_record += "{Mnemonic {MOVES ";
      if (size == BYTE)
        trace_record += ".B ";
      else if (size == WORD)
        trace_record += ".W ";
      else if (size == LONG)
        trace_record += ".L ";
      trace_record += ea_description;
      trace_record += ",";
      trace_record += ourRegisterData[general_index].name;
      trace_record += "}} ";
    }
  }

  return (EXECUTE_OK);
}

int cpu32::ExecuteMOVEM(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag, size;
  unsigned long address, offset, reg;
  unsigned int list, data;
  std::string ea_description, mnemonic;

  // Determine size and offset
  if (opcode & 64) {
    offset = 4;
    size = LONG;
  } else {
    offset = 2;
    size = WORD;
  }

  if (trace) {
    mnemonic += "{Mnemonic {MOVEM";
    if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  // Get the register list mask
  if ((status = Peek(register_value[PC_INDEX], list, WORD)) != EXECUTE_OK)
    return (status);
  SetRegister(PC_INDEX, register_value[PC_INDEX] + 2, LONG);

  // Get the effective address (if this isn't predecrement)
  if ((opcode & 0x38) != 32) {
    if ((status = ComputeEffectiveAddress(address, in_register_flag,
                                          ea_description, opcode & 0x3f, size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }
  }

  if ((opcode & 0x38) == 32) // Predecrement mode
  {
    if ((register_value[SR_INDEX] & S_FLAG) &&
        ((A0_INDEX + (opcode & 7)) == USP_INDEX))
      reg = SSP_INDEX;
    else
      reg = A0_INDEX + (opcode & 7);
    address = register_value[reg];
    if (trace) {
      ea_description += "-(";
      ea_description += ourRegisterData[reg].name;
      ea_description += ")";
    }

    for (unsigned int t = A0_INDEX + 7;;) {
      if (list & (1 << (A0_INDEX + 7 - t))) {
        if ((register_value[SR_INDEX] & S_FLAG) && (t == USP_INDEX))
          reg = SSP_INDEX;
        else
          reg = t;

        address -= offset;
        if ((status = Poke(address, register_value[reg], size)) != EXECUTE_OK)
          return (status);

        if (trace) {
          mnemonic += ourRegisterData[reg].name;
          mnemonic += " ";
        }
      }

      if (t == D0_INDEX)
        break;
      else
        --t;
    }
    if (trace) {
      mnemonic += ",";
      mnemonic += ea_description;
    }
  } else // Postincrement or Control mode
  {
    if (trace) {
      mnemonic += ea_description;
      mnemonic += ",";
    }
    for (unsigned int t = D0_INDEX; t <= A0_INDEX + 7; ++t) {
      if (list & (1 << (t - D0_INDEX))) {
        if ((register_value[SR_INDEX] & S_FLAG) && (t == USP_INDEX))
          reg = SSP_INDEX;
        else
          reg = t;

        if (opcode & 1024) {
          if ((status = Peek(address, data, size)) != EXECUTE_OK)
            return (status);
          SetRegister(reg, data, size);
        } else {
          if ((status = Poke(address, register_value[reg], size)) != EXECUTE_OK)
            return (status);
        }
        address += offset;

        if (trace) {
          mnemonic += ourRegisterData[reg].name;
          mnemonic += " ";
        }
      }
    }
  }

  if (((opcode & 0x38) == 32) || ((opcode & 0x38) == 24)) {
    if ((register_value[SR_INDEX] & S_FLAG) && ((opcode & 7) == 7))
      SetRegister(SSP_INDEX, address, LONG);
    else
      SetRegister(A0_INDEX + (opcode & 7), address, LONG);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteMOVEP(int opcode, std::string &trace_record, int trace) {
  unsigned long Dn;
  int in_register_flag;
  unsigned long src_address, dest_address;
  std::string ea_description;
  int status;

  // Get the data register we're working with
  Dn = D0_INDEX + ((opcode & 0x0e00) >> 9);

  switch ((opcode & 0x01c0) >> 6) {
  case 4: // WORD from Memory to Register
  {
    // Get the source effective address
    if ((status = ComputeEffectiveAddress(
             src_address, in_register_flag, ea_description,
             (0x28 | (opcode & 0x7)), WORD, trace)) != EXECUTE_OK) {
      return status;
    }

    unsigned int b0, b1;

    if ((status = Peek((src_address + 0), b1, BYTE)) != EXECUTE_OK)
      return status;
    if ((status = Peek((src_address + 2), b0, BYTE)) != EXECUTE_OK)
      return status;

    SetRegister(Dn, (b1 << 8) | b0, WORD);

    if (trace) {
      trace_record += "{Mnemonic {MOVEP.W ";
      trace_record += ea_description;
      trace_record += ",D";
      trace_record += IntToString(Dn, 1);
      trace_record += "}} ";
    }
    break;
  }

  case 5: // LONG from Memory to Register
  {
    // Get the source effective address
    if ((status = ComputeEffectiveAddress(
             src_address, in_register_flag, ea_description,
             (0x28 | (opcode & 0x7)), LONG, trace)) != EXECUTE_OK) {
      return (status);
    }

    unsigned int b0, b1, b2, b3;

    if ((status = Peek((src_address + 0), b3, BYTE)) != EXECUTE_OK)
      return status;
    if ((status = Peek((src_address + 2), b2, BYTE)) != EXECUTE_OK)
      return status;
    if ((status = Peek((src_address + 4), b1, BYTE)) != EXECUTE_OK)
      return status;
    if ((status = Peek((src_address + 6), b0, BYTE)) != EXECUTE_OK)
      return status;

    SetRegister(Dn, (b3 << 24) | (b2 << 16) | (b1 << 8) | b0, LONG);

    if (trace) {
      trace_record += "{Mnemonic {MOVEP.L ";
      trace_record += ea_description;
      trace_record += ",D";
      trace_record += IntToString(Dn, 1);
      trace_record += "}} ";
    }
    break;
  }

  case 6: // WORD from Register to Memory
  {
    // Get the destination address
    if ((status = ComputeEffectiveAddress(
             dest_address, in_register_flag, ea_description,
             (0x28 | (opcode & 0x7)), WORD, trace)) != EXECUTE_OK) {
      return status;
    }

    unsigned int value = register_value[Dn];

    if ((status = Poke((dest_address + 0), value >> 8, BYTE)) != EXECUTE_OK)
      return status;
    if ((status = Poke((dest_address + 2), value, BYTE)) != EXECUTE_OK)
      return status;

    if (trace) {
      trace_record += "{Mnemonic {MOVEP.W D";
      trace_record += IntToString(Dn, 1);
      trace_record += ",";
      trace_record += ea_description;
      trace_record += "}} ";
    }
    break;
  }

  case 7: // LONG from Register to Memory
  {
    // Get the destination address
    if ((status = ComputeEffectiveAddress(
             dest_address, in_register_flag, ea_description,
             (0x28 | (opcode & 0x7)), LONG, trace)) != EXECUTE_OK) {
      return (status);
    }

    unsigned value = register_value[Dn];

    if ((status = Poke((dest_address + 0), value >> 24, BYTE)) != EXECUTE_OK)
      return status;
    if ((status = Poke((dest_address + 2), value >> 16, BYTE)) != EXECUTE_OK)
      return status;
    if ((status = Poke((dest_address + 4), value >> 8, BYTE)) != EXECUTE_OK)
      return status;
    if ((status = Poke((dest_address + 6), value, BYTE)) != EXECUTE_OK)
      return status;

    if (trace) {
      trace_record += "{Mnemonic {MOVEP.L D";
      trace_record += IntToString(Dn, 1);
      trace_record += ",";
      trace_record += ea_description;
      trace_record += "}} ";
    }
    break;
  }
  }

  return EXECUTE_OK;
}

int cpu32::ExecuteMOVEQ(int opcode, std::string &trace_record, int trace) {
  unsigned long register_number;
  unsigned int data;

  // Get the destination data register number
  register_number = D0_INDEX + ((opcode >> 9) & 0x07);

  // Get the immediate data
  data = SignExtend((opcode & 0xff), BYTE);

  SetConditionCodes(0, 0, data, LONG, OTHER, C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
  SetRegister(register_number, data, LONG);

  if (trace) {
    trace_record += "{Mnemonic {MOVEQ.L #$";
    trace_record += IntToString(data, 2);
    trace_record += ",";
    trace_record += ourRegisterData[register_number].name;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteMOVEfromSR(int opcode, std::string &trace_record, int trace) {
  int status, in_register;
  unsigned long address;
  std::string ea_description;

  // Get the destination data pointer
  if ((status = ComputeEffectiveAddress(address, in_register, ea_description,
                                        opcode & 0x3f, WORD, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register)
    SetRegister(address, register_value[SR_INDEX], WORD);
  else if ((status = Poke(address, register_value[SR_INDEX], WORD)) !=
           EXECUTE_OK)
    return (status);

  if (trace) {
    trace_record += "{Mnemonic {MOVE.W SR,";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteMOVEUSP(int opcode, std::string &trace_record, int trace) {
  int status, register_number;
  std::string ea_description;

  // Make sure we're in supervisor mode or trap
  if (!(register_value[SR_INDEX] & S_FLAG)) {
    SetRegister(PC_INDEX, register_value[PC_INDEX] - 2, LONG);
    if ((status = ProcessException(8)) != EXECUTE_OK)
      return (status);
    if (trace)
      ;
    trace_record += "{Mnemonic {Privilege Violation Exception}} ";
    return (EXECUTE_PRIVILEGED_OK);
  }

  // Get the address register index
  if ((opcode & 7) == 7)
    register_number = SSP_INDEX;
  else
    register_number = A0_INDEX + (opcode & 7);

  // Move from or to the USP
  if (opcode & 8) {
    SetRegister(register_number, register_value[USP_INDEX], LONG);
    if (trace) {
      ea_description = "USP,";
      ea_description += ourRegisterData[register_number].name;
    }
  } else {
    SetRegister(USP_INDEX, register_value[register_number], LONG);
    if (trace) {
      ea_description = ourRegisterData[register_number].name;
      ea_description += ",USP";
    }
  }

  if (trace) {
    trace_record += "{Mnemonic {MOVE.L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_PRIVILEGED_OK);
}

int cpu32::ExecuteMOVEtoCCR(int opcode, std::string &trace_record, int trace) {
  int status, in_register;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  // Get the destination data pointer
  if ((status = ComputeEffectiveAddress(address, in_register, ea_description,
                                        opcode & 0x3f, WORD, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register)
    data = register_value[address];
  else if ((status = Peek(address, data, WORD)) != EXECUTE_OK)
    return (status);

  SetRegister(SR_INDEX, data, BYTE);

  if (trace) {
    trace_record += "{Mnemonic {MOVE.W ";
    trace_record += ea_description;
    trace_record += ",CCR}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteMOVEtoSR(int opcode, std::string &trace_record, int trace) {
  int status, in_register;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  // Make sure we're in supervisor mode or trap
  if (!(register_value[SR_INDEX] & S_FLAG)) {
    SetRegister(PC_INDEX, register_value[PC_INDEX] - 2, LONG);
    if ((status = ProcessException(8)) != EXECUTE_OK)
      return (status);
    if (trace)
      ;
    trace_record += "{Mnemonic {Privilege Violation Exception}} ";
    return (EXECUTE_PRIVILEGED_OK);
  }

  // Get the destination data pointer
  if ((status = ComputeEffectiveAddress(address, in_register, ea_description,
                                        opcode & 0x3f, WORD, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register)
    data = register_value[address];
  else if ((status = Peek(address, data, WORD)) != EXECUTE_OK)
    return (status);

  SetRegister(SR_INDEX, data, WORD);

  if (trace) {
    trace_record += "{Mnemonic {MOVE.W ";
    trace_record += ea_description;
    trace_record += ",SR}} ";
  }
  return (EXECUTE_PRIVILEGED_OK);
}

int cpu32::ExecuteMULS(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int ea_data;
  int result, data;
  std::string ea_description;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, WORD, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, WORD)) != EXECUTE_OK)
    return (status);

  // Get the register number
  register_number = D0_INDEX + ((opcode & 0x0e00) >> 9);
  data = register_value[register_number];

  // Sign extend operands
  data = SignExtend(data, WORD);
  ea_data = SignExtend(ea_data, WORD);

  result = data * ea_data;
  SetConditionCodes(0, 0, result, LONG, OTHER,
                    V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
  SetRegister(register_number, result, LONG);

  if (trace) {
    trace_record += "{Mnemonic {MULS.W ";
    trace_record += ea_description;
    trace_record += ",";
    trace_record += ourRegisterData[register_number].name;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteMULU(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int result, ea_data;
  std::string ea_description;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, WORD, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address] & 0xffff;
  else if ((status = Peek(ea_address, ea_data, WORD)) != EXECUTE_OK)
    return (status);

  // Get the register number
  register_number = D0_INDEX + ((opcode & 0x0e00) >> 9);

  result = (register_value[register_number] & 0xffff) * ea_data;
  SetConditionCodes(0, 0, result, LONG, OTHER,
                    V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
  SetRegister(register_number, result, LONG);

  if (trace) {
    trace_record += "{Mnemonic {MULU.W ";
    trace_record += ea_description;
    trace_record += ",";
    trace_record += ourRegisterData[register_number].name;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteMULL(int opcode, std::string &trace_record, int trace) {
  // Note: check update of condition codes!
  int status, in_register_flag;
  unsigned long ea_address;
  unsigned int ea_data;
  unsigned int data;
  std::string ea_description;
  unsigned long address;
  unsigned int opcode2;
  unsigned long l1 = 0, l2 = 0, w3 = 0, w4 = 0;
  int sign = 1;

  // Operation is MULx.L <ea>,DI or MULx.L <ea>,DI:DH
  unsigned long di_number, dh_number;

  // Get the second word
  address = register_value[PC_INDEX];
  register_value[PC_INDEX] += 2;

  if ((status = Peek(address, opcode2, WORD)) != EXECUTE_OK)
    return (status);

  // Check the second opcode
  if (opcode2 & 0x83F8) {
    //**CPU_DEBUG_PRINT1(this, "[MULL] Bad Opcode 2 value", opcode2);
    // second operand is badly formed
    return (EXECUTE_ILLEGAL_INSTRUCTION);
  }

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, LONG, trace)) != EXECUTE_OK) {
    //**CPU_DEBUG_PRINT1(this, "[MULL] Bad <ea_address>", (opcode&0x3F));
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, LONG)) != EXECUTE_OK)
    return (status);

  // Compute DI and DH
  dh_number = D0_INDEX + (opcode2 & 0x7);
  di_number = D0_INDEX + ((opcode2 & 0x7000) >> 12);
  data = register_value[di_number];
  //**CPU_DEBUG_PRINT2(this, "[MULL] DH=%d, DI=%d\n", (opcode2&0x7),
  //((opcode2&0x7000)>>12));

  // If signed, memorize sign.
  if (opcode2 & (1 << 11)) {
    // signed
    if (((long)ea_data) < 0) {
      sign = -1 * sign;
      ea_data = -ea_data;
      ;
    }
    if (((long)data) < 0) {
      sign = -1 * sign;
      data = -data;
    }
    //**CPU_DEBUG_PRINT1(this, "[MULL] Signed operation: sign is", sign);
  }
  //**CPU_DEBUG_PRINT4(this, "[MULL] OPERATION IS:", hex, sign, ea_data, data);

  // Compute as if 64bits
  // result is l1,w3,w4 (4 words)
  w4 = (ea_data & 0xFFFF) * (data & 0xFFFF);
  w3 += w4 >> 16;
  w4 &= 0xFFFF;
  w3 += (ea_data & 0xFFFF) * ((data >> 16) & 0xFFFF) +
        ((ea_data >> 16) & 0xFFFF) * (data & 0xFFFF);
  l1 += w3 >> 16;
  w3 &= 0xFFFF;
  l1 += ((ea_data >> 16) & 0xFFFF) * ((data >> 16) & 0xFFFF);
  l2 = (w3 << 16) + w4;

  //**CPU_DEBUG_PRINT4(this, "[MULL] result is", hex, l1, l2, dec);

  // 64bits or 32bits
  if (opcode2 & (1 << 10)) {
    // 64bits
    if (dh_number == di_number) {
      //**CPU_DEBUG_PRINT0(this, "[MULL] 64bits registers not distinct.");
      // the registers must differ (in theory, result is simply undefined)
      return (EXECUTE_ILLEGAL_INSTRUCTION);
    }
    // Check if signed or unsigned
    if (opcode2 & (1 << 11)) {
      // signed
      l1 *= sign;
      SetConditionCodes(0, 0, l1, LONG, OTHER,
                        V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
      SetRegister(dh_number, l1, LONG);
      SetRegister(di_number, l2, LONG);

      if (trace) {
        trace_record += "{Mnemonic {MULS.L ";
        trace_record += ea_description;
        trace_record += ",";
        trace_record += ourRegisterData[dh_number].name;
        trace_record += ":";
        trace_record += ourRegisterData[di_number].name;
        trace_record += "}} ";
      }
    } else {
      // unsigned
      SetConditionCodes(0, 0, l1, LONG, OTHER,
                        V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
      SetRegister(dh_number, l1, LONG);
      SetRegister(di_number, l2, LONG);

      if (trace) {
        trace_record += "{Mnemonic {MULU.L ";
        trace_record += ea_description;
        trace_record += ",";
        trace_record += ourRegisterData[dh_number].name;
        trace_record += ":";
        trace_record += ourRegisterData[di_number].name;
        trace_record += "}} ";
      }
    }
  } else {
    // 32bits
    // Check if signed or unsigned
    if (opcode2 & (1 << 11)) {
      // signed
      l2 *= sign;
      SetConditionCodes(0, 0, l2, LONG, OTHER,
                        V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
      SetRegister(di_number, l2, LONG);

      if (trace) {
        trace_record += "{Mnemonic {MULS.L ";
        trace_record += ea_description;
        trace_record += ",";
        trace_record += ourRegisterData[di_number].name;
        trace_record += "}} ";
      }
    } else {
      // unsigned
      SetConditionCodes(0, 0, l2, LONG, OTHER,
                        V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
      SetRegister(di_number, l2, LONG);

      if (trace) {
        trace_record += "{Mnemonic {MULU.L ";
        trace_record += ea_description;
        trace_record += ",";
        trace_record += ourRegisterData[di_number].name;
        trace_record += "}} ";
      }
    }
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteNBCD(int opcode, std::string &trace_record, int trace) {
  // Long Link
  if (((opcode >> 3) & 0x7) == 1) {
    return ExecuteLINK(opcode, trace_record, trace);
  }

  return (ExecuteInvalid(opcode, trace_record, trace));
}

int cpu32::ExecuteNEG(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long address;
  unsigned int data, result;
  std::string ea_description;

  size = ((opcode & 0x00c0) >> 6);

  // Get the effective address
  if ((status =
           ComputeEffectiveAddress(address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the data
  if (in_register_flag)
    data = register_value[address];
  else if ((status = Peek(address, data, size)) != EXECUTE_OK)
    return (status);

  data = SignExtend(data, size);
  result = 0 - data;
  SetConditionCodes(data, 0, result, size, SUBTRACTION,
                    C_FLAG | X_FLAG | V_FLAG | Z_FLAG | N_FLAG);

  // Store the result
  if (in_register_flag)
    SetRegister(address, result, size);
  else if ((status = Poke(address, result, size)) != EXECUTE_OK)
    return (status);

  if (trace) {
    trace_record += "{Mnemonic {NEG";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteNEGX(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long address;
  unsigned int data, result;
  std::string ea_description;

  size = ((opcode & 0x00c0) >> 6);

  // Get the effective address
  if ((status =
           ComputeEffectiveAddress(address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the data
  if (in_register_flag)
    data = register_value[address];
  else if ((status = Peek(address, data, size)) != EXECUTE_OK)
    return (status);

  data = SignExtend(data, size);

  if (register_value[SR_INDEX] & X_FLAG)
    result = 0 - data - 1;
  else
    result = 0 - data;

  SetConditionCodes(data, 0, result, size, SUBTRACTION,
                    C_FLAG | X_FLAG | V_FLAG | N_FLAG);
  if (size == BYTE)
    result = result & 0xff;
  else if (size == WORD)
    result = result & 0xffff;
  else if (size == LONG)
    result = result & 0xffffffff;

  if (result)
    register_value[SR_INDEX] &= ~Z_FLAG;

  // Store the result
  if (in_register_flag)
    SetRegister(address, result, size);
  else if ((status = Poke(address, result, size)) != EXECUTE_OK)
    return (status);

  if (trace) {
    trace_record += "{Mnemonic {NEGX";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteNOP(int, std::string &trace_record, int trace) {
  if (trace)
    trace_record += "{Mnemonic {NOP}}";

  return (EXECUTE_OK);
}

int cpu32::ExecuteNOT(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long address;
  unsigned int data, result;
  std::string ea_description;

  size = ((opcode & 0x00c0) >> 6);

  // Get the effective address
  if ((status =
           ComputeEffectiveAddress(address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the data
  if (in_register_flag)
    data = register_value[address];
  else if ((status = Peek(address, data, size)) != EXECUTE_OK)
    return (status);

  result = ~data;
  SetConditionCodes(data, 0, result, size, OTHER,
                    C_FLAG | V_FLAG | Z_FLAG | N_FLAG);

  // Store the result
  if (in_register_flag)
    SetRegister(address, result, size);
  else if ((status = Poke(address, result, size)) != EXECUTE_OK)
    return (status);

  if (trace) {
    trace_record += "{Mnemonic {NOT";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteOR(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int result, ea_data;
  std::string mnemonic, ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, size)) != EXECUTE_OK)
    return (status);

  // Get the register number
  register_number = D0_INDEX + ((opcode & 0x0e00) >> 9);

  if (trace) {
    mnemonic += "{Mnemonic {OR";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  if (opcode & 0x0100) // <Dn> | <ea> -> <ea>
  {
    if (trace) {
      mnemonic += ourRegisterData[register_number].name;
      mnemonic += ",";
      mnemonic += ea_description;
    }
    result = register_value[register_number] | ea_data;
    SetConditionCodes(register_value[register_number], ea_data, result, size,
                      OTHER, V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
    if ((status = Poke(ea_address, result, size)) != EXECUTE_OK)
      return (status);
  } else // <ea> | <Dn> -> <Dn>
  {
    if (trace) {
      mnemonic += ea_description;
      mnemonic += ",";
      mnemonic += ourRegisterData[register_number].name;
    }
    result = ea_data | register_value[register_number];
    SetConditionCodes(ea_data, register_value[register_number], result, size,
                      OTHER, V_FLAG | C_FLAG | Z_FLAG | N_FLAG);
    SetRegister(register_number, result, size);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteORI(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register;
  unsigned long dest_addr, src_addr;
  unsigned int result, src, dest;
  std::string mnemonic;

  size = (opcode & 0x00c0) >> 6;

  if (trace) {
    mnemonic += "{Mnemonic {ORI";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register, mnemonic, 0x3c,
                                        size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, size)) != EXECUTE_OK)
    return (status);

  if (trace)
    mnemonic += ",";

  // Get the destination data pointer
  if ((status = ComputeEffectiveAddress(dest_addr, in_register, mnemonic,
                                        opcode & 0x3f, size, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register) {
    dest = register_value[dest_addr];
    result = dest | src;
    SetConditionCodes(src, dest, result, size, OTHER,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
    SetRegister(dest_addr, result, size);
  } else {
    if ((status = Peek(dest_addr, dest, size)) != EXECUTE_OK)
      return (status);
    result = dest | src;
    SetConditionCodes(src, dest, result, size, OTHER,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
    if ((status = Poke(dest_addr, result, size)) != EXECUTE_OK)
      return (status);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteORItoCCR(int, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long src_addr;
  unsigned int src;
  std::string mnemonic;

  if (trace)
    mnemonic += "{Mnemonic {ORI.B ";

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register_flag, mnemonic,
                                        0x3c, BYTE, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, BYTE)) != EXECUTE_OK)
    return (status);

  SetRegister(SR_INDEX, register_value[SR_INDEX] | src, BYTE);
  if (trace) {
    mnemonic += ",CCR}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteORItoSR(int, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long src_addr;
  unsigned int src;
  std::string mnemonic;

  // Make sure we're in supervisor mode or trap
  if (!(register_value[SR_INDEX] & S_FLAG)) {
    SetRegister(PC_INDEX, register_value[PC_INDEX] - 2, LONG);
    if ((status = ProcessException(8)) != EXECUTE_OK)
      return (status);
    if (trace)
      ;
    trace_record += "{Mnemonic {Privilege Violation Exception}} ";
    return (EXECUTE_PRIVILEGED_OK);
  }

  if (trace)
    mnemonic += "{Mnemonic {ORI.W ";

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register_flag, mnemonic,
                                        0x3c, WORD, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, WORD)) != EXECUTE_OK)
    return (status);

  SetRegister(SR_INDEX, register_value[SR_INDEX] | src, WORD);

  if (trace) {
    mnemonic += ",SR}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_PRIVILEGED_OK);
}

int cpu32::ExecutePEA(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag, stack_index;
  unsigned long address;
  std::string ea_description;

  // Get the effective address
  if ((status =
           ComputeEffectiveAddress(address, in_register_flag, ea_description,
                                   opcode & 0x3f, LONG, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Get the stack pointer index
  if (register_value[SR_INDEX] & S_FLAG)
    stack_index = SSP_INDEX;
  else
    stack_index = USP_INDEX;

  // Push the effective address onto the stack
  SetRegister(stack_index, register_value[stack_index] - 4, LONG);
  if ((status = Poke(register_value[stack_index], address, LONG)) != EXECUTE_OK)
    return (status);

  if (trace) {
    trace_record += "{Mnemonic {PEA.L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteRESET(int, std::string &trace_record, int trace) {
  int status;

  // Make sure we're in supervisor mode or trap
  if (!(register_value[SR_INDEX] & S_FLAG)) {
    SetRegister(PC_INDEX, register_value[PC_INDEX] - 2, LONG);
    if ((status = ProcessException(8)) != EXECUTE_OK)
      return (status);
    if (trace)
      ;
    trace_record += "{Mnemonic {Privilege Violation Exception}} ";
    return (EXECUTE_PRIVILEGED_OK);
  }

  // Tell the AddressSpace to reset all of the attached devices
  myAddressSpaces[0]->Reset();

  if (trace)
    trace_record += "{Mnemonic {RESET}} ";

  return (EXECUTE_PRIVILEGED_OK);
}

int cpu32::ExecuteROL(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag, shift_count;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Check to see if this is a memory or register rotate
  if (size == 3) {
    size = WORD; // Memory always rotates a word

    // Get the address
    if ((status = ComputeEffectiveAddress(address, in_register_flag,
                                          ea_description, opcode & 0x3f, size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }

    // Fetch the data
    if ((status = Peek(address, data, size)) != EXECUTE_OK)
      return (status);

    // Rotate the data to the left by one bit.
    if (data & 0x8000)
      data = (data << 1) | 0x0001;
    else
      data = (data << 1);

    // Store the shifted data
    if ((status = Poke(address, data, size)) != EXECUTE_OK)
      return (status);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | C_FLAG | V_FLAG);
    if (data & 0x0001)
      register_value[SR_INDEX] |= C_FLAG;
  } else {
    // Compute the shift count
    if (opcode & 32) {
      shift_count = register_value[D0_INDEX + ((opcode & 0x0e00) >> 9)] & 0x3f;
      if (trace)
        ea_description =
            ourRegisterData[D0_INDEX + ((opcode & 0x0e00) >> 9)].name;
    } else {
      if ((shift_count = (opcode & 0x0e00) >> 9) == 0)
        shift_count = 8;

      if (trace) {
        ea_description = "#$";
        ea_description += IntToString(shift_count, 1);
      }
    }

    if (trace) {
      ea_description += ",";
      ea_description += ourRegisterData[D0_INDEX + (opcode & 7)].name;
    }

    unsigned int msb;

    // Setup MSB
    if (size == BYTE)
      msb = 0x80;
    else if (size == WORD)
      msb = 0x8000;
    else
      msb = 0x80000000;

    // Perform the shift on the data
    data = register_value[D0_INDEX + (opcode & 7)];
    for (int t = 0; t < shift_count; ++t) {
      if (data & msb)
        data = (data << 1) | 1;
      else
        data = (data << 1);
    }

    SetRegister(D0_INDEX + (opcode & 7), data, size);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | C_FLAG | V_FLAG);
    if (data & 1)
      register_value[SR_INDEX] |= C_FLAG;
  }

  if (trace) {
    trace_record += "{Mnemonic {ROL";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteROR(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag, shift_count;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Check to see if this is a memory or register rotate
  if (size == 3) {
    size = WORD; // Memory always rotates a word

    // Get the address
    if ((status = ComputeEffectiveAddress(address, in_register_flag,
                                          ea_description, opcode & 0x3f, size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }

    // Fetch the data
    if ((status = Peek(address, data, size)) != EXECUTE_OK)
      return (status);

    // Rotate the data to the right by one bit.
    if (data & 1)
      data = 0x8000 | (data >> 1);
    else
      data = (data >> 1) & 0x7fff;

    // Store the shifted data
    if ((status = Poke(address, data, size)) != EXECUTE_OK)
      return (status);

    // Set the condition codes
    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | C_FLAG | V_FLAG);
    if (data & 0x8000)
      register_value[SR_INDEX] |= C_FLAG;
  } else {
    // Compute the shift count
    if (opcode & 32) {
      shift_count = register_value[D0_INDEX + ((opcode & 0x0e00) >> 9)] & 0x3f;
      if (trace)
        ea_description =
            ourRegisterData[D0_INDEX + ((opcode & 0x0e00) >> 9)].name;
    } else {
      if ((shift_count = (opcode & 0x0e00) >> 9) == 0)
        shift_count = 8;

      if (trace) {
        ea_description = "#$";
        ea_description += IntToString(shift_count, 1);
      }
    }

    if (trace) {
      ea_description += ",";
      ea_description += ourRegisterData[D0_INDEX + (opcode & 7)].name;
    }

    unsigned int msb;

    // Setup most sign. byte indicator
    if (size == BYTE)
      msb = 0x80;
    else if (size == WORD)
      msb = 0x8000;
    else
      msb = 0x80000000;

    // Perform the shift on the data
    data = register_value[D0_INDEX + (opcode & 7)];
    for (int t = 0; t < shift_count; ++t) {
      if (data & 1)
        data = msb | (data >> 1);
      else
        data = (data >> 1) & ~msb;
    }

    SetRegister(D0_INDEX + (opcode & 7), data, size);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | C_FLAG | V_FLAG);
    if (data & msb)
      register_value[SR_INDEX] |= C_FLAG;
  }

  if (trace) {
    trace_record += "{Mnemonic {ROR";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteROXL(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag, shift_count;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Check to see if this is a memory or register rotate
  if (size == 3) {
    size = WORD; // Memory always rotates a word

    // Get the address
    if ((status = ComputeEffectiveAddress(address, in_register_flag,
                                          ea_description, opcode & 0x3f, size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }

    // Fetch the data
    if ((status = Peek(address, data, size)) != EXECUTE_OK)
      return (status);

    // Rotate the data to the left by one bit.
    if (register_value[SR_INDEX] & X_FLAG)
      data = (data << 1) | 0x0001;
    else
      data = (data << 1);

    // Store the shifted data
    if ((status = Poke(address, data, size)) != EXECUTE_OK)
      return (status);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | C_FLAG | X_FLAG | V_FLAG);
    if (data & 0x00010000) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }
  } else {
    // Compute the shift count
    if (opcode & 32) {
      shift_count = register_value[D0_INDEX + ((opcode & 0x0e00) >> 9)] & 0x3f;
      if (trace)
        ea_description =
            ourRegisterData[D0_INDEX + ((opcode & 0x0e00) >> 9)].name;
    } else {
      if ((shift_count = (opcode & 0x0e00) >> 9) == 0)
        shift_count = 8;

      if (trace) {
        ea_description = "#$";
        ea_description += IntToString(shift_count, 1);
      }
    }

    if (trace) {
      ea_description += ",";
      ea_description += ourRegisterData[D0_INDEX + (opcode & 7)].name;
    }

    unsigned int msb, extend;

    // Setup msb
    if (size == BYTE)
      msb = 0x80;
    else if (size == WORD)
      msb = 0x8000;
    else
      msb = 0x80000000;

    // Set extend
    extend = register_value[SR_INDEX] & X_FLAG;

    // Perform the shift on the data
    data = register_value[D0_INDEX + (opcode & 7)];
    for (int t = 0; t < shift_count; ++t) {
      if (extend) {
        extend = data & msb;
        data = (data << 1) | 1;
      } else {
        extend = data & msb;
        data = (data << 1);
      }
    }

    SetRegister(D0_INDEX + (opcode & 7), data, size);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | X_FLAG | C_FLAG | V_FLAG);
    if (extend) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }
  }

  if (trace) {
    trace_record += "{Mnemonic {ROXL";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteROXR(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag, shift_count;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Check to see if this is a memory or register rotate
  if (size == 3) {
    size = WORD; // Memory always rotates a word

    // Get the address
    if ((status = ComputeEffectiveAddress(address, in_register_flag,
                                          ea_description, opcode & 0x3f, size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }

    // Fetch the data
    if ((status = Peek(address, data, size)) != EXECUTE_OK)
      return (status);

    // Set carry flag
    int carry = data & 1;

    // Rotate the data to the right by one bit.
    if (register_value[SR_INDEX] & X_FLAG)
      data = 0x8000 | (data >> 1);
    else
      data = (data >> 1) & 0x7fff;

    // Store the shifted data
    if ((status = Poke(address, data, size)) != EXECUTE_OK)
      return (status);

    // Set the condition codes
    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | X_FLAG | C_FLAG | V_FLAG);
    if (carry) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }
  } else {
    // Compute the shift count
    if (opcode & 32) {
      shift_count = register_value[D0_INDEX + ((opcode & 0x0e00) >> 9)] & 0x3f;
      if (trace)
        ea_description =
            ourRegisterData[D0_INDEX + ((opcode & 0x0e00) >> 9)].name;
    } else {
      if ((shift_count = (opcode & 0x0e00) >> 9) == 0)
        shift_count = 8;

      if (trace) {
        ea_description = "#$";
        ea_description += IntToString(shift_count, 1);
      }
    }

    if (trace) {
      ea_description += ",";
      ea_description += ourRegisterData[D0_INDEX + (opcode & 7)].name;
    }

    unsigned int msb, extend;

    // Setup most sign. byte indicator
    if (size == BYTE)
      msb = 0x80;
    else if (size == WORD)
      msb = 0x8000;
    else
      msb = 0x80000000;

    extend = register_value[SR_INDEX] & X_FLAG;

    // Perform the shift on the data
    data = register_value[D0_INDEX + (opcode & 7)];
    for (int t = 0; t < shift_count; ++t) {
      if (extend) {
        extend = data & 1;
        data = msb | (data >> 1);
      } else {
        extend = data & 1;
        data = (data >> 1) & ~msb;
      }
    }

    SetRegister(D0_INDEX + (opcode & 7), data, size);

    SetConditionCodes(0, 0, data, size, OTHER,
                      N_FLAG | Z_FLAG | X_FLAG | C_FLAG | V_FLAG);
    if (extend) {
      register_value[SR_INDEX] |= C_FLAG;
      register_value[SR_INDEX] |= X_FLAG;
    }
  }

  if (trace) {
    trace_record += "{Mnemonic {ROXR";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteRTE(int, std::string &trace_record, int trace) {
  int status;
  unsigned int sr, pc;
  unsigned int offset;

  // Make sure we're in supervisor mode or trap
  if (!(register_value[SR_INDEX] & S_FLAG)) {
    SetRegister(PC_INDEX, register_value[PC_INDEX] - 2, LONG);
    if ((status = ProcessException(8)) != EXECUTE_OK)
      return (status);
    if (trace)
      ;
    trace_record += "{Mnemonic {Privilege Violation Exception}} ";
    return (EXECUTE_PRIVILEGED_OK);
  }

  // Pop the SR off the stack
  if ((status = Peek(register_value[SSP_INDEX], sr, WORD)) != EXECUTE_OK)
    return (status);

  SetRegister(SSP_INDEX, register_value[SSP_INDEX] + 2, LONG);
  SetRegister(SR_INDEX, sr, WORD);

  // Pop the PC off the stack
  if ((status = Peek(register_value[SSP_INDEX], pc, LONG)) != EXECUTE_OK)
    return (status);

  SetRegister(SSP_INDEX, register_value[SSP_INDEX] + 4, LONG);
  SetRegister(PC_INDEX, pc, LONG);

  // Pop the vector offset off the stack
  if ((status = Peek(register_value[SSP_INDEX], offset, WORD)) != EXECUTE_OK)
    return (status);

  // should I consider the format of the offset (bit 12-15) ?
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] + 2, LONG);

  if (trace)
    trace_record += "{Mnemonic {RTE}} ";

  return (EXECUTE_PRIVILEGED_OK);
}

int cpu32::ExecuteRTD(int opcode, std::string &trace_record, int trace) {
  // Return And Deallocate
  int status;
  unsigned int pc;
  unsigned int extend_word;
  int stack_register;

  //**CPU_DEBUG_PRINT2(this,"[cpu32] RTD", hex, register_value[PC_INDEX]);

  // Determine which stack pointer to use
  if (register_value[SR_INDEX] & S_FLAG)
    stack_register = SSP_INDEX;
  else
    stack_register = USP_INDEX;

  if ((status = Peek(register_value[PC_INDEX], extend_word, WORD)) !=
      EXECUTE_OK) {
    register_value[PC_INDEX] += 2;
    return (status);
  } else {
    register_value[PC_INDEX] += 2;
  }
  extend_word = SignExtend(extend_word, WORD);

  // Pop the PC off the stack
  if ((status = Peek(register_value[stack_register], pc, LONG)) != EXECUTE_OK)
    return (status);

  SetRegister(stack_register, register_value[stack_register] + 4 + extend_word,
              LONG);
  SetRegister(PC_INDEX, pc, LONG);

  if (trace) {
    trace_record += "{Mnemonic {RTD #";
    trace_record += IntToString(extend_word, 4);
    trace_record += "}} ";
  }

  return (EXECUTE_OK);
}

int cpu32::ExecuteRTR(int, std::string &trace_record, int trace) {
  int status;
  unsigned int ccr, pc;
  int stackRegister;

  // Determine which stack pointer to use
  if (register_value[SR_INDEX] & S_FLAG)
    stackRegister = SSP_INDEX;
  else
    stackRegister = USP_INDEX;

  // Pop the CCR off the stack
  if ((status = Peek(register_value[stackRegister], ccr, WORD)) != EXECUTE_OK)
    return (status);

  SetRegister(stackRegister, register_value[stackRegister] + 2, LONG);
  SetRegister(SR_INDEX, ccr, BYTE);

  // Pop the PC off the stack
  if ((status = Peek(register_value[stackRegister], pc, LONG)) != EXECUTE_OK)
    return (status);

  SetRegister(stackRegister, register_value[stackRegister] + 4, LONG);
  SetRegister(PC_INDEX, pc, LONG);

  if (trace)
    trace_record += "{Mnemonic {RTR}} ";

  return (EXECUTE_OK);
}

int cpu32::ExecuteRTS(int, std::string &trace_record, int trace) {
  int status;
  unsigned int pc;
  int stackRegister;

  // Determine which stack pointer to use
  if (register_value[SR_INDEX] & S_FLAG)
    stackRegister = SSP_INDEX;
  else
    stackRegister = USP_INDEX;

  // Pop the PC off the stack
  if ((status = Peek(register_value[stackRegister], pc, LONG)) != EXECUTE_OK)
    return (status);

  SetRegister(stackRegister, register_value[stackRegister] + 4, LONG);
  SetRegister(PC_INDEX, pc, LONG);

  if (trace)
    trace_record += "{Mnemonic {RTS}} ";

  return (EXECUTE_OK);
}

int cpu32::ExecuteSBCD(int opcode, std::string &trace_record, int trace) {
  return (ExecuteInvalid(opcode, trace_record, trace));
}

int cpu32::ExecuteSTOP(int, std::string &trace_record, int trace) {
  unsigned int newStatusRegister;
  int status;
  std::string mnemonic;

  // Fetch the 16-bit immediate data
  status = Peek(register_value[PC_INDEX], newStatusRegister, WORD);
  if (status != EXECUTE_OK)
    return (status);

  // Increment the program counter
  SetRegister(PC_INDEX, register_value[PC_INDEX] + 2, LONG);

  // Set the status register
  SetRegister(SR_INDEX, newStatusRegister, WORD);

  // Make sure the S-Bit is set
  if (!(register_value[SR_INDEX] & S_FLAG)) {
    if ((status = ProcessException(8)) != EXECUTE_OK)
      return (status);
    if (trace)
      trace_record += "{Mnemonic {Privilege Violation Exception}} ";
    return (EXECUTE_PRIVILEGED_OK);
  }

  // Stop the processor
  myState = STOP_STATE;

  if (trace) {
    trace_record += "{Mnemonic {STOP #$";
    trace_record += IntToString(newStatusRegister, 4);
    trace_record += "}} ";
  }
  return (EXECUTE_PRIVILEGED_OK);
}

int cpu32::ExecuteBREAK(int, std::string &trace_record, int trace) {

  // Put the processor in our "fake" break state so the simulator will
  // stop running a program.
  myState = BREAK_STATE;

  if (trace)
    trace_record += "{Mnemonic {BREAK}} ";

  return (EXECUTE_OK);
}

int cpu32::ExecuteSUB(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int result, ea_data;
  std::string mnemonic, ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, size)) != EXECUTE_OK)
    return (status);

  // Get the register number
  register_number = D0_INDEX + ((opcode & 0x0e00) >> 9);

  if (trace) {
    mnemonic += "{Mnemonic {SUB";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  if (opcode & 0x0100) // <ea> - <Dn> -> <ea>
  {
    if (trace) {
      mnemonic += ourRegisterData[register_number].name;
      mnemonic += ",";
      mnemonic += ea_description;
    }
    result = ea_data - register_value[register_number];
    SetConditionCodes(register_value[register_number], ea_data, result, size,
                      SUBTRACTION, C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
    if ((status = Poke(ea_address, result, size)) != EXECUTE_OK)
      return (status);
  } else // <Dn> - <ea> -> <Dn>
  {
    if (trace) {
      mnemonic += ea_description;
      mnemonic += ",";
      mnemonic += ourRegisterData[register_number].name;
    }
    result = register_value[register_number] - ea_data;
    SetConditionCodes(ea_data, register_value[register_number], result, size,
                      SUBTRACTION, C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
    SetRegister(register_number, result, size);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteSUBA(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, register_number;
  unsigned int result, ea_data;
  std::string ea_description;

  if (opcode & 0x0100)
    size = LONG;
  else
    size = WORD;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, size)) != EXECUTE_OK)
    return (status);

  ea_data = SignExtend(ea_data, size);

  // Get the register number
  register_number = A0_INDEX + ((opcode & 0x0e00) >> 9);

  // Adjust register_number if it's A7 and we're in supervisor mode
  if ((register_number == USP_INDEX) && (register_value[SR_INDEX] & S_FLAG))
    register_number = SSP_INDEX;

  result = register_value[register_number] - ea_data;
  SetRegister(register_number, result, LONG);

  if (trace) {
    trace_record += "{Mnemonic {SUBA";
    if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += ",";
    trace_record += ourRegisterData[register_number].name;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteSUBI(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register;
  unsigned long dest_addr, src_addr;
  unsigned int result, src, dest;
  std::string mnemonic;

  size = (opcode & 0x00c0) >> 6;

  if (trace) {
    mnemonic += "{Mnemonic {SUBI";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  // Get the immediate data pointer
  if ((status = ComputeEffectiveAddress(src_addr, in_register, mnemonic, 0x3c,
                                        size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the immediate data
  if ((status = Peek(src_addr, src, size)) != EXECUTE_OK)
    return (status);

  if (trace)
    mnemonic += ",";

  // Get the destination data pointer
  if ((status = ComputeEffectiveAddress(dest_addr, in_register, mnemonic,
                                        opcode & 0x3f, size, trace)) !=
      EXECUTE_OK) {
    return (status);
  }

  if (in_register) {
    dest = register_value[dest_addr];
    result = dest - src;
    SetConditionCodes(src, dest, result, size, SUBTRACTION,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
    SetRegister(dest_addr, result, size);
  } else {
    if ((status = Peek(dest_addr, dest, size)) != EXECUTE_OK)
      return (status);
    result = dest - src;
    SetConditionCodes(src, dest, result, size, SUBTRACTION,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
    if ((status = Poke(dest_addr, result, size)) != EXECUTE_OK)
      return (status);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteSUBQ(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long ea_address, immediate_data;
  unsigned int result, ea_data;
  std::string ea_description;

  size = (opcode & 0x00c0) >> 6;

  // Get the immediate data out of the opcode
  if ((immediate_data = (opcode & 0x0e00) >> 9) == 0)
    immediate_data = 8;

  // Get the <ea> data address
  if ((status =
           ComputeEffectiveAddress(ea_address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the <ea> data
  if (in_register_flag)
    ea_data = register_value[ea_address];
  else if ((status = Peek(ea_address, ea_data, size)) != EXECUTE_OK)
    return (status);

  if (in_register_flag) {
    result = ea_data - immediate_data;
    if ((ea_address >= A0_INDEX) && (ea_address <= SSP_INDEX)) {
      SetRegister(ea_address, result, LONG);
    } else {
      SetConditionCodes(immediate_data, ea_data, result, size, SUBTRACTION,
                        C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
      SetRegister(ea_address, result, size);
    }
  } else {
    result = ea_data - immediate_data;
    SetConditionCodes(immediate_data, ea_data, result, size, SUBTRACTION,
                      C_FLAG | V_FLAG | Z_FLAG | N_FLAG | X_FLAG);
    if ((status = Poke(ea_address, result, size)) != EXECUTE_OK)
      return (status);
  }

  if (trace) {
    trace_record += "{Mnemonic {SUBQ";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += "#$";
    trace_record += IntToString(immediate_data, 1);
    trace_record += ",";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteSUBX(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long src_address, dest_address;
  unsigned int result, src, dest;
  std::string mnemonic;

  size = (opcode & 0x00c0) >> 6;

  if (trace) {
    mnemonic += "{Mnemonic {SUBX";
    if (size == BYTE)
      mnemonic += ".B ";
    else if (size == WORD)
      mnemonic += ".W ";
    else if (size == LONG)
      mnemonic += ".L ";
  }

  // Get the addresses
  if (opcode & 8) {
    if ((status = ComputeEffectiveAddress(src_address, in_register_flag,
                                          mnemonic, 0x20 | (opcode & 7), size,
                                          trace)) != EXECUTE_OK) {
      return (status);
    }

    if (trace)
      mnemonic += ",";

    if ((status = ComputeEffectiveAddress(
             dest_address, in_register_flag, mnemonic,
             0x20 | ((opcode & 0x0e00) >> 9), size, trace)) != EXECUTE_OK) {
      return (status);
    }

    if ((status = Peek(src_address, src, size)) != EXECUTE_OK)
      return (status);
    if ((status = Peek(dest_address, dest, size)) != EXECUTE_OK)
      return (status);
  } else {
    src_address = D0_INDEX + (opcode & 0x0007);
    src = register_value[src_address];
    dest_address = D0_INDEX + ((opcode & 0x0e00) >> 9);
    dest = register_value[dest_address];
    if (trace) {
      mnemonic += ourRegisterData[src_address].name;
      mnemonic += ",";
      mnemonic += ourRegisterData[dest_address].name;
    }
  }

  if (register_value[SR_INDEX] & X_FLAG)
    result = dest - src - 1;
  else
    result = dest - src;

  if (size == BYTE)
    result = result & 0xff;
  else if (size == WORD)
    result = result & 0xffff;
  else if (size == LONG)
    result = result & 0xffffffff;

  SetConditionCodes(src, dest, result, size, SUBTRACTION,
                    C_FLAG | V_FLAG | N_FLAG | X_FLAG);
  if (result)
    register_value[SR_INDEX] &= ~Z_FLAG;

  if (opcode & 8) {
    if ((status = Poke(dest_address, result, size)) != EXECUTE_OK)
      return (status);
  } else {
    SetRegister(dest_address, result, size);
  }

  if (trace) {
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteSWAP(int opcode, std::string &trace_record, int trace) {
  int register_number;
  unsigned int data;

  register_number = D0_INDEX + (opcode & 7);
  data = register_value[register_number];
  data = ((data >> 16) & 0xffff) | ((data << 16) & 0xffff0000);
  SetRegister(register_number, data, LONG);
  SetConditionCodes(0, 0, data, LONG, OTHER, C_FLAG | V_FLAG | Z_FLAG | N_FLAG);
  if (trace) {
    trace_record += "{Mnemonic {SWAP.W ";
    trace_record += ourRegisterData[register_number].name;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteScc(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long address;
  unsigned int result;
  std::string mnemonic, ea_description;

  if (trace)
    trace_record += "{Mnemonic {S";

  // Get the effective address
  if ((status =
           ComputeEffectiveAddress(address, in_register_flag, ea_description,
                                   opcode & 0x3f, BYTE, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Check to see if the result should be all 1's or 0's
  if (CheckConditionCodes((opcode & 0x0f00) >> 8, mnemonic, trace))
    result = 0xff;
  else
    result = 0;

  // Store the result
  if (in_register_flag)
    SetRegister(address, result, BYTE);
  else if ((status = Poke(address, result, BYTE)) != EXECUTE_OK)
    return (status);

  if (trace) {
    mnemonic += ".B ";
    mnemonic += ea_description;
    mnemonic += "}} ";
    trace_record += mnemonic;
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteTAS(int opcode, std::string &trace_record, int trace) {
  int status, in_register_flag;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  // Get the effective address
  if ((status =
           ComputeEffectiveAddress(address, in_register_flag, ea_description,
                                   opcode & 0x3f, BYTE, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the data
  if (in_register_flag)
    data = register_value[address];
  else if ((status = Peek(address, data, BYTE)) != EXECUTE_OK)
    return (status);

  SetConditionCodes(0, 0, data, BYTE, OTHER, C_FLAG | V_FLAG | Z_FLAG | N_FLAG);

  // Set the high order bit
  data |= 0x80;

  // Store the result
  if (in_register_flag)
    SetRegister(address, data, BYTE);
  else if ((status = Poke(address, data, BYTE)) != EXECUTE_OK)
    return (status);

  if (trace) {
    trace_record += "{Mnemonic {TAS.B ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteTRAP(int opcode, std::string &trace_record, int trace) {
  int status;

  // Process the exception
  if ((status = ProcessException(32 + (opcode & 0xf))) != EXECUTE_OK)
    return (status);

  if (trace) {
    trace_record += "{Mnemonic {TRAP #$";
    trace_record += IntToString(32 + (opcode & 0xf), 1);
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteTRAPV(int, std::string &trace_record, int trace) {
  int status;

  // If the overflow bit is set then trap
  if (register_value[SR_INDEX] & V_FLAG) {
    // Process the exception
    if ((status = ProcessException(7)) != EXECUTE_OK)
      return (status);
  }

  if (trace)
    trace_record += "{Mnemonic {TRAPV}} ";

  return (EXECUTE_OK);
}

int cpu32::ExecuteTST(int opcode, std::string &trace_record, int trace) {
  int status, size, in_register_flag;
  unsigned long address;
  unsigned int data;
  std::string ea_description;

  size = ((opcode & 0x00c0) >> 6);

  // Get the effective address
  if ((status =
           ComputeEffectiveAddress(address, in_register_flag, ea_description,
                                   opcode & 0x3f, size, trace)) != EXECUTE_OK) {
    return (status);
  }

  // Fetch the data
  if (in_register_flag)
    data = register_value[address];
  else if ((status = Peek(address, data, size)) != EXECUTE_OK)
    return (status);

  SetConditionCodes(0, 0, data, size, OTHER, C_FLAG | V_FLAG | Z_FLAG | N_FLAG);

  if (trace) {
    trace_record += "{Mnemonic {TST";
    if (size == BYTE)
      trace_record += ".B ";
    else if (size == WORD)
      trace_record += ".W ";
    else if (size == LONG)
      trace_record += ".L ";
    trace_record += ea_description;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteUNLK(int opcode, std::string &trace_record, int trace) {
  int status, stack_index;
  unsigned int register_number, fp;

  // Get the address register number
  register_number = A0_INDEX + (opcode & 0x0007);

  // Adjust register_number if it's A7 and we're in supervisor mode
  if ((register_number == USP_INDEX) && (register_value[SR_INDEX] & S_FLAG))
    register_number = SSP_INDEX;

  // Get the stack index
  if (register_value[SR_INDEX] & S_FLAG)
    stack_index = SSP_INDEX;
  else
    stack_index = USP_INDEX;

  // Set the stack pointer to the frame pointer
  SetRegister(stack_index, register_value[register_number], LONG);

  // Pop the frame pointer from the stack
  if ((status = Peek(register_value[stack_index], fp, LONG)) != EXECUTE_OK) {
    return (status);
  }
  SetRegister(register_number, fp, LONG);
  SetRegister(stack_index, register_value[stack_index] + 4, LONG);

  if (trace) {
    trace_record += "{Mnemonic {UNLK ";
    trace_record += ourRegisterData[register_number].name;
    trace_record += "}} ";
  }
  return (EXECUTE_OK);
}

int cpu32::ExecuteInvalid(int, std::string &trace_record, int trace) {
  int status;

  // Move the PC back to the start of the illegal instruction opcode
  SetRegister(PC_INDEX, register_value[PC_INDEX] - 2, LONG);

  // Process the illegal instruction exception
  if ((status = ProcessException(4)) != EXECUTE_OK)
    return (status);

  if (trace)
    trace_record += "{Mnemonic {Illegal Instruction Exception}} ";

  return (EXECUTE_OK);
}

int cpu32::ProcessException(int vector) {
  int status;

  // Copy the SR to a temp
  unsigned long sr = register_value[SR_INDEX];

  // Change to Supervisor mode and clear the Trace mode
  register_value[SR_INDEX] |= S_FLAG;
  register_value[SR_INDEX] &= ~T_FLAG;

  // Push the vector offset (format is 0000)
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 2, LONG);
  if ((status = Poke(register_value[SSP_INDEX], 4 * vector, WORD)) !=
      EXECUTE_OK) {
    return (status);
  }

  // Push the PC and SR onto the supervisor stack
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 4, LONG);
  if ((status = Poke(register_value[SSP_INDEX], register_value[PC_INDEX],
                     LONG)) != EXECUTE_OK) {
    return (status);
  }

  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 2, LONG);
  if ((status = Poke(register_value[SSP_INDEX], sr, WORD)) != EXECUTE_OK) {
    return (status);
  }

  // Get the service routine's address
  unsigned int service_address;
  if ((status = Peek(register_value[VBR_INDEX] + vector * 4, service_address,
                     LONG)) != EXECUTE_OK)
    return (status);

  // Change the program counter to the service routine's address
  SetRegister(PC_INDEX, service_address, LONG);

  return (EXECUTE_OK);
}

int cpu32::ExecuteAddressError(int opcode, std::string &trace_record, int trace) {
  int status;

  // Copy the SR to a temp
  unsigned long sr = register_value[SR_INDEX];

  // Change to Supervisor mode and clear the Trace mode
  register_value[SR_INDEX] |= S_FLAG;
  register_value[SR_INDEX] &= ~T_FLAG;

  // Push the PC
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 4, LONG);
  if ((status = Poke(register_value[SSP_INDEX], register_value[PC_INDEX],
                     LONG)) != EXECUTE_OK) {
    return (status);
  }

  // Push the SR (tmp)
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 2, LONG);
  if ((status = Poke(register_value[SSP_INDEX], sr, WORD)) != EXECUTE_OK) {
    return (status);
  }

  // Push the Opcode Word of instruction in Error
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 2, LONG);
  if ((status = Poke(register_value[SSP_INDEX], opcode, WORD)) != EXECUTE_OK) {
    return (status);
  }

  // NOTE: The following two values are not correct!!! The correct values
  //       could be derived by adding a few variables to the CPU, but
  //       I don't have time to do it now :-)
  //
  // Push Memory Address at the Fault
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 4, LONG);
  if ((status = Poke(register_value[SSP_INDEX], 0, LONG)) != EXECUTE_OK) {
    return (status);
  }

  // Push status information word
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 2, LONG);
  if ((status = Poke(register_value[SSP_INDEX], 9, WORD)) != EXECUTE_OK) {
    return (status);
  }

  // Get the service routine's address
  unsigned int service_address;
  if ((status = Peek(register_value[VBR_INDEX] + 0x0c, service_address,
                     LONG)) != EXECUTE_OK)
    return (status);

  // Change the program counter to the service routine's address
  SetRegister(PC_INDEX, service_address, LONG);

  if (trace)
    trace_record += "{Mnemonic {Address Error Exception}} ";

  return (EXECUTE_OK);
}

int cpu32::ExecuteBusError(int opcode, std::string &trace_record, int trace) {
  int status;

  // Copy the SR to a temp
  unsigned long sr = register_value[SR_INDEX];

  // Change to Supervisor mode and clear the Trace mode
  register_value[SR_INDEX] |= S_FLAG;
  register_value[SR_INDEX] &= ~T_FLAG;

  // Push the PC
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 4, LONG);
  if ((status = Poke(register_value[SSP_INDEX], register_value[PC_INDEX],
                     LONG)) != EXECUTE_OK) {
    return (status);
  }

  // Push the SR (tmp)
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 2, LONG);
  if ((status = Poke(register_value[SSP_INDEX], sr, WORD)) != EXECUTE_OK) {
    return (status);
  }

  // Push the Opcode Word of instruction in Error
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 2, LONG);
  if ((status = Poke(register_value[SSP_INDEX], opcode, WORD)) != EXECUTE_OK) {
    return (status);
  }

  // NOTE: The following two values are not correct!!! The correct values
  //       could be derived by adding a few variables to the CPU, but
  //       I don't have time to do it now :-)
  //
  // Push Memory Address at the Fault
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 4, LONG);
  if ((status = Poke(register_value[SSP_INDEX], 0, LONG)) != EXECUTE_OK) {
    return (status);
  }

  // Push status information word
  SetRegister(SSP_INDEX, register_value[SSP_INDEX] - 2, LONG);
  if ((status = Poke(register_value[SSP_INDEX], 9, WORD)) != EXECUTE_OK) {
    return (status);
  }

  // Get the service routine's address
  unsigned int service_address;
  if ((status = Peek(register_value[VBR_INDEX] + 0x08, service_address,
                     LONG)) != EXECUTE_OK)
    return (status);

  // Change the program counter to the service routine's address
  SetRegister(PC_INDEX, service_address, LONG);

  if (trace)
    trace_record += "{Mnemonic {Bus Error Exception}} ";

  return (EXECUTE_OK);
}
