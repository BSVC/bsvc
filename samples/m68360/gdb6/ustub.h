/*
 * ustub.h
 * 
 * Utilities functions for gdb stub
 *
 * DJ, July 96.
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */

#ifndef USTUB_H
#define USTUB_H

extern const char hexchars[];

extern int hex(char ch);
extern int hexToInt(char **ptr, int *intValue);

extern char* mem2hex(char* mem, char* buf, int count);
extern char* hex2mem(char *buf, char *mem, int count);


#endif /* USTUB_H */
