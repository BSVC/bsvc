|
| boot.s
|
| Set the initial stack and reset vector
| Install exception routines
| Jump to '_main'
|
| DJ (c) 1996
| 
| USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
| WITH NO WARRANTY FOR ANY PURPOSE.
|

|
| INITIAL STACK AND RESET VECTOR 
|
.equ BOO_STACK,0xF000
.equ APP_START,0x4000

.data
.long BOO_STACK			| Vector 0 (SSP)
.long BOO_START			| Vector - (PC)
.space 1016				| (256-2)*4
.even
|DS.L 245
	
|
| BOOT CODE
|
.text
.global BOO_START
BOO_START:
	| initialize exception vectors
	LEA 0x08,A0				| start with vector 2
	LEA _exceptionTable,A1
BOO_START_L1:		
	MOVE.L A1,(A0)+
	ADD.L #6,A1
	CMP.L #0x400,A0			| $400 is the end of vector table
	BLT BOO_START_L1
	| initialize exception table with 'jsr unimplemented-exception'
	MOVE.W 0,D0
	LEA _exceptionTable,A0
BOO_START_L2:
	MOVE.W #0x4EB9,(A0)+	| 4EB9 is 'jsr'
	MOVE.L #BOO_UNIMP_EXCPT,(A0)+
	ADDQ.W #1,D0
	CMP.W #255,D0
	BLT BOO_START_L2

| we're ready: unmask interrupt
	ANDI #0b1111100011111111,SR
		
| jump to _main	
	JMP _main

|
| Exception: unimplemented
|
.global BOO_UNIMP_EXCPT
BOO_UNIMP_EXCPT:	
    MOVE.L (sp)+,d2 		| get return address and 
	LEA _exceptionTable,a3  | compute exception number
	SUB.L a3,d2						 
	DIVS.W 	#6,d2
	ADDQ.W 	#1,d2
	MOVE.B d2,d3
	AND.B #0xF, d2
	CMP.B #10,d2
	BGT zz1
	ADD.B #'0',d2
	BRA zz2	
zz1:
	ADD.B #'A',d2
zz2:
	ASR.B #4,d3
	AND.B #0xF, d3
	CMP.B #10,d3
	BGT zz3
	ADD.B #'0',d3
	BRA zz4
zz3:
	ADD.B #'A',d3
zz4:  
	LEA BOO_UNIMP_EXCPT_NUM,A0
	MOVE.B d3,(A0)+
	MOVE.B d2,(A0)+
	JSR _DUART_INIT
	LEA BOO_UNIMP_EXCPT_TEXT,A0
	JSR _DUART_PRINT

BOOTL5:	BRA BOOTL5

BOO_UNIMP_EXCPT_TEXT: 
	.ascii "|| unimplemented exception 0x"
BOO_UNIMP_EXCPT_NUM:
	.ascii "XX"
	.ascii "||\12\0"
	.even

|
| Exception table
|
| exception table is a set of 255 group of 6 bytes, each group is
| a instruction executed when the 'corresponding' exception occurs
| These group are initialized with an simple instruction
| "jsr unimplemented-exception", but this can be overwritten.
|
.global _exceptionTable
_exceptionTable:
.space 1530 || (255*6)

|
| End of file
|
