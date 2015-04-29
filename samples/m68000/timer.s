*----------------------------------------------------------------------
*
* This file contains the following:
*
*    1) main     - program which tests the timer
*    2) tstart   - subroutine which sets the timer
*    3) thandler - timer interrupt handler
*    4) delay    - delay function based on polling, but uses the timer 
*    5) functions for basic character I/O 
*
*		M. Vuskovic, July 20, 1998
*----------------------------------------------------------------------

		ORG     $0
		DC.L    $8000           Stack pointer value after a reset 
		DC.L    main            Program counter value after a reset

*  ______________________________________________________________
* | DUART Port Constants.  These are required to use the
* | DUART port.
* |______________________________________________________________

DUART   equ $effc01             DUART address for XTerm sim board
MR1A    EQU 0
MR2A    EQU 0
SRA     EQU 2
CRA     EQU 4
CSRA    EQU 2
TBA     EQU 6
RBA     EQU 6
IMR     EQU 10
TBB     EQU $16
CRB     EQU $14

* This program tests the timer. It starts the timer by calling "tstart"
* then enters an idle loop, which is interrupted every 1 second
* The timer interrupt handler is given in "tih.sa"

		ORG     $2000           Start at location 2000 Hex

main:						
		movea.l	#$8000,sp	 	 ; Allocate the stack	

* Rquired steps to print to the DUART port
* ______________________________________________________________
		LEA     DUART,A1
		MOVE.B  #%00010000,CRA(A1)     Reset MR?A pointer
		MOVE.B  #%00100011,MR1A(A1)    8 data bits
		MOVE.B  #%00010111,MR2A(A1)    Normal Mode
		MOVE.B  #%10111011,CSRA(A1)    Set clock to 9600
		MOVE.B  #%00000101,CRA(A1)     Enable Rx and Tx
* ______________________________________________________________

		move.w	#$2400,SR
		jsr     tstart       	 ; Set up the timer
loop:		muls.w	d1,d1		 ; Idle loop
		btst	#0,TCR(A0)
		bne	loop
		bset	#0,TCR(A0)
		bra	loop
		  

* Timer constants:

TIMER		equ	$10021		; Timer base address
TCR		equ	0		; Offset to Timer Control Register
TIVR		equ	2		; Offset to Timer Interrupt Vector Reg
CPR		equ	4		; Offset to Counter Preload Register
TSR		equ	20		; Offset to Timer Status Register
TIVN		equ	84		; Timer Interrupt Vector Number
TIVA 		equ	TIVN*4		; Timer Interrupt Vector Address
TICSEC		equ	125000		; Number of ticks per 1 second


*					; Set up the timer and start it
tstart:						
	 	move.l	#thandler,TIVA	; Setup i/r vector
		lea	TIMER,a0	; Get pointer to timer data
		move.b	#TIVN,TIVR(a0)	; Tell timer the i/r vector number	
		move.l	#TICSEC,d0	; Compute number of ticks
		movep.l	d0,CPR(a0)	; Load CPR 
		move.b	#$A0,TCR(a0)	; Initialize timer (timer disabled)
		bset	#0,TCR(a0)	; Enable timer
		rts
	
	
* 					; Timer interrupt handler
*					; Outputs a wakeup message	
thandler:
		move.l	a0,-(SP)
		move.w	#$2700,SR	; Disable interrupts
		jsr	newline

		pea	TIMERMESS
		jsr 	outstr

		jsr	newline
		lea	TIMER,A0	; Get the timer address
		bclr	#0,TSR(A0)	; Negate timer interrupt request
		move.l	(SP)+,a0
		rte			; Return from interrupt


*----------------------------------------------------------------------
* Entry point:	delay
* Description:	Delays execution of the caller for T seconds
* Input:		T - number of seconds (register d0)
* Output:		None
* Comment:		Subroutine is based on polling instead of timer interrupt
* Side effects:	None
*----------------------------------------------------------------------

delay:	
		lea	TIMER,a0 	; Pointer to timer data structure
		move.b	#$80,TCR(a0)	; Initialize timer:
*				  	; ...no vectored interrupt
*				   	; ...reset condition
*				   	; ...clock control (=div 32)
*				   	; ...timer off
 		move.l	#TICSEC,d0	; Get number of ticks 
*		asl.l	#3,d0		; "
		movep.l	d0,CPR(a0) 	; Load CPR with number of ticks
		bset	#0,TCR(a0)	; Start the timer
wait:
		btst	#0,TSR(a0)	; Wait for zero count
		beq	wait
		bclr	#0,TSR(a0)	; Stop the timer
		rts

TIMERMESS	dc.b	'Timer: Time to wake up!',$07,0
NOTYET		dc.b	'MainBody: Doing the Do',$07,0


*----------------------------------------------------------------------
*
*  Basic I/O functions:	
*					  
*----------------------------------------------------------------------

* Global symbols:
* Make sure ACIA is the same addrss of the M6... terminal.
*ACIA	equ	$50040			ACIA address for FORCE board
*ACIA   equ	$1000			ACIA address for MOTOROLA board
DUART	equ	$effc01			ACIA address for XTerm sim board
TBA		equ	6
RBA		equ 6
CR		equ	$0d		Carriage Return
LF		equ	$0a		Line Feed
BLNK	equ	$20			Blank character
NULL	equ	$0			NULL character for string termination

*----------------------------------------------------------------------
* Entry point:	inch
* Description:	Inputs character from the keyboard
* Input:		None
* Output:		Character from the keyboard (in register d0)
* Comment:		Subroutine is based on busy-wait (polling)
* Side effects:	None
*----------------------------------------------------------------------
inch   	move.l  a0,-(a7)
        move.l  #DUART,a0      	Point a0 at the DUART control and status reg
inlp    btst    #0,SRA(a0)
        beq     inlp          	Wait for next character to arrive
        move.b  RBA+DUART,d0    Move char from DUART receiver buffer into d0
*        and.l   #$0000007f,d0 	Clear parity bit and high bytes of d0
        move.l  (a7)+,a0
        rts

*----------------------------------------------------------------------
* Entry point:	outch
* Description:	Outputs character to the terminal screen
* Input:		Pointer to the character (longword on stack)
* Output:		None
* Comment:		Subroutine is based on busy-wait (polling)
* Side effects:	None
*----------------------------------------------------------------------
c_byte	equ	8
c_size	equ 2

outch:	 link	a6,#0
		 movem.l d0/a0,-(sp)

*         move.l #DUART,a0	* Point a0 at the ACIA control and status reg
outlp:   btst   #2,SRA+DUART	* Test Transmit data register empty.
         beq.s  outlp         	* If not keep polling.
         move.b c_byte(a6),d0   * Move stack parameter to d0.
         move.b  d0,TBA+DUART   * Transmit the paramiter in d0.

         movem.l  (sp)+,d0/a0   * Restore registers
         unlk	 a6
         move.l	 (sp),c_size(sp)
         addq.l	 #2,sp
         rts


*----------------------------------------------------------------------
* Entry point:	inpstr
* Description:	Reads and echoes a string from the terminal
* Input:		Pointer to the beginning of the buffer where the
*			string should be placed (longword on stack)
* Output:		None			
* Comment:		Input string is any sequence of printable characters
*			delimited by blank or carriage return.
*			Subroutine is attaching a NULL character to the end of
*			the string.
*----------------------------------------------------------------------
inpstr:
	movem.l	d0/a0,-(sp)		Save registers
	move.l	12(sp),a0		Get pointer to the string buffer
inplp:
	jsr		inch		Get character from terminal
	move.b	d0,-(sp)		Call outch (echo character)
	jsr 	outch			"
*	add.l	#4,sp			"NOT NEEDED OUTCH pops VALUES.
	cmp.b	#BLNK,d0		If end of input string
	beq		finish		... exit the loop
	cmp.b	#CR,d0      		"
	beq		finish 		"
	move.b	d0,(a0)+		Put character into buffer
	bra		inplp		Iterate
finish:
	move.b	#NULL,(a0)		Put string terminator into buffer
	movem.l	(sp)+,d0/a0		Restore registers
	rts

*----------------------------------------------------------------------
* Entry point:	outstr
* Description:	Outputs string of characters to the terminal screen
* Input:		Pointer to the string (longword on stack)
* Output:		None
* Comment:		String terminator is NULL character
* Side effects:	None
*----------------------------------------------------------------------
STRING		equ	8
STRSIZE		equ 4

outstr:  link	 a6,#0
		 movem.l d0/a0,-(sp)

         move.l  STRING(A6),A0     	Get pointer to string
strlp:   tst.b   (a0)
         beq     strout        	Quit when we see NULL

         move.b  (a0),d0       	Get next byte into d0.lowbyte
         move.b  d0,-(sp)     	Call outch
         jsr     outch

         add.l   #1,a0         	Advance pointer
         bra     strlp         	End of loop

strout:  movem.l (sp)+,d0/a0
		 unlk	a6
		 move.l	 (sp),STRSIZE(sp)
		 addq.l	 #STRSIZE,sp
         rts

*----------------------------------------------------------------------
* Entry point:	newline
* Description:	Outputs new line to the terminal and the host
* Input:		None
* Output:		None
* Comment:		CR and LF are sent to the terminal, while only CR
*				is sent to the host. Subroutine then synchronizes 
*				communication between the host and the single board.
* Side effects:	None
*----------------------------------------------------------------------
newline:
	movem.l	a0/d0,-(sp)
	move.b  #CR,-(sp)		Call outch (send CR to terminal)	
	jsr	outch				"
*	move.l  #DUART,a0    		Pointer to ACIA
outlp2:
	btst    #2,SRA+DUART		Send line feed to terminal 
	beq     outlp2      		"
	move.b  #LF,TBA+DUART 		Transmit the paramiter in d0.
	movem.l	(sp)+,a0/d0
	rts

	end

