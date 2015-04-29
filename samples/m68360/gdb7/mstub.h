/*
 * mstub.h
 * 
 * Definitions for gdb stub (extracted from m68k-stub.c)
 *
 * DJ, July 96.
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */

#ifndef MSTUB_H
#define MSTUB_H

/* there are 180 bytes of registers on a 68020 w/68881
 * many of the fpa registers are 12 byte (96 bit) registers 
 */
/* only 16*4+4+4=72 on a 68000/CPU32 ! */
#if 0
#define NUMREGBYTES 180
#else
#define NUMREGBYTES 72
#endif
enum regnames {D0,D1,D2,D3,D4,D5,D6,D7, 
               A0,A1,A2,A3,A4,A5,A6,A7, 
               PS,PC,
               FP0,FP1,FP2,FP3,FP4,FP5,FP6,FP7,
               FPCONTROL,FPSTATUS,FPIADDR
              };


/* We keep a whole frame cache here.  "Why?", I hear you cry, "doesn''t
 * GDB handle that sort of thing?"  Well, yes, I believe the only
 * reason for this cache is to save and restore floating point state
 * (fsave/frestore).  A cleaner way to do this would be to make the
 * fsave data part of the registers which GDB deals with like any
 * other registers.  This should not be a performance problem if the
 * ability to read individual registers is added to the protocol.  
 */

typedef struct FrameStruct
{
    struct FrameStruct  *previous;
    int       exceptionPC;      /* pc value when this frame created */
    int       exceptionVector;  /* cpu vector causing exception     */
    short     frameSize;        /* size of cpu frame in words       */
    short     sr;               /* for 68000, this not always sr    */
    int       pc;
    short     format;
    int       fsaveHeader;
    int       morejunk[0];        /* exception frame, fp save... */
} Frame;

#define FRAMESIZE 500

extern int*stackPtr;

extern void handle_exception(int exceptionVector);
extern Frame *lastFrame;

#endif /* MSTUB_H */



