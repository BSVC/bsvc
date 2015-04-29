///////////////////////////////////////////////////////////////////////////////
//
// M68681.hxx
//
// The Motorola 68681 DUART
//
// Sim68000 "Motorola 68000 Simulator"
// Copyright (c) 1993
// By: Bradford W. Mott
// Novermber 24,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: M68681.hxx,v 1.1 1996/08/02 15:01:01 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef M68681_HXX
#define M68681_HXX

#include <string>
#include <sys/types.h>

#include "BasicDevice.hxx"

class M68681 : public BasicDevice {
  public:
    // Constructor
    M68681(const string& args, BasicCPU& cpu);

    // Destructor
    ~M68681();

    // Answers true iff the address maps into the device
    bool CheckMapped(unsigned long addr) const;

    // Answers the lowest address used by the device
    unsigned long LowestAddress() const
    { return base_address; }

    // Answers the highest address used by the device
    unsigned long HighestAddress() const
    { 
      return base_address+offset_to_first_register+15*offset_between_registers;
    }

    // Get a byte from the device
    unsigned char Peek(unsigned long addr);

    // Put a byte into the device
    void Poke(unsigned long addr, unsigned char c);

    // Reset handler
    void Reset();

    // The BasicDevice's InterruptAcknowledge does not handle vectored
    // interrupts so we'll have to change it!!!
    long InterruptAcknowledge(int level);
   
    // Handle the DUART's events
    void EventCallback(long type, void* pointer);

  private:
    unsigned char MR1A;                    // Mode register 1 A
    unsigned char MR2A;                    // Mode register 2 A
    unsigned char SRA;                     // Status register A
    unsigned char CSRA;                    // Clock-select register A
    unsigned char CRA;                     // Command register A
    unsigned char RBA;                     // Receiver buffer A
    unsigned char TBA;                     // Transmitter buffer A
    unsigned char IPCR;                    // Input port change register

    unsigned char ACR;                     // Auxiliary control register
    unsigned char ISR;                     // Interrupt status register
    unsigned char IMR;                     // Interrupt mask register
    unsigned char CUR;                     // Counter mode: counter MSB
    unsigned char CTUR;                    // Counter/timer uppper register
    unsigned char CLR;                     // Counter mode: counter LSB
    unsigned char CTLR;                    // Counter/timer lower register

    unsigned char MR1B;                    // Mode register 1 B
    unsigned char MR2B;                    // Mode register 2 B
    unsigned char SRB;                     // Status register B
    unsigned char CSRB;                    // Clock-select register B
    unsigned char CRB;                     // Command register B
    unsigned char RBB;                     // Receiver buffer B
    unsigned char TBB;                     // Transmitter buffer B

    unsigned char IVR;                     // Interrupt-vector register

    unsigned char mr1a_pointer;            // Determines MR1A/MR2A
    unsigned char mr1b_pointer;            // Determines MR1B/MR2B

    unsigned char receiver_a_state;        // State of receiver A
    unsigned char transmitter_a_state;     // State of transmitter A
    unsigned char receiver_b_state;        // State of receiver B
    unsigned char transmitter_b_state;     // State of transmitter B

    static long baudrate_table[32];        // Table of times for baud rates
   
    int coma_read_id;                      // Pipe to command for port a
    int coma_write_id;                     // Pipe to command for port a
    int comb_read_id;                      // Pipe to command for port b
    int comb_write_id;                     // Pipe to command for port b
    pid_t coma_pid;                        // Proccess ID for port a command
    pid_t comb_pid;                        // Proccess ID for port b command
 
    unsigned long base_address;             // Base address of the DUART
    unsigned long offset_to_first_register; // Offset to the first registers
    unsigned long offset_between_registers; // Offset to between registers
    unsigned long interrupt_level;          // The interrupt level sent to CPU

    bool StartPortCommand(const string& command, bool std_flag, 
        int &read, int &write, pid_t &pid);

    void SetInterruptStatusRegister();
};
#endif

