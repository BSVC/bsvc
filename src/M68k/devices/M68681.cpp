//
// The Motorola 68681 DUART
//

#include <iostream>
#include <sstream>
#include <string>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "Framework/BasicCPU.hpp"
#include "M68k/devices/M68681.hpp"

/*
#include <stdio.h>
#include <stdarg.h>
namespace {
void log(const char *fmt, ...)
{
  FILE *fp = fopen("/tmp/log.txt", "a");
  if (fp != NULL) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);
    fclose(fp);
  }
}
}
*/

// Callback types
#define READ_A_CALLBACK 1
#define WRITE_A_CALLBACK 2
#define READ_B_CALLBACK 3
#define WRITE_B_CALLBACK 4

// The duration of the default read events
#define DEFAULT_READ_CALLBACK_DURATION 50000

// Channel/port states
#define ACTIVE 1
#define INACTIVE 0

// Bit fields in status registers
#define RxRDY 1
#define FFULL 2
#define TxRDY 4
#define TxEMT 8

// Baudrate table (contains event duration times in micro-seconds)
long M68681::baudrate_table[32] = {
    160000, // 50     baudrate (i.e. (8*1000000)/50) uS)
    72727,  // 110
    59479,  // 134.5
    40000,  // 200
    26666,  // 300
    13333,  // 600
    6666,   // 1200
    7619,   // 1050
    3333,   // 2400
    1666,   // 4800
    1111,   // 7200
    833,    // 9600
    208,    // 38400
    1666,   // 4800   (Should be timer but that's not implemented)
    1666,   // 4800   (Should be IP4-16X but that's not implemented)
    1666,   // 4800   (Should be IP4-1X but that's not implemented)
    106666, // 75
    72727,  // 110
    59479,  // 134.5
    53333,  // 150
    26666,  // 300
    13333,  // 600
    6666,   // 1200
    4000,   // 2000
    3333,   // 2400
    1666,   // 4800
    4444,   // 1800
    833,    // 9600
    416,    // 19200
    1666,   // 4800   (Should be timer but that's not implemented)
    1666,   // 4800   (Should be IP4-16X but that's not implemented)
    1666    // 4800   (Should be IP4-1X but that's not implemented)
};

// The Constructor
M68681::M68681(const std::string &args, BasicCPU &cpu)
    : BasicDevice("M68681", args, cpu) {
  std::istringstream in(args);
  std::string keyword, equals;
  int portAStdIOFlag;
  int portBStdIOFlag;

  // Scan "BaseAddress = nnnn"
  in >> keyword >> equals >> std::hex >> base_address;
  if ((!in) || (keyword != "BaseAddress") || (equals != "=")) {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }
  base_address *= cpu.Granularity();

  // Scan "OffsetToFirstRegister = [0-8]"
  in >> keyword >> equals >> std::hex >> offset_to_first_register;
  if ((!in) || (keyword != "OffsetToFirstRegister") || (equals != "=")) {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }
  offset_to_first_register *= cpu.Granularity();

  // Scan "OffsetBetweenRegisters = [1-8]"
  in >> keyword >> equals >> std::hex >> offset_between_registers;
  if ((!in) || (keyword != "OffsetBetweenRegisters") || (equals != "=")) {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }
  offset_between_registers *= cpu.Granularity();

  // Scan "InterruptLevel = [1-7]"
  in >> keyword >> equals >> std::hex >> interrupt_level;
  if ((!in) || (keyword != "InterruptLevel") || (equals != "=")) {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }

  // Scan "PortAStandardInputOutputFlag = 0|1"
  in >> keyword >> equals >> portAStdIOFlag;
  if ((!in) || (keyword != "PortAStandardInputOutputFlag") || (equals != "=")) {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }

  // Scan "PortBStandardInputOutputFlag = 0|1"
  in >> keyword >> equals >> portBStdIOFlag;
  if ((!in) || (keyword != "PortBStandardInputOutputFlag") || (equals != "=")) {
    ErrorMessage("Invalid initialization arguments!");
    return;
  }

  int loc;
  loc = args.find("PortACommand = ") + 15;
  std::string portACommand(args, loc, args.find(" PortBCommand = ") - loc);

  loc = args.find("PortBCommand = ") + 15;
  std::string portBCommand(args, loc, args.size() - loc);

  // Default Invalid values for everything
  coma_pid = coma_read_id = coma_write_id = -1;
  comb_pid = comb_read_id = comb_write_id = -1;

  // Startup the process for port a
  if (portACommand != "") {
    if (!StartPortCommand(portACommand, portAStdIOFlag, coma_read_id,
                          coma_write_id, coma_pid)) {
      ErrorMessage("Problem starting port a's process!");
      return;
    }
  }

  // Startup the process for port b
  if (portBCommand != "") {
    if (!StartPortCommand(portBCommand, portBStdIOFlag, comb_read_id,
                          comb_write_id, comb_pid)) {
      ErrorMessage("Problem starting port b's process!");
      return;
    }
  }

  // Reset the DUART to its startup state
  Reset();

  if (coma_read_id != -1) {
    // Schedule an event to start checking the A pipe
    (cpu.eventHandler())
        .Add(this, READ_A_CALLBACK, 0, DEFAULT_READ_CALLBACK_DURATION);
  }

  if (comb_read_id != -1) {
    // Schedule an event to start checking the B pipe
    (cpu.eventHandler())
        .Add(this, READ_B_CALLBACK, 0, DEFAULT_READ_CALLBACK_DURATION);
  }
}

// Startup a process
bool M68681::StartPortCommand(const std::string &command, bool std_flag,
                              int &read_pipe_id, int &write_pipe_id,
                              pid_t &pid) {
  int read_pipe_ids[2];
  int write_pipe_ids[2];

  // Create the write pipe
  if (pipe(write_pipe_ids))
    return false;

  // Create the read pipe
  if (pipe(read_pipe_ids))
    return false;

  // Set the NONBLOCK flag for the reading pipe
  if (fcntl(read_pipe_ids[0], F_SETFL, O_NONBLOCK) == -1)
    return false;

  // Set the close on exec bits
  if (fcntl(write_pipe_ids[1], F_SETFD, 1) == -1)
    return false;
  if (fcntl(read_pipe_ids[0], F_SETFD, 1) == -1)
    return false;

  if ((pid = fork()) == 0) {
    // See if the pipes should be connected to STDIN and STDOUT
    if (std_flag) {
      // Close STDIN and put the pipe in its place
      if (dup2(write_pipe_ids[0], 0) == -1)
        close(write_pipe_ids[0]);

      // Close STDOUT and put the pipe in its place
      if (dup2(read_pipe_ids[1], 1) == -1)
        close(read_pipe_ids[1]);

      // Dup the STDOUT to STDERR
      dup2(1, 2);
    } else {
      // If pipe isn't connected to STDIN and STDOUT then we will
      // close STDIN, STDOUT, and STDERR and put the read pipe
      // at descriptor 3 and the write pipe at descriptor 4.

      // First dup down to stdin and stdout
      dup2(write_pipe_ids[0], 0);
      close(write_pipe_ids[0]);
      dup2(read_pipe_ids[1], 1);
      close(read_pipe_ids[1]);

      // Now dup read pipe to 3 and write pipe to 4
      dup2(0, 3);
      dup2(1, 4);

      // Close STDIN, STDOUT, STDERR
      close(0);
      close(1);
      close(2);
    }

    std::ostringstream out;
    out << "exec " << command << std::flush;

    if (execl("/bin/sh", "sh", "-c", out.str().c_str(), NULL))
      exit(0);

    return false;
  } else {
    close(write_pipe_ids[0]);
    close(read_pipe_ids[1]);
    read_pipe_id = read_pipe_ids[0];
    write_pipe_id = write_pipe_ids[1];

    return true;
  }
}

M68681::~M68681() {
  // We need to destory the Command Process
  if (coma_pid != -1) {
    close(coma_read_id);
    close(coma_write_id);
    kill(coma_pid, SIGKILL);
  }

  // We need to destory the Command Process
  if (comb_pid != -1) {
    close(comb_read_id);
    close(comb_write_id);
    kill(comb_pid, SIGKILL);
  }
}

// Perform a reset on the DUART
void M68681::Reset() {
  BasicDevice::Reset(); // Do the BasicDevice reset

  IVR = 0x0f;                  // 68000's Uninitialized interrupt vector
  ISR = 0;                     // Clear the interrupt status register
  IMR = 0;                     // Clear the interrupt mask register
  SRA = 0;                     // Clear status register A
  SRB = 0;                     // Clear status register B
  receiver_a_state = INACTIVE; // Set the channels to inactive state
  transmitter_a_state = INACTIVE;
  receiver_b_state = INACTIVE;
  transmitter_b_state = INACTIVE;

  mr1a_pointer = 1; // The mr?a address points to MR1A
  mr1b_pointer = 1; // The mr?b address points to MR1B
}

// Check to see if the address maps into this device
bool M68681::CheckMapped(Address address) const {
  return (address >= base_address) &&
         (address <= base_address + offset_to_first_register +
                         15 * offset_between_registers);
}

// Read one of the registers in the DUART
Byte M68681::Peek(Address addr) {
  addr -= (base_address + offset_to_first_register);

  if (addr == 0) { // Mode Register A
    if (mr1a_pointer) {
      mr1a_pointer = 0;
      return MR1A;
    } else {
      return MR2A;
    }
  } else if (addr == 1 * offset_between_registers) {   // Status Register A
    return SRA;
  } else if (addr == 3 * offset_between_registers) {   // Receive Buffer A
    SRA &= ~RxRDY;
    SRA &= ~FFULL;
    SetInterruptStatusRegister();
    return RBA;
  } else if (addr == 4 * offset_between_registers) {   // Input Port Change Register
    return IPCR;
  } else if (addr == 5 * offset_between_registers) {   // Interrupt Status Register
    return ISR;
  } else if (addr == 6 * offset_between_registers) {   // Current MSB of Counter
    return CUR;
  } else if (addr == 7 * offset_between_registers) {   // Current LSB of Counter
    return CLR;
  } else if (addr == 8 * offset_between_registers) {   // Mode Register B
    if (mr1b_pointer) {
      mr1b_pointer = 0;
      return MR1B;
    }
    return MR2B;
  } else if (addr == 9 * offset_between_registers) {   // Status Register B
    return SRB;
  } else if (addr == 11 * offset_between_registers) {  // Receive Buffer B
    SRB &= ~RxRDY;
    SRB &= ~FFULL;
    SetInterruptStatusRegister();
    return RBB;
  } else if (addr == 12 * offset_between_registers) {  // Interrupt-vector register
    return IVR;
  }
  return 0;
}

// Write to one of the registers in the DUART
void M68681::Poke(Address addr, Byte c) {
  addr -= (base_address + offset_to_first_register);
  if (addr == 0) {  // Mode Register A
    if (mr1a_pointer) {
      mr1a_pointer = 0;
      MR1A = c;
      SetInterruptStatusRegister();
    } else {
      MR2A = c;
    }
  } else if (addr == offset_between_registers) {  // Clock-select Register A
    CSRA = c;
  } else if (addr == 2 * offset_between_registers) { // Command Register A
    CRA = c;
    switch ((CRA & 0x70) >> 4) {
    case 1: // Reset mode register pointer
      mr1a_pointer = 1;
      break;
    case 2: // Reset receiver
      receiver_a_state = INACTIVE;
      SRA &= ~RxRDY;
      SRA &= ~FFULL;
      SetInterruptStatusRegister();
      break;
    case 3: // Reset transmitter
      transmitter_a_state = INACTIVE;
      SRA &= ~TxRDY;
      SRA &= ~TxEMT;
      SetInterruptStatusRegister();
      break;
    case 4: // Clear error status
      SRA &= 0x0f;
      break;
    }
    switch ((CRA & 0x0c) >> 2) {
    case 1: // Enable transmitter
      transmitter_a_state = ACTIVE;
      SRA |= TxRDY;
      SRA |= TxEMT;
      SetInterruptStatusRegister();
      break;
    case 2: // Disable transmitter
      transmitter_a_state = INACTIVE;
      SRA &= ~TxRDY;
      SRA &= ~TxEMT;
      SetInterruptStatusRegister();
      break;
    }
    switch (c & 0x3) {
    case 1: // Enable receiver
      receiver_a_state = ACTIVE;
      SRA &= ~RxRDY;
      SRA &= ~FFULL;
      SetInterruptStatusRegister();
      break;
    case 2:
      receiver_a_state = INACTIVE;
      break;
    }
  } else if (addr == 3 * offset_between_registers) {  // Transmitter Buffer A
    TBA = c;       // Store the transmit data
    SRA &= ~TxRDY; // Mark register as full
    SRA &= ~TxEMT;
    SetInterruptStatusRegister();

    if (ACR & 128)
      myCPU.eventHandler()
          .Add(this, WRITE_A_CALLBACK, 0, baudrate_table[(CSRA & 0xf) + 16]);
    else
      myCPU.eventHandler()
          .Add(this, WRITE_A_CALLBACK, 0, baudrate_table[(CSRA & 0xf)]);
  } else if (addr == 5 * offset_between_registers) {  // Interrupt Mask Register
    IMR = c;
  } else if (addr == 6 * offset_between_registers) {  // Counter/Timer Upper Reg
    CTUR = c;
  } else if (addr == 7 * offset_between_registers) {  // Counter/Timer Lower Reg
    CTLR = c;
  } else if (addr == 8 * offset_between_registers) {  // Mode Register B
    if (mr1b_pointer) {
      mr1b_pointer = 0;
      MR1B = c;
      SetInterruptStatusRegister();
    } else {
      MR2B = c;
    }
  } else if (addr == 9 * offset_between_registers) {  // Clock-select Register B
    CSRB = c;
  } else if (addr == 10 * offset_between_registers) { // Command Register B
    CRB = c;
    switch ((CRB & 0x70) >> 4) {
    case 1: // Reset mode register pointer
      mr1b_pointer = 1;
      break;
    case 2: // Reset receiver
      receiver_b_state = INACTIVE;
      SRB &= ~RxRDY;
      SRB &= ~FFULL;
      SetInterruptStatusRegister();
      break;
    case 3: // Reset transmitter
      transmitter_b_state = INACTIVE;
      SRB &= ~TxRDY;
      SRB &= ~TxEMT;
      SetInterruptStatusRegister();
      break;
    case 4: // Clear error status
      SRB &= 0x0f;
      break;
    }
    switch ((CRB & 0x0c) >> 2) {
    case 1: // Enable transmitter
      transmitter_b_state = ACTIVE;
      SRB |= TxRDY;
      SRB |= TxEMT;
      SetInterruptStatusRegister();
      break;
    case 2: // Disable transmitter
      transmitter_b_state = INACTIVE;
      SRB &= ~TxRDY;
      SRB &= ~TxEMT;
      SetInterruptStatusRegister();
      break;
    }
    switch (c & 0x3) {
    case 1: // Enable receiver
      receiver_b_state = ACTIVE;
      SRB &= ~RxRDY;
      SRB &= ~FFULL;
      SetInterruptStatusRegister();
      break;
    case 2:
      receiver_b_state = INACTIVE;
      break;
    }
  } else if (addr == 11 * offset_between_registers) // Transmitter Buffer B
  {
    TBB = c;       // Store the transmit data
    SRB &= ~TxRDY; // Mark register as full
    SRB &= ~TxEMT;
    SetInterruptStatusRegister();

    if (ACR & 128)
      (myCPU.eventHandler())
          .Add(this, WRITE_B_CALLBACK, 0, baudrate_table[(CSRB & 0xf) + 16]);
    else
      (myCPU.eventHandler())
          .Add(this, WRITE_B_CALLBACK, 0, baudrate_table[(CSRB & 0xf)]);
  } else if (addr == 12 * offset_between_registers) // Interrupt-vector register
  {
    IVR = c;
  }
}

// Handle event callbacks
void M68681::EventCallback(int type, void *) {
  SetInterruptStatusRegister();

  if (type == WRITE_A_CALLBACK) {
    Byte c;

    // If pipe is not availiable then just pretend we sent it somewhere
    if (coma_write_id == -1) {
      SRA |= TxRDY; // Ready for more data
      SRA |= TxEMT; // Transmitter is empty
      SetInterruptStatusRegister();
      return;
    } else if (transmitter_a_state == INACTIVE) {
      return;
    }

    // Mask off bits that shouldn't be transmitted
    switch (MR1A & 3) {
    case 0:
      c = TBA & 0x1f;
      break;
    case 1:
      c = TBA & 0x3f;
      break;
    case 2:
      c = TBA & 0x7f;
      break;

    default:
      c = TBA;
    }

    // Do the data transfer
    switch ((MR2A & 0xc0) >> 6) {
    case 1: // Automatic-echo mode (transmitter link disabled)
      SRA |= TxRDY;
      SRA |= TxEMT;
      break;

    default: // Normal mode
      if (write(coma_write_id, &c, 1) != 1) {
        exit(1);
      }
      SRA |= TxRDY; // Ready for more data
      SRA |= TxEMT; // Transmitter is empty
      SetInterruptStatusRegister();
    }
  } else if (type == READ_A_CALLBACK) {
    Byte c;

    // If receiver is disabled or full then just add event and return
    if ((receiver_a_state == INACTIVE) || (SRA & FFULL)) {
      // Reschedule another read callback to check for more characters
      myCPU.eventHandler()
          .Add(this, READ_A_CALLBACK, 0, DEFAULT_READ_CALLBACK_DURATION);
      return;
    }

    // Try to read a byte from the pipe
    if (read(coma_read_id, &c, 1) == 1) {
      // Mask off bits that shouldn't be received
      switch (MR1A & 3) {
      case 0:
        RBA = c & 0x1f;
        break;
      case 1:
        RBA = c & 0x3f;
        break;
      case 2:
        RBA = c & 0x7f;
        break;

      default:
        RBA = c;
      }

      // Handle any special stuff for the funky modes
      switch ((MR2A & 0xc0) >> 6) {
      case 1: // Automatic-echo mode
        if (write(coma_write_id, &RBA, 1) != 1) {
          exit(1);
        }
        break;
      }

      // If we're already full then set the overrun error bit
      //      if(SRA & FFULL)
      //        SRA |= 16;

      SRA |= RxRDY;
      SRA |= FFULL;
      SetInterruptStatusRegister();

      // Reschedule another read callback to check for more characters
      if (ACR & 128)
        myCPU.eventHandler()
            .Add(this, READ_A_CALLBACK, 0, baudrate_table[(CSRA >> 4) + 16]);
      else
        myCPU.eventHandler()
            .Add(this, READ_A_CALLBACK, 0, baudrate_table[(CSRA >> 4)]);
    } else {
      // Reschedule another read callback to check for more characters
      myCPU.eventHandler()
          .Add(this, READ_A_CALLBACK, 0, DEFAULT_READ_CALLBACK_DURATION);
    }
  } else if (type == WRITE_B_CALLBACK) {
    Byte c;

    // If pipe is not availiable then just pertend we sent it somewhere
    if (comb_write_id == -1) {
      SRB |= TxRDY;
      SRB |= TxEMT;
      SetInterruptStatusRegister();
      return;
    } else if (transmitter_b_state == INACTIVE) {
      return;
    }

    // Mask off bits that shouldn't be transmitted
    switch (MR1B & 3) {
    case 0:
      c = TBB & 0x1f;
      break;
    case 1:
      c = TBB & 0x3f;
      break;
    case 2:
      c = TBB & 0x7f;
      break;

    default:
      c = TBB;
    }

    // Do the data transfer
    switch ((MR2B & 0xc0) >> 6) {
    case 1: // Automatic-echo mode (transmitter link disabled)
      SRB |= TxRDY;
      SRB |= TxEMT;
      break;
    case 2: // Local-loopback mode (not implemented)
    case 3: // Multidrop mode (not implemented)

    default: // Normal mode
      if (write(comb_write_id, &c, 1) != 1) {
        exit(1);
      }
      SRB |= TxRDY;
      SRB |= TxEMT;
      SetInterruptStatusRegister();
    }
  } else if (type == READ_B_CALLBACK) {
    Byte c;

    // If receiver is disabled or full then just add event and return
    if ((receiver_b_state == INACTIVE) || (SRB & FFULL)) {
      if (SRB & FFULL) {
        if (ACR & 128)
          (myCPU.eventHandler())
              .Add(this, READ_B_CALLBACK, 0, baudrate_table[(CSRB >> 4) + 16]);
        else
          (myCPU.eventHandler())
              .Add(this, READ_B_CALLBACK, 0, baudrate_table[(CSRB >> 4)]);
      } else {
        // Reschedule another read callback to check for more characters
        (myCPU.eventHandler())
            .Add(this, READ_B_CALLBACK, 0, DEFAULT_READ_CALLBACK_DURATION);
      }
      return;
    }

    // Try to read a byte from the pipe
    if (read(comb_read_id, &c, 1) == 1) {
      // Mask off bits that shouldn't be received
      switch (MR1B & 3) {
      case 0:
        RBB = c & 0x1f;
        break;
      case 1:
        RBB = c & 0x3f;
        break;
      case 2:
        RBB = c & 0x7f;
        break;

      default:
        RBB = c;
      }

      // Handle any special stuff for the funky modes
      switch ((MR2B & 0xc0) >> 6) {
      case 1: // Automatic-echo mode (transmitter link disabled)
        if (write(comb_write_id, &RBB, 1) != 1) {
          exit(1);
        }
        break;
      }

      // If we're already full then set the overrun error bit
      //      if(SRB & FFULL)
      //        SRB |= 16;

      SRB |= RxRDY;
      SRB |= FFULL;
      SetInterruptStatusRegister();

      // Reschedule another read callback to check for more characters
      if (ACR & 128)
        (myCPU.eventHandler())
            .Add(this, READ_B_CALLBACK, 0, baudrate_table[(CSRB >> 4) + 16]);
      else
        (myCPU.eventHandler())
            .Add(this, READ_B_CALLBACK, 0, baudrate_table[(CSRB >> 4)]);
    } else {
      // Reschedule another read callback to check for more characters
      (myCPU.eventHandler())
          .Add(this, READ_B_CALLBACK, 0, DEFAULT_READ_CALLBACK_DURATION);
    }
  }
}

// Set the Interrupt Status Register (ISR) and request an interrupt if needed
void M68681::SetInterruptStatusRegister() {
  ISR = 0;

  // See if transmitter A is ready for data
  if (SRA & TxRDY)
    ISR |= 1;

  // See if receiver A has data or if the FIFO is full
  if (MR1A & 64) {
    if (SRA & FFULL)
      ISR |= 2;
  } else {
    if (SRA & RxRDY)
      ISR |= 2;
  }

  // See if transmitter B is ready for data
  if (SRB & TxRDY)
    ISR |= 16;

  // See if receiver B has data or if the FIFO is full
  if (MR1B & 64) {
    if (SRB & FFULL)
      ISR |= 32;
  } else {
    if (SRB & RxRDY)
      ISR |= 32;
  }

  // Request an interrupt if one is needed
  if (ISR & IMR)
    InterruptRequest(interrupt_level);
}

// Acknowledege Interrupt - Called by CPU when it processes the interrupt
int M68681::InterruptAcknowledge(int) {
  if (!myInterruptPending) {
    return SPURIOUS_INTERRUPT;
  }
  myInterruptPending = false; // Clear the pending interrupt flag
  return IVR;                 // Return the programmed Interrupt Vector
}
