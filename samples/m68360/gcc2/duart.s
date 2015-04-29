|
| duart.s
|
| Assembly language function giving access to the 68881 DUART.
| (Adapted from 'MatrixMultiply.s' example).
|
| DJ 1996/11
| 
| USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
| WITH NO WARRANTY FOR ANY PURPOSE.
|

| Implements:
| - _DUART_INIT: initialize the DUART (can be called from C)
| - _DUART_GETC: get a character, returned in register D0 (can be called from C)
| - _DUART_PUTC: output a character, expected in the stack (can be called from C)
| - _DUART_PRINT: output a string of character (use A0 as parameter)
|   (DO NOT CALL FROM C!)

.equ DUA_ADD, 0xeffc01
.equ DUA_MR1A,	0
.equ DUA_MR2A,	0
.equ DUA_SRA,	2
.equ DUA_CRA,	4
.equ DUA_CSRA,	2
.equ DUA_TBA,	6
.equ DUA_RBA,	6
.equ DUA_IMR,	10
.equ DUA_TBB,	0x16
.equ DUA_CRB,	0x14
.equ DUA_MR1B,	0x10
.equ DUA_MR2B,	0x10
.equ DUA_SRB,	0x12
.equ DUA_IVR,	0x18
.equ LINEFEED,	10		| LineFeed Character value
.equ CARR_RETURN,	13		| Carrage Return character value

|
| _DUART_INIT: Initialization of the duart
|
.global _DUART_INIT
_DUART_INIT: MOVE.L A1,-(A7)			| Save A1
	LEA	DUA_ADD,A1
	ORI 	#0b0000011100000000,SR		| Masks interrupts
	MOVE.B	#0b00010000,(DUA_CRA,A1)	| Reset MR?A pointer
	MOVE.B  #0b00100011,(DUA_MR1A,A1)	| 8 data bits
	MOVE.B  #0b00010111,(DUA_MR2A,A1)	| Normal Mode
	MOVE.B  #0b10111011,(DUA_SRA,A1)	| Set clock to 9600
	MOVE.B  #0b00000101,(DUA_CRA,A1)	| Enable Rx and Tx
	MOVE.L	(A7)+,A1					| Restore A1
	RTS

|
| _DUART_PUTC: The character in the stack is transmitted to the CONSOLE ACIA.
| The newline character <CR> is expanded into <LF>/<CR>.
| Define a subroutine DUA_PUTC, which print the character contained in D0
|

DUA_PUTC:	BTST #2,DUA_SRA+DUA_ADD	| Test Transmit data register empty
	BEQ.S	DUA_PUTC				| If not keep polling
	MOVE.B	D0,DUA_TBA+DUA_ADD		| Transmit the character
	CMP.B	#LINEFEED,D0			| Check for LINEFEED
	BNE		DUA_PUTCEXT				| if not L/F, exit
	MOVE.B	#CARR_RETURN,D0			| Load CARR_RETURN into D0
	BRA		DUA_PUTC				| Output C/R
DUA_PUTCEXT: RTS

.global _DUART_PUTC
_DUART_PUTC:	LINK A6,#-2
	MOVE.L 	(8,A6),D0
	JSR 	DUA_PUTC	
	UNLK 	A6
	RTS

|
| _DUART_GETC: Get a character from the CONSOLE ACIA and return it in D0
|

.global _DUART_GETC
_DUART_GETC:	BTST	#0,DUA_SRA+DUA_ADD	| Test Receive data register full
	BEQ.S	_DUART_GETC					| If not keep polling
	MOVE.B	DUA_RBA+DUA_ADD,D0			| Read the character
	RTS

|
| _DUART_PRINT: print a string of charater, until null
| 
.global _DUART_PRINT
_DUART_PRINT:	MOVE.L		D0,-(A7)	| Save register d0 to stack
DUAPRLOOP:		MOVE.B		(A0)+,D0	| Get a character to print
		CMP.B	#0x00,D0				| Is it null?
		BEQ		DUAPRNTEND				| yes: it's over
		JSR		DUA_PUTC				| no: print it
		BRA		DUAPRLOOP				| repeat
DUAPRNTEND:	MOVE.L		(A7)+,D0		| Restore d0
		RTS

| 
| End of file
|


