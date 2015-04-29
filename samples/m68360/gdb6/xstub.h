/*
 * xstub.h
 * 
 * Miscellaneous functions for gdb stub
 *
 * DJ, July 96.
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */

#ifndef XSTUB_H
#define XSTUB_H


/* 
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 * at least NUMREGBYTES*2 are needed for register packets
 */
#define BUFMAX 400

typedef void (*ExceptionHook)(int);   /* pointer to function with int parm */


extern ExceptionHook exceptionHook;  /* hook variable for errors/exceptions */

extern int getDebugChar(void);
extern void putDebugChar(int c);
extern void exceptionHandler();  /* assign an exception handler */
extern void flush_i_cache();

extern void getpacket(char *buffer);
extern void putpacket(char *buffer);

#endif /* XSTUB_H */

