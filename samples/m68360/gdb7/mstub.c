/*
 * mstub.c adapted from m68k-stub.c in gdb distribution
 * Modified for BSVC
 * 
 * DJ, July 1996
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */

#include <libc.h>

#include "ustub.h"
#include "xstub.h"
#include "mstub.h"
#include "astub.h"


/* Forward declarations */
static void initializeRemcomErrorFrame();
static char initialized;
int   gdbFrameStack[FRAMESIZE];
Frame *lastFrame = NULL;

/* Globals */
int registers[NUMREGBYTES/4];
int superStack;

static jmp_buf remcomEnv;

#define STACKSIZE 10000
int remcomStack[STACKSIZE/sizeof(int)];
int* stackPtr = &remcomStack[STACKSIZE/sizeof(int) - 1];

/*
 * In many cases, the system will want to continue exception processing
 * when a continue command is given.  
 * oldExceptionHook is a function to invoke in this case.
 */
static ExceptionHook oldExceptionHook = NULL;

#ifdef mc68020
/* the size of the exception stack on the 68020 varies with the type of
 * exception.  The following table is the number of WORDS used
 * for each exception format.
 */
const short exceptionSize[] = { 4,4,6,4,4,4,4,4,29,10,16,46,12,4,4,4 };
#endif

#ifdef mc68332
static const short exceptionSize[] = { 4,4,6,4,4,4,4,4,4,4,4,4,16,4,4,4 };
#endif

char  remcomInBuffer[BUFMAX];
char  remcomOutBuffer[BUFMAX];
static short error;

/*=======================================================================*/
void _returnFromException(Frame *frame)
/*=======================================================================*/
{
    /* if no passed in frame, use the last one */
    if (! frame)
    {
        frame = lastFrame;
		frame->frameSize = 4;
        frame->format = 0;
        frame->fsaveHeader = -1; /* restore regs, but we dont have fsave info*/
    }

#if !defined (mc68020) && !defined (mc68332)
    /* a 68000 cannot use the internal info pushed onto a bus error
     * or address error frame when doing an RTE so don''t put this info
     * onto the stack or the stack will creep every time this happens.
     */
    frame->frameSize=3;
#endif

    /* throw away any frames in the list after this frame */
    lastFrame = frame;

    frame->sr = registers[(int) PS];
    frame->pc = registers[(int) PC];

    if (registers[(int) PS] & 0x2000)
    { 
        /* return to supervisor mode... */
        return_to_super();
    }
    else
    { /* return to user mode */
        return_to_user();
    }
}


/*=======================================================================*/
void handle_buserror(void)
/*=======================================================================*/
{
	/* a bus error has occurred, perform a longjmp
	 * to return execution and allow handling of the error 
	 */
	longjmp(remcomEnv,1);
}

/*=======================================================================*/
int computeSignal(int exceptionVector)
/*=======================================================================*/
{
	/* this function takes the 68000 exception number and attempts to
	 * translate this number into a unix compatible signal value 
	 */
	int sigval;
	switch (exceptionVector) {
	 case 2 : sigval = 10; break; /* bus error           */
	 case 3 : sigval = 10; break; /* address error       */
	 case 4 : sigval = 4;  break; /* illegal instruction */
	 case 5 : sigval = 8;  break; /* zero divide         */
	 case 6 : sigval = 8;  break; /* chk instruction     */
	 case 7 : sigval = 8;  break; /* trapv instruction   */
	 case 8 : sigval = 11; break; /* privilege violation */
	 case 9 : sigval = 5;  break; /* trace trap          */
	 case 10: sigval = 4;  break; /* line 1010 emulator  */
	 case 11: sigval = 4;  break; /* line 1111 emulator  */
		
		/* Coprocessor protocol violation.  Using a standard MMU or FPU
		 * this cannot be triggered by software.  Call it a SIGBUS.  
		 */
	 case 13: sigval = 10;  break;
		
	 case 31: sigval = 2;   break; /* interrupt           */
	 case 33: sigval = 5;   break; /* breakpoint          */
		
		/* This is a trap #8 instruction.  Apparently it is someone''s software
		 * convention for some sort of SIGFPE condition.  Whose?  How many
		 * people are being screwed by having this code the way it is?
		 * Is there a clean solution?  
		 */
	 case 40: sigval = 8;  break; /* floating point err  */
		
	 case 48: sigval = 8;  break; /* floating point err  */
	 case 49: sigval = 8;  break; /* floating point err  */
	 case 50: sigval = 8;  break; /* zero divide         */
	 case 51: sigval = 8;  break; /* underflow           */
	 case 52: sigval = 8;  break; /* operand error       */
	 case 53: sigval = 8;  break; /* overflow            */
	 case 54: sigval = 8;  break; /* NAN                 */
	 default: 
		sigval = 7;         /* "software generated" */
	}
	return (sigval);
}

/*=====================================================================*/
void process_read_memory()
/*=====================================================================*/
{	
	char *ptr;
	int addr;
	int length;
	
	if (setjmp(remcomEnv) == 0)
	{
		exceptionHandler(2,handle_buserror); 
		/* Try to read %x,%x. If success, set ptr to 0 */
		ptr = &remcomInBuffer[1];
		if (hexToInt(&ptr,&addr))
		{
			if (*(ptr++) == ',')
			{
				if (hexToInt(&ptr,&length)) 
				{
					ptr = 0;
					mem2hex((char*) addr, remcomOutBuffer, length);
				}
				
				if (ptr)
				{
					/* command was incorrect */
					strcpy(remcomOutBuffer,"E01");
				}     
			}
		}
	} 
	else 
	{
		exceptionHandler(2,_catchException);   
		strcpy(remcomOutBuffer,"E03"); /* bus error */
	}     
	
	/* restore handler for bus error */
	exceptionHandler(2,_catchException);   
}

/*=====================================================================*/
void process_step_or_continue()
/*=====================================================================*/
{
	char *ptr;
	int addr;
	int newPC;
	Frame  *frame;
	
	/* try to read optional parameter, pc unchanged if no parm */
	ptr = &remcomInBuffer[1];
	if (hexToInt(&ptr,&addr))
	  registers[ PC ] = addr;
	
	newPC = registers[PC];
	
	/* clear the trace bit */
	registers[PS] &= 0x7fff;
	
	/* set the trace bit if we''re stepping */
	if (remcomInBuffer[0] == 's') 
	  registers[PS] |= 0x8000;
	
	/*
	 * look for newPC in the linked list of exception frames.
	 * if it is found, use the old frame it.  otherwise,
	 * fake up a dummy frame in returnFromException().
	 */
	frame = lastFrame;
	while (frame)
	{
		if (frame->exceptionPC == newPC)
		{					
			break;
		}
		
		if (frame == frame->previous)
		{
			frame = (Frame*)0; /* no match found */ 
			break; 
		}
		frame = frame->previous;
	} /* end of 'while(frame)' */
	
			
	/*
	 * If we found a match for the PC AND we are not returning
	 * as a result of a breakpoint (33),
	 * trace exception (9), nmi (31), jmp to
	 * the old exception handler as if this code never ran.
	 */
	if (frame) 
	{
		if ((frame->exceptionVector != 9)  && 
			(frame->exceptionVector != 31) && 
			(frame->exceptionVector != 33))
		{ 
			/*
			 * invoke the previous handler.
			 */
			if (oldExceptionHook)
			  (*oldExceptionHook) (frame->exceptionVector);
			newPC = registers[ PC ];    /* pc may have changed  */
			if (newPC != frame->exceptionPC)
			{
				/* re-use the last frame, we''re skipping it (longjump?)*/
				frame = (Frame *) 0;
				_returnFromException( frame );  /* this is a jump */
			}
		}
	}         
	
	/* if we couldn''t find a frame, create one */
	if (frame == (Frame*)0)
	{
		frame = lastFrame -1 ;
		
		/* by using a bunch of print commands with breakpoints,
		 * it''s possible for the frame stack to creep down.  If it creeps
		 * too far, give up and reset it to the top.  Normal use should
		 * not see this happen.
		 */
		if ((unsigned int) (frame-2) < (unsigned int) &gdbFrameStack)
		{
			initializeRemcomErrorFrame();
			frame = lastFrame; 
		}
		frame->previous = lastFrame;
		lastFrame = frame;
		frame = 0;  /* null so _return... will properly initialize it */ 
	}    
	
	_returnFromException( frame ); /* this is a jump */
}

/*=======================================================================*/
void handle_exception(int exceptionVector)
/*=======================================================================*/
{
	/*
	 * This function does all command processing for interfacing to gdb.
	 */
	int    sigval;
	int    addr, length;
	char * ptr;

	/* reply to host that an exception has occurred */
	sigval = computeSignal( exceptionVector );
	
	remcomOutBuffer[0] = 'S';
	remcomOutBuffer[1] =  hexchars[sigval >> 4];
	remcomOutBuffer[2] =  hexchars[sigval % 16];
	remcomOutBuffer[3] = 0;
	
	putpacket(remcomOutBuffer); 
	
	while (1)
	{
		error = 0;
		remcomOutBuffer[0] = 0;
		getpacket(remcomInBuffer);
		switch (remcomInBuffer[0])
		{
		 case '?' :
			remcomOutBuffer[0] = 'S';
			remcomOutBuffer[1] =  hexchars[sigval >> 4];
			remcomOutBuffer[2] =  hexchars[sigval % 16];
			remcomOutBuffer[3] = 0;
			break; 
		 case 'g' : /* return the value of the CPU registers */
			mem2hex((char*) registers, remcomOutBuffer, NUMREGBYTES);
			break;
		 case 'G' : /* set the value of the CPU registers - return OK */
			hex2mem(&remcomInBuffer[1], (char*) registers, NUMREGBYTES);
			strcpy(remcomOutBuffer,"OK");
			break;			
		 case 'm' :
			/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
			process_read_memory();
			break;
		 case 'M' : 
			/* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			if (setjmp(remcomEnv) == 0)
			{
				exceptionHandler(2,handle_buserror); 
				/* Try to write '%x,%x:'. If succes, set ptr = 0. */
				ptr = &remcomInBuffer[1];
				if (hexToInt(&ptr,&addr))
				  if (*(ptr++) == ',')
					if (hexToInt(&ptr,&length))
					  if (*(ptr++) == ':')
				{
					hex2mem(ptr, (char*) addr, length);
					ptr = 0;
					strcpy(remcomOutBuffer,"OK");
				}
				if (ptr)
				{
					strcpy(remcomOutBuffer,"E02"); /* malformed write memory cmd. */
				}
			}
			else 
			{
				strcpy(remcomOutBuffer,"E03"); /* bus error */
			}     
			/* restore handler for bus error */
			exceptionHandler(2,_catchException);
			break;			
     case 'c' : 
     case 's' : 
			/* cAA..AA    Continue at address AA..AA(optional) */
			/* sAA..AA   Step one instruction from AA..AA(optional) */
			process_step_or_continue(); /* never returns! */
			remcomOutBuffer[0] = 0;
          break;			
			/* kill the program */
		 case 'k' :  /* do nothing */
			/* reply with 0 since request is not supported */
			remcomOutBuffer[0] = 0;
			break;

		 case 'P' :
			/* not currently supported */
			remcomOutBuffer[0] = 0;
			break;

		 case 'H' :
		 case 'q' :
			/* not currently supported */
			remcomOutBuffer[0] = 0;
			break;

			/* default: unknown command! */
		 default:
			/* reply with 0 when request is not supported */
			remcomOutBuffer[0] = 0;
			break;
		} /* switch */ 
		
		/* reply to the request */
		putpacket(remcomOutBuffer);
    }
	
}

/*=======================================================================*/
void initializeRemcomErrorFrame(void)
/*=======================================================================*/
{
	lastFrame = ((Frame *) &gdbFrameStack[FRAMESIZE-1]) - 1;
    lastFrame->previous = lastFrame;
}

/*=======================================================================*/
void set_debug_traps(void)
/*=======================================================================*/
{
	/* this function is used to set up exception handlers for tracing and 
	 * breakpoints 
	 */
	extern void _debug_level7();
	extern void remcomHandler();
	int exception;

	initializeRemcomErrorFrame();
	stackPtr  = &remcomStack[STACKSIZE/sizeof(int) - 1];

	for (exception = 2; exception <= 23; exception++)
      exceptionHandler(exception,_catchException);   

	/* level 7 interrupt */
	exceptionHandler(31,_debug_level7);    
	
	/* breakpoint exception (trap #1) */
	exceptionHandler(33,_catchException);
  
	/* This is a trap #8 instruction.  Apparently it is someone''s software
     * convention for some sort of SIGFPE condition.  Whose?  How many
     * people are being screwed by having this code the way it is?
     * Is there a clean solution?  
	 */
	exceptionHandler(40,_catchException);
  
	/* 48 to 54 are floating point coprocessor errors */
	for (exception = 48; exception <= 54; exception++)
      exceptionHandler(exception,_catchException);   

	if (oldExceptionHook != remcomHandler)
	{
		oldExceptionHook = exceptionHook;
		exceptionHook    = remcomHandler;
	}
  
	initialized = 1;
}

   
/*=======================================================================*/
void breakpoint(void)
/*=======================================================================*/
{
	/* This function will generate a breakpoint exception.  It is used at the
	 *  beginning of a program to sync up with a debugger and can be used
	 *  otherwise as a quick means to stop program execution and "break" into
	 *  the debugger. 
	 */
	if (initialized) BREAKPOINT();
}



