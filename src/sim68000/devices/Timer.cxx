///////////////////////////////////////////////////////////////////////////////
//
// Timer.cxx
//
//   This class keeps up with a list of all of the availible devices and
// allocates them.  It's dervied from the BasicDeviceRegistry
//
// FILENAME: Timer.cxx
// DESC: Simulates an timer for the M68000 CPU. Thus giving 
//       the Simulator the ablility to simulate a timmer intrurpt.
//       Programmers can impliment time slicing with this timmer.
//
//       Contains Register and Offsets for the timer. 
//
// CREATE:  7-17-98
// OWNER:   Xavier Plasencia
// ORG:	    SDSU
// DEPEN:   This file requiers the Event hander and the BasicDevice files.  
//
///////////////////////////////////////////////////////////////////////////////
// $Id: $
///////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
  #include <iostream>
  #include <strstream>
  #include <cstring>
#else
  #include <iostream.h>
  #include <strstream.h>
  #include <string.h>
#endif

#ifdef USE_STD
  using namespace std;
#endif

#include "BasicCPU.hxx"
#include "Timer.hxx"

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
Timer::Timer(const string& args, BasicCPU& cpu)
	: BasicDevice("Timer", args, cpu)
{
  istrstream in((char*)args.data(), args.size());
  string keyword, equals;
  unsigned long base;
  unsigned long irq;

  firstTime = true;
  // Scan "BaseAddress = nnnn"
  // This is important for the CPU to know what address space
  // is used by the device.
  in >> keyword >> equals >> hex >> base;
  if((!in) || (keyword != "BaseAddress") || (equals != "="))
  {
     ErrorMessage("Invalid initialization arguments!");
     return;
  }

  in >> keyword >> equals >> hex >> irq;
  if((!in) || (keyword != "IRQ") || (equals != "="))
  {
     ErrorMessage("Invalid initialization arguments!");
     return;
  }

/*
  I've hardcoded the cycles setting to 2000 which should provide the
  125000 ticks per second for the PI/T

  unsigned long timer_cycle;
  in >> keyword >> equals >> hex >> timer_cycle;
  if((!in) || (keyword != "CycleSec") || (equals != "="))
  {
     ErrorMessage("Invalid initialization arguments!");
     return;
  }
  Timer::TIMER_CPU_CYCLE = timer_cycle;
*/

  Timer::TIMER_IRQ = irq;
  Timer::TIMER_CPU_CYCLE = 2000;
  baseAddress = base * cpu.Granularity();

  // This is needed to start the whole process.  The eventhandler
  // will always request an event and test to see if the user has
  // enabled the timer it not nothing happens but if so. The
  // CPR  register is copied to the CNTR and is decremented for every
  // Cycle.
  (myCPU.eventHandler()).Add(this, TIMER_EVENT, 0, TIMER_CPU_CYCLE);

  Reset(); 
}

///////////////////////////////////////////////////////////////////////////////
// This routine should set myInterruptPending flag and send a request
// to the CPU for an interrupt.
///////////////////////////////////////////////////////////////////////////////
void Timer::InterruptRequest(int level)
{
  // If no interrupt is pending then request one
  if(!myInterruptPending)
  {
    myInterruptPending = true;
    myCPU.InterruptRequest(this, level);
  }
}
       
/*___________________________________________________________________________
 | FUNCTIONAME: InterruptAcknowledge(int)
 | DESC: This routine is called by the CPU when it processes a 
 |  requested interrupt. It should return the vector number associated with 
 |  the interrupt or AUTOVECTOR_INTERRUPT if the device doesn't generate 
 |	vectors.  This default routine only does autovector interrupts.
 | 	This function is called by the CPU.  The return value is usally the
 | 	memory location of a exemption funcion.  How to handel these functions
 |	one shold refer to a Motorla text book.
 | PARAM: None
 | NOTE: Vecotor codes 5 and 7 are the only masks that will make the timer 
 | do somthing.  5 is a VECTOR and 7 is an AUTOVECTOR.  The code
 | can be found in any manual.
 |___________________________________________________________________________
*/
long Timer::InterruptAcknowledge(int)
{
  unsigned char cTCR = timerValue[TCR];

  if(myInterruptPending)
  {
    switch(cTCR>>5)
    {
      case 5:
        //By returning the address of the vector, the cpu will
        //call the vector that is located in the right mem location.
        myInterruptPending = false;
        return timerValue[TIVR];
        break; 

      case 7:
        myInterruptPending = false;
        return AUTOVECTOR_INTERRUPT;
        break;

      default:
        return SPURIOUS_INTERRUPT;
    }
  }
  else
  {
    return(SPURIOUS_INTERRUPT);
  }
}

/*______________________________________________________________
 | METHOD: Timer::CheckMapped( unsigned long adress)
 | DESC: Check to see if address mapps to device. 
 |______________________________________________________________
*/
bool Timer::CheckMapped(unsigned long address) const
{
  return ((address >= baseAddress ) && 
      (address <= baseAddress+(22*sizeof(char))));
}

/*______________________________________________________________
 | METHOD: Timer::Peek(unsigned long address)
 | DESC:  Returns a byte from the device's address.
 |
 |	PEEK is the protocal that is used by the device to 
 |	inform the CPU what the registry settings are.
 |  The use need not update the memory location for the CPU
 |	All of this is done by the cpu and thus uses peek to
 |	read the data that the device is using for the  Memory
 |  (i.e. RAM)
 |______________________________________________________________
*/
unsigned char Timer::Peek(unsigned long address)
{
  switch((address-baseAddress))
  {
    case TCR:
      return timerValue[TCR];
    case TIVR:
      return timerValue[TIVR];
    case NULR:
      return timerValue[NULR];
    case CPRH:
      return timerValue[CPRH];
    case CPRM:
      return timerValue[CPRM];
    case CPRL:
      return timerValue[CPRL];
    case CNTRH:
      return timerValue[CNTRH];
    case CNTRM:
      return timerValue[CNTRM];
    case CNTRL:
      return timerValue[CNTRL];
    case TSR:
      return timerValue[TSR];
   }
   return 0; //Error not a valid address.
}

/*______________________________________________________________
 | METHOD: Timer::Poke(unsigned long address, char c)
 | DESC:  This is the interface to the rest of the World.  Here
 |	the CPU or anyother devece writes to memory and updates the
 |  device.  Poke Changes the value of the registers.  
 |  Notice that there is no way for the user to WRITE to the 
 |  registers CNTR, this is because they are read only.
 |______________________________________________________________
*/
void Timer::Poke(unsigned long address, unsigned char c)
{
  switch((address-baseAddress))
  {
    case TCR:
      timerValue[TCR]=c;
      break;
    case TIVR:
      timerValue[TIVR]=c;
      break;
    case NULR:
      timerValue[NULR]=c;
      break;
    case CPRH:
      timerValue[CPRH]=c;
      break;
    case CPRM:
      timerValue[CPRM]=c;
      break;
    case CPRL:
      timerValue[CPRL]=c;
      break;
    case TSR:
      timerValue[TSR]=c;
      break;
  }
}

/*______________________________________________________________
 | METHOD:  Timer::Reset()
 | DESC:	Resets the timer. And Initalizes the Registers.
 |______________________________________________________________
*/
void Timer::Reset()
{
  // Resets the interupt handler.
  myInterruptPending = false;
  firstTime = true;

  // Clears the timers.
  for(int i=0; i <22; i++)
    timerValue[i]=0;
}

/*______________________________________________________________
 | METHOD:	VetcorTimer::EventCallback(long *data, void *ptr)
 | DESC:	Function that executes when interupt occures.
 | DEP FUNC: Requires decCNTR copyCPRtoCNTR.
 |______________________________________________________________
*/
void Timer::EventCallback(long data, void *ptr)
{
  unsigned char cTCR = timerValue[TCR];

  // Test to see if timer is enabled, if False(0) do nothing.
  // but dispatch an event.
  if((cTCR&1) == 1)
  {
    // This test to see if the timer has not started a 
    // count down.  If it hasnt, copy the CPR register vaules
    // to the CNTR registers.
    if(firstTime == true)
    {
      firstTime = false;
      copyCPRtoCNTR();
    }

    // Decriments the CNTR if 0 then call interrupt.
    // else dispatch an event.
    if(decCNTR() != 0)
    {
      (myCPU.eventHandler()).Add(this, TIMER_EVENT, 0, TIMER_CPU_CYCLE);
      return;
    }

    // Since the timer is now done, set the first time to ture,
    // so the next time the TCR is set to 1 it will copy the
    // CPR Register to CNTR.
    firstTime = true;

    // Protocal says that the TCR should flip to 0. and that
    // TSR bit switches to 1. That is done below.
    timerValue[TCR] = (timerValue[TCR] & 0xfe);
    timerValue[TSR] = (timerValue[TSR] | 0x01);

    // Dispatch an IRQ only if user has set this in the
    // TCR register. (5 and 7) are the only valid
    // Interupts.
    if (((cTCR>>5) & 5) || ((cTCR>>5) &7))
    {
      InterruptRequest(TIMER_IRQ);
    }
    (myCPU.eventHandler()).Add(this, TIMER_EVENT, 0, TIMER_CPU_CYCLE);
  }
  else
  {
    (myCPU.eventHandler()).Add(this, TIMER_EVENT, 0, TIMER_CPU_CYCLE);
  }
}

/*______________________________________________________________
 | METHOD:  CopyCPRtoCNTR
 | DESC:	Copies the register value of CPR to CNTR.
 | USED:	By funce  EventCallback
 | DEP FUNC: None
 |______________________________________________________________
*/
void Timer::copyCPRtoCNTR()
{
  timerValue[CNTRH] = timerValue[CPRH];
  timerValue[CNTRM] = timerValue[CPRM];
  timerValue[CNTRL] = timerValue[CPRL];
}

/*______________________________________________________________
 | METHOD: decCNTR
 | DESC: decriments the counter and checks for CNTR==0
 |		retuns the value of CNTR.
 |______________________________________________________________
*/
unsigned int Timer::decCNTR()
{
  int decValue;
  int temp24;

  // Grab the values of the 24bit register to a value.
  decValue = 0;
  temp24 = (unsigned int) timerValue[CNTRH];
  decValue = (decValue | ( temp24 <<  24));

  temp24 = (unsigned int) timerValue[CNTRM];
  decValue = (decValue | (temp24 << 16));

  decValue = (decValue | timerValue[CNTRL]);

  decValue -= 250;
  if(decValue < 0)
  {
    decValue = 0;
  }

  // Put the value  back  the 24bit register to a value.
  // with the decremnt.
  timerValue[CNTRH] = ( unsigned char) (decValue >>  24);
  timerValue[CNTRM] = ( unsigned char) (decValue >> 16);
  timerValue[CNTRL] = ( unsigned char) (decValue );

  return (unsigned int) decValue;
}

