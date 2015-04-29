/*
 * libc.h
 * 
 * Declares some useful function of the C standard library
 * (should be standard header, such as 'stdio.h', 'string.h'...
 * 
 * DJ (c) 1996
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */

#ifndef LIBC_H
#define LIBC_H

/* stdio.h */
#define NULL ((void*)0)
extern void puts(const char*);
extern void gets(char*);
extern void putchar(char);

/* string.h */
extern char *strcpy(char* s1, const char *s2);
extern int strlen(const char *s);
extern int strcmp(const char *s1, const char *s2);
extern int strncmp(const char *s1, const char *s2, int n);

/* memory.h */
extern void *memset(void* addr, int value, int length);
extern void bcopy(char *b1, char*b2, int length);

/* stdlib.h */
extern int atoi(const char *s);

/* setjmp.h */
typedef char jmp_buf[68];
extern int setjmp(jmp_buf);
extern int longjmp(jmp_buf,int);

/* SPECIFIC */
extern void itoa(unsigned short value, char* s);
extern void itox(unsigned short value, char* s);

#endif /* LIBC_H */

/*
 * End of file
 */


