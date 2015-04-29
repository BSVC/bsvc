/*
 * astub.c
 * 
 * Assembly language written function for m68k stub
 * (extracted from m68k-stub.c)
 * 
 * DJ, July 1996
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */


#include "mstub.h"
#include "astub.h"

asm("
.text
.globl _return_to_super
_return_to_super:
        movel   _registers+60,sp /* get new stack pointer */        
        movel   _lastFrame,a0   /* get last frame info  */              
        bra     return_to_any

.globl _return_to_user
_return_to_user:
        movel   _registers+60,a0 /* get usp */                          
        movel   a0,usp           /* set usp */				
        movel   _superStack,sp  /* get original stack pointer */        

return_to_any:
        movel   _lastFrame,a0   /* get last frame info  */              
        movel   a0@+,_lastFrame /* link in previous frame     */        
        addql   #8,a0           /* skip over pc, vector# */
        movew   a0@+,d0         /* get # of words in cpu frame */       
        addw    d0,a0           /* point to end of data        */       
        addw    d0,a0           /* point to end of data        */       
        movel   a0,a1                                                   
#                                                                       
# copy the stack frame                                                  
        subql   #1,d0
copyUserLoop:                                                               
        movew   a1@-,sp@-                                               
        dbf     d0,copyUserLoop                                             
");                                                                     
   asm("   moveml  _registers,d0-d7/a0-a6");			        
   asm("   rte");  /* pop and go! */                                    

/* this function is called immediately when a level 7 interrupt occurs */
/* if the previous interrupt level was 7 then we''re already servicing  */
/* this interrupt and an rte is in order to return to the debugger.    */
/* For the 68000, the offset for sr is 6 due to the jsr return address */
asm("
.text
.globl __debug_level7
__debug_level7:
	movew   d0,sp@-");
#if defined (mc68020) || defined (mc68332)
asm("	movew   sp@(2),d0");
#else
asm("	movew   sp@(6),d0");
#endif
asm("	andiw   #0x700,d0
	cmpiw   #0x700,d0
	beq     _already7
        movew   sp@+,d0	
        bra     __catchException
_already7:
	movew   sp@+,d0");
#if !defined (mc68020) && !defined (mc68332)
asm("	lea     sp@(4),sp");     /* pull off 68000 return address */
#endif
asm("	rte");


#if defined (mc68020) || defined (mc68332)
/* This function is called when a 68020 exception occurs.  It saves
 * all the cpu and fpcp regs in the _registers array, creates a frame on a
 * linked list of frames which has the cpu and fpcp stack frames needed
 * to properly restore the context of these processors, and invokes
 * an exception handler (remcom_handler).
 *
 * stack on entry:                       stack on exit:
 *   N bytes of junk                     exception # MSWord
 *   Exception Format Word               exception # MSWord
 *   Program counter LSWord              
 *   Program counter MSWord             
 *   Status Register                                   
 *                                       
 */
asm("
.text
.globl __catchException
__catchException:");
DISABLE_INTERRUPTS();
asm("
    moveml  d0-d7/a0-a6,_registers /* save registers        */
	movel	_lastFrame,a0	/* last frame pointer */
");
asm("
    lea     _registers,a5   /* get address of registers     */
    movel   sp@+,d2         /* pop return address           */
    movew   sp@,d1          /* get status register */
    movew   d1,a5@(66)      /* save sr		 	*/	
	movel   sp@(2),a4       /* save pc in a4 for later use  */
    movel   a4,a5@(68)      /* save pc in _registers[] 	*/

# figure out how many bytes in the stack frame
	movew   sp@(6),d0	/* get ''020 exception format	*/
    movew   d0,d2           /* make a copy of format word   */
    andiw   #0xf000,d0      /* mask off format type         */
    rolw    #5,d0           /* rotate into the low byte *2  */
    lea     _exceptionSize,a1   
    addw    d0,a1           /* index into the table         */
	movew   a1@,d0          /* get number of words in frame */
    movew   d0,d3           /* save it                      */
    subw    d0,a0		/* adjust save pointer          */
    subw    d0,a0		/* adjust save pointer(bytes)   */
	movel   a0,a1           /* copy save pointer            */
	subql   #1,d0           /* predecrement loop counter    */

# copy the frame
saveFrameLoop:
	movew  	sp@+,a1@+
	dbf     d0,saveFrameLoop

# now that the stack has been cleaned,
# save the a7 in use at time of exception
    movel   sp,_superStack  /* save supervisor sp           */
    andiw   #0x2000,d1      /* were we in supervisor mode ? */
    beq     userMode       
    movel   a7,a5@(60)      /* save a7                  */
    bra     a7saveDone
userMode:  
	movel   usp,a1    	
    movel   a1,a5@(60)      /* save user stack pointer	*/
a7saveDone:
# save size of frame
    movew   d3,a0@-

# compute exception number
    andl    #0xfff,d2   	/* mask off vector offset	*/
    lsrw    #2,d2   	/* divide by 4 to get vect num	*/
    movel   d2,a0@-         /* save it                      */

# save pc causing exception
    movel   a4,a0@-

# save old frame link and set the new value
	movel	_lastFrame,a1	/* last frame pointer */
	movel   a1,a0@-		/* save pointer to prev frame	*/
    movel   a0,_lastFrame

    movel   d2,sp@-		/* push exception num           */
	movel   _exceptionHook,a0  /* get address of handler */
    jbsr    a0@             /* and call it */
    clrl    sp@             /* replace exception num parm with frame ptr */
    jbsr     __returnFromException   /* jbsr, but never returns */
");
#else /* mc68000 */
/* This function is called when an exception occurs.  It translates the
 * return address found on the stack into an exception vector # which
 * is then handled by either handle_exception or a system handler.
 * _catchException provides a front end for both.  
 * 
 * stack on entry:                       stack on exit:
 *   Program counter MSWord              exception # MSWord 
 *   Program counter LSWord              exception # MSWord
 *   Status Register                     
 *   Return Address  MSWord              
 *   Return Address  LSWord             
 */
asm("
.text
.globl __catchException
__catchException:");
DISABLE_INTERRUPTS();
asm("
    moveml d0-d7/a0-a6,_registers  /* save registers               */
	movel	_lastFrame,a0	/* last frame pointer */
");
asm("
        lea     _registers,a5   /* get address of registers     */
        movel   sp@+,d2         /* pop return address           */
     	/* addl 	#1530,d2        / * convert return addr to 	*/
	    lea _exceptionTable,a3      
    	subl a3,d2						 
	    divs 	#6,d2   	/*  exception number (-1)		*/
    	extl    d2   
        addql #1,d2							 
        moveql  #3,d3           /* assume a three word frame     */

        cmpiw   #3,d2           /* bus error or address error ? */
        bgt     normal          /* if >3 then normal error      */
        movel   sp@+,a0@-       /* copy error info to frame buffer */
        movel   sp@+,a0@-       /* these are never used         */
        moveql  #7,d3           /* this is a 7 word frame       */
     
normal:   
	    movew   sp@+,d1         /* pop status register          */
        movel   sp@+,a4         /* pop program counter          */
        movew   d1,a5@(66)      /* save sr		 	*/	
        movel   a4,a5@(68)      /* save pc in _registers[]      	*/
        movel   a4,a0@-         /* copy pc to frame buffer      */
	    movew   d1,a0@-         /* copy sr to frame buffer      */

        movel   sp,_superStack  /* save supervisor sp          */

        andiw   #0x2000,d1      /* were we in supervisor mode ? */
        beq     userMode       
        movel   a7,a5@(60)      /* save a7                  */
        bra     saveDone             
userMode:
        movel   usp,a1    	/* save user stack pointer 	*/
        movel   a1,a5@(60)      /* save user stack pointer	*/
saveDone:

        movew   d3,a0@-         /* push frame size in words     */
        movel   d2,a0@-         /* push vector number           */
        movel   a4,a0@-         /* push exception pc            */

#
# save old frame link and set the new value
     	movel	_lastFrame,a1	/* last frame pointer */
    	movel   a1,a0@-		/* save pointer to prev frame	*/
        movel   a0,_lastFrame

        movel   d2,sp@-		/* push exception num           */
	    movel   _exceptionHook,a0  /* get address of handler */
        jbsr    a0@             /* and call it */
        clrl    sp@             /* replace exception num parm with frame ptr */
        jbsr     __returnFromException   /* jbsr, but never returns */
");
#endif


/*
 * remcomHandler is a front end for handle_exception.  It moves the
 * stack pointer into an area reserved for debugger use in case the
 * breakpoint happened in supervisor mode.
 */
asm(".global _remcomHandler");
asm("_remcomHandler:");
asm("    addl    #4,sp");        /* pop off return address     */
asm("    movel   sp@+,d0");      /* get the exception number   */
asm("    movel   _stackPtr,sp"); /* move to remcom stack area  */
asm("    movel   d0,sp@-");	/* push exception onto stack  */
asm("    jbsr    _handle_exception");    /* this never returns */
asm("    rts");                  /* return */
