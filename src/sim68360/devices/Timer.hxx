///////////////////////////////////////////////////////////////////////////////
//
// Timer.hxx
//
// FILENAME: Timer.hxx
// DESC: Simulates an ECB timer for the M68000 CPU. Thus giving 
//       the Simulator the ablility to simulate a timmer intrurpt.
//       Programmers can impliment time slicing with this timmer.
//
//       Contains Register and Offsets for the ECB timer. 
//
// CREATE:  7-17-98
// OWNER:   Xavier Plasencia
// ORG:	    SDSU
// DEPEN:   This file requiers the Event hander and the BasicDevice files.  
//
///////////////////////////////////////////////////////////////////////////////
// $Id: $
///////////////////////////////////////////////////////////////////////////////

#ifndef TIMER_HXX 
#define TIMER_HXX  

#include <string>

#ifdef USE_STD
  using namespace std;
#endif

#include "BasicDevice.hxx"

#define AUTOVECTOR_INTERRUPT  -1
#define SPURIOUS_INTERRUPT    -2

/*________________________
 |Contants
 |________________________
*/
 #define TCR   0
 #define TIVR  2
 #define NULR  4
 #define CPRH  6
 #define CPRM  8
 #define CPRL  10 //0xA  in Hex
 #define NULTR 12 //0xC  in hex
 #define CNTRH 14
 #define CNTRM 16
 #define CNTRL 18
 #define TSR   20 //0x14 in hex

 #define TIMER_EVENT			100

class Timer : public BasicDevice 
{
  public:
    // Constructor
    Timer( const string& args, BasicCPU& cpu);

    // Destructor
    ~Timer(){};

    // Answers true iff the address maps into the device
    bool CheckMapped(unsigned long address) const ;

    // Answers the lowest address used by the device
    unsigned long LowestAddress() const {return baseAddress;}

    // Answers the highest address used by the device
    unsigned long HighestAddress() const  
    { return (baseAddress + (15*sizeof(char)));}

    // Get a byte from the device
    unsigned char Peek(unsigned long address);

    // Put a byte into the device
    void Poke(unsigned long address, unsigned char c);

    // Reset the device
    void Reset();

    // This routine sends an interrupt request (IRQ) to the CPU
    void InterruptRequest(int level);
       
    // This routine is called by the CPU when it processes an interrupt
    long InterruptAcknowledge(int level);

    void EventCallback(long data, void *ptr);

    // This is Device specific function that copyies CPR reg to CNTR.
    void copyCPRtoCNTR();

    // Device specific func: Decriments the CNTR and checks for 0.
    unsigned int decCNTR();

  protected:
    // Interrupt pending flag
    bool myInterruptPending;

    // Base location
    unsigned long baseAddress;

    // IRQ level set by the setup file.
    unsigned long TIMER_IRQ;

    // Time between event calls.
    unsigned long TIMER_CPU_CYCLE;

    // First time the caller is set.
    bool firstTime;

    // Timer value.
    unsigned char timerValue[22];
};
#endif

