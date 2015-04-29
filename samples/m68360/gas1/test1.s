| test1.s
| print a simple message and wait
|
| DJ (c) 1996
| 
| USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
| WITH NO WARRANTY FOR ANY PURPOSE.
|

  | this is absolute address `START' (0x1000)  
  | display welcome message
.global start
start:
  JSR _DUART_INIT
  LEA MESSAGE,A0
  JSR _DUART_PRINT
  | infinite loop  
LOOP:	BRA LOOP

MESSAGE:
	.ascii "Welcome on BSVC/68k\12\0"
