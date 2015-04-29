/* 
 * test1.cmd: command file for gnu ld 
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
.data 0x0 : { boot1.o (.data) }
.text 0x1000 : { test1.o (.text) duart.o (.text) }
}

OUTPUT(test1.H68)
OUTPUT_FORMAT(srec)
