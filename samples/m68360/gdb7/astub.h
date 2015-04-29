/*
 * astub.h
 * 
 * Assembly language macros for gdb stub (extracted from m68k-stub.c)
 *
 * DJ, July 96.
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */

#ifndef ASTUB_H
#define ASTUB_H

#define BREAKPOINT() asm("    trap #1");

#define DISABLE_INTERRUPTS() asm("    oriw #0x0700,sr");
/*
 * Assembly written functions
 */
extern void return_to_super(void);
extern void return_to_user(void);
extern void _catchException(void);

#endif /* ASTUB.H */


