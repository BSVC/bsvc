/*
 * test4.cmd: command file for gnu ld
 *
 * DJ (c) 1996
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 *
 */

MEMORY
{
ram : ORIGIN = 0, LENGTH = 0x10000
device : ORIGIN = 0xeffc00, LENGTH = 0x100
}

SECTIONS
{
.data 0x0 : { boot2.o (.data) }
.text 0x1000 : { *(.text) }
.text 0xA000 : { *(.data) }
}

OUTPUT(test4.H68)
OUTPUT_FORMAT(srec)
