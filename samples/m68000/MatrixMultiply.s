*
* Interactive Matrix Multiply program for the Motorola 68000
* 
        ORG     $0 
        DC.L    $8000           Stack pointer value after a reset
        DC.L    START           Program counter value after a reset


	ORG	$1000

DUART	EQU	$effc01
MR1A	EQU	0
MR2A	EQU	0
SRA	EQU	2
CRA	EQU	4
CSRA	EQU	2
TBA	EQU	6
RBA	EQU	6
IMR	EQU	10
TBB	EQU	$16
CRB	EQU	$14
MR1B	EQU	$10
MR2B	EQU	$10
SRB	EQU	$12

START:	LEA	DUART,A1
	MOVE.B	#%00010000,CRA(A1)     Reset MR?A pointer
	MOVE.B  #%00100011,MR1A(A1)    8 data bits
	MOVE.B  #%00010111,MR2A(A1)    Normal Mode
	MOVE.B  #%10111011,CSRA(A1)    Set clock to 9600
	MOVE.B  #%00000101,CRA(A1)     Enable Rx and Tx

	MOVE.L	#$4000,A7              Setup the stack pointer

MAIN:	JSR CLEAR
	JSR TITLE

	JSR CLEAR

	JSR GETSIZE

	MOVE.W	MA_C,D1
	MOVE.W	MA_R,D0
	MOVE.L	#MA_DAT,A0
	MOVE.L	#MA_TI,A1
	JSR	MATIN

	MOVE.W	MB_C,D1
	MOVE.W	MB_R,D0
	MOVE.L	#MB_DAT,A0
	MOVE.L	#MB_TI,A1
	JSR MATIN

	MOVE.W	MA_R,D0
	MOVE.W	MA_C,D1
	MOVE.W	MB_R,D2
	MOVE.W	MB_C,D3
	MOVE.L	#MA_DAT,A0
	MOVE.L	#MB_DAT,A1
	MOVE.L	#MC_DAT,A2
	JSR	MATMULT

	MOVE.W	MA_R,D0
	MOVE.W	MB_C,D1
	MOVE.L	#MC_DAT,A0
	MOVE.L	#MC_TI,A1
	JSR	MATOUT

	JMP	EXIT
	NOP

MA_R:	DS.W	1
MA_C:	DS.W	1
MB_R:	DS.W	1
MB_C:	DS.W	1
MA_DAT:	DS.W	100				Matrix A Data
MB_DAT:	DS.W	100                             Matrix B Data
MC_DAT: DS.L	100				Matrix C Data (Result)
MA_TI:	DC.B	'Enter Matrix A',0,0
MB_TI:	DC.B	'Enter Matrix B',0,0
MC_TI:	DC.B	'Resulting Matrix C',0,0

*
* Input a string
*

INPUT:		MOVEM.L	A0-A6/D0-D7,-(A7)

		MOVE.L	A0,A5			Save buffer address
		MOVE.L	A1,A6			Save valid string address
                MOVE.L	D0,D7			Save Max length
		MOVE.L	#0,D1			Current string length

IN_LOOP:	JSR	GETC			Get a character

		CMPI.B	#$08,D0			Is it backspace
		BEQ	IN_BS

		CMPI.B	#$0D,D0			Is it return
		BEQ	IN_RET

* Make sure it's a valid character
		MOVE.L	A6,A0
IN_VL:		CMPI.B	#0,(A0)			End of valid string?
		BEQ	IN_INVALID

		CMP.B	(A0)+,D0		Is it a valid char?
		BEQ	IN_VALID

		BRA	IN_VL			Loop

IN_INVALID:	JSR	BELL			Sound Bell
		JMP	IN_LOOP			Get another character

IN_VALID:	CMP.W	D7,D1			Have we reach max lenght?
		BNE	IN_VA2			NO!

		JSR	BELL
		JMP	IN_LOOP

IN_VA2:		MOVE.B	D0,0(A5,D1)		Save character in buffer
		ADDI.W	#1,D1			Inc Lenght counter
		JSR	PUTC			Output character to screen

		JMP	IN_LOOP


IN_BS:		CMPI.W	#0,D1			Make sure no wrap around
		BNE	IN_BOK

		JSR	BELL
		JMP	IN_LOOP

IN_BOK:		MOVE.B	#$08,D0			BackSpace on screen
		JSR	PUTC
		MOVE.B	#$20,D0
		JSR	PUTC
		MOVE.B	#$08,D0
		JSR	PUTC

		SUBI.W	#1,D1			Dec length counter
		JMP	IN_LOOP


IN_RET:		MOVE.B	#0,0(A5,D1)		Null Terminate

		MOVEM.L	(A7)+,A0-A6/D0-D7	Restore REGS

		RTS
*
* Routines for performing Polled I/O At the Console Serial Port
*
* By: Bradford W. Mott
*

LINEFEED:	EQU	10		* LineFeed Character value
CARR_RETURN:	EQU	13		* Carrage Return character value

*
* The character in D0 is transmitted to the CONSOLE ACIA.
* The newline character <LF> is expanded into <LF>/<CR>.
*

PUTC:	BTST	#2,SRA+DUART		* Test Transmit data register empty
	BEQ.S	PUTC			* If not keep polling
	MOVE.B	D0,TBA+DUART		* Transmit the character
	CMP.B	#LINEFEED,D0		* Check for LINEFEED
	BNE	PUTEXT
	MOVE.B	#CARR_RETURN,D0		* Load CR into D0
	BRA	PUTC			* Output CR
PUTEXT:	RTS				* Return to calling procedure


*
* Get a character from the CONSOLE ACIA and return it in D0
*

GETC:	BTST	#0,SRA+DUART		* Test Receive data register full
	BEQ.S	GETC			* If not keep polling
	MOVE.B	RBA+DUART,D0		* Read the character
	RTS

PRINT:		MOVE.L		D0,-(A7)	*SAVE CURRENT REGS TO STACK
PRLOOP:		MOVE.B		(A0)+,D0	*GET A CHARACTER
		CMP.B		#$00,D0		*IS IT A NULL CHARACTER?
		BEQ		PRNTEND		*IF SO, THEN YOU'RE DONE
		JSR		PUTC		*IF NOT, THEN SEND CHARACTER TO SCREEN
		BRA		PRLOOP		*LOOP UNTIL DONE
PRNTEND:	MOVE.L		(A7)+,D0	*RESTORE REGS
		RTS


TITLE:		MOVE.L 	#SCREEN,A0	*INITIALIZE SCREEN DATA POINTER
TILP:		JSR	PRINT		*PRINT DATA TO SCREEN
		CMPI.B	#$3D,(A0)	*END OF SCREEN DATA?
		BNE	TILP		*IF NOT THEN KEEP PRINTING DATA
TILP1:		JSR	GETC		*IF FINISHED, GET A KEY SRIKE
		CMPI.B	#$0D,D0		*IS IT THE RETURN KEY?
		BNE	TILP1		*IF NOT THEN TRY AGAIN
		JSR	PRINT		*IF SO, THEN PRINT CLEAR SCREEN CHARACTERS
		RTS			*RETURN TO MAIN

SCREEN:		DC.B	$1B,'[2J'	
		DC.B	'****',$08,$08,$08,$08,$1B,'[4h'
		DC.B	'                                                                        ****',$1B,'[4l',$1B,'E'
		DC.B	'**',$08,$08,$1B,'[4h'
		DC.B	'                                                                            **',$1B,'[4l',$1B,'E'
		DC.B	'* *',$08,$08,$08,$1B,'[4h'
		DC.B	'                                                                          * *',$1B,'[4l',$1B,'E'
		DC.B	'*',$08,$1B,'[4h'
		DC.B	'                                                                              *',$1B,'[4l',$1B,'E',$1B,'E'
		DC.B	'                      M   M   A   TTTTT RRRR   III  X   X',$1B,'E'
		DC.B  	'                      MM MM  A A    T   R   R   I    X X ',$1B,'E'
		DC.B	'                      M M M A   A   T   R   R   I     X  ',$1B,'E'
		DC.B	'                      M   M AAAAA   T   RRRR    I    X X ',$1B,'E'
		DC.B	'                      M   M A   A   T   R  R    I   X   X',$1B,'E'
		DC.B	'                      M   M A   A   T   R   R  III  X   X',$1B,'E'
		DC.B	$1B,'E'
		DC.B	'                M   M U   U L     TTTTT  III  PPPP  L     Y   Y',$1B,'E'
		DC.B	'                MM MM U   U L       T     I   P   P L     Y   Y',$1B,'E'
		DC.B	'                M M M U   U L       T     I   P   P L      Y Y ',$1B,'E'
		DC.B 	'                M   M U   U L       T     I   PPPP  L       Y  ',$1B,'E'
		DC.B 	'                M   M U   U L       T     I   P     L       Y  ',$1B,'E'
		DC.B	'                M   M  UUU  LLLLL   T    III  P     LLLLL   Y  ',$1B,'E'
		DC.B	$1B,'E'
		DC.B	'*',$08,$1B,'[4h'
		DC.B	'                                                                              *',$1B,'[4l',$1B,'E'
		DC.B	'* *',$08,$08,$08,$1B,'[4h'
		DC.B	'                                                                          * *',$1B,'[4l',$1B,'E'
		DC.B	'**',$08,$08,$1B,'[4h'
		DC.B	'                                                                            **',$1B,'[4l',$1B,'E'
		DC.B	'****',$08,$08,$08,$08,$1B,'[4h'
		DC.B	'                                                                        ****',$1B,'[4l',$1B,'E'
		DC.B	'Press RETURN to Continue... ',$1B,'[4l',$00,$3D,$1B,'[2J',$00,$00
                                                                                
LTOA:		MOVEM.L		D0-D7/A0-A6,-(A7)

		MOVE.B		#$00,-(A0)	*APPEND NULL CHARACTER TO OUTPUT STRING
		MOVE.B		#$00,D2		*SET NEGATIVE FLAG TO INDICATE NON NEGATIVE NUMBER FOR NOW
		MOVE.L		D1,D3		*MOVE HEX OUTPUT (D1) TO HOLDER (D3)
		BPL		LTOALOOP	*IF POSITIVE THEN GO CONVERT
		BNE 		LTOA2		*IF ZERO THEN YOUR FINISHED

		MOVE.B		#48,-(A0)
		BRA		LTOAEND

LTOA2:		NEG.L		D3		*IF NEGATIVE THEN NEGATE
		MOVE.B		#$AA,D7		*SET NEGATIVE FLAG

LTOALOOP:	JSR		LDIV		*GO PERFORM LONG DIV BY 10 ON HEX NUMBER
		ADD.W		#$30,D3		*ADD $30 TO REMAINDER
		MOVE.B		D3,-(A0)	*MOVE LSB TO ASCII CHAR STRING POINTER
		MOVE.L		D0,D3		*MOVE RECEIVED QUOTIENT TO D3 TO SEND TO LDIV AGAIN
		TST.L		D0		*IS QUOTIENT EQUAL TO ZERO?
		BNE		LTOALOOP	*IF NOT THEN KEEP CONVERTING

		CMPI.B		#$AA,D7		*WAS NEGATIVE FLAG SET?
		BNE		LTOAEND		*NO, GO TO THE END
		MOVE.B		#$2D,-(A0)	*YES, APPEND A NEGATIVE SIGN TO ASCII STRING


LTOAEND:	MOVEM.L		(A7)+,D0-D7/A0-A6
		RTS				*RETURN


ATOL:		MOVEM.L		D1-D7/A0-A6,-(A7)	*SAVE PERTINENT REGISTERS
		MOVE.B		#0,D4		*SET NEGATIVE FLAG TO OFF
		MOVE.L		#0,D0		*CLEAR RESULT REGISTER
		MOVE.L		#0,D1		*CLEAR CHARACTER HOLDER

ATOLLP1:	MOVE.B		(A0)+,D1	*GET FIRST CHARACTER IN ASCII STRING
		BEQ		ATOLDONE	*IF NULL, THEN YOU'RE FINISHED
		MOVE.L		#VALIDSTR,A1	*SET CHARACTER CHECK POINTER

ATOLLP2:	CMP.B		#0,(A1)		*CHECKED FOR ALL VALID CHARACTERS?
		BEQ		ATOLLP1		*IF YES, GET NEXT CHARACTER IN ASCII STRING
		CMP.B		#$2D,D1		*IS IT THE NEGATIVE SIGN?
		BEQ		SETNEG		*IF YES, GO SET NEGATIVE FLAG
		CMP.B		(A1)+,D1	*IS THE CHARACTER VALID?
		BNE		ATOLLP2		*IF NOT, COMPARE TO OTHER VALID CHARACTERS
		
		AND.B		#$0F,D1		*MASK UPPER BITS OF ASCII CHARACTER
		MULU		#10,D0		*BEGIN CONVERSION TO BASE 10
		ADD.L		D1,D0		*ADD NEXT DIGIT IN STRING TO RESULT
		BRA		ATOLLP1		*GET NEXT CHARACTER IN ASCII STRING

SETNEG:		MOVE.B		#$FF,D4		*SET NEGATIVE FLAG
		BRA		ATOLLP1		*GET NEXT CHARACTER IN ASCII STRING

ATOLDONE:	CMP.B		#$FF,D4		*WAS IT A NEGATIVE NUMBER?
		BNE		ATOLEND		*NO, SKIP TO END
		NEG.L		D0		*YES, NEGATE THE RESULT

ATOLEND:	MOVEM.L		(A7)+,D1-D7/A0-A6	*RECALL PERTINENT REGISTERS
		RTS				*RETURN RESULT

VALIDSTR:	DC.B		'1234567890',0,0

LDIV:		MOVE.L		D3,D0		*STORE IN D0 FOR NOW
		CLR.W		D3		*CLEAR LOWER WORD OF DIVIDEND
		SWAP		D3		*MOVE UPPER WORD TO LOWER POSITION
		SWAP		D0		*SWAP THE HOLDER
		DIVU		#$A,D3		*PERFORM CONVERSION DIVISION
		MOVE.W		D3,D0		*MOVE MSW QUOTIENT TO D0
		SWAP		D0		*SWAP IT INTO MSW POSITION
		MOVE.W		D0,D3		*MOVE LSW INTO DIVIDEND
		DIVU		#$A,D3		*PERFORM CONVERSION DIVISION
		MOVE.W		D3,D0		*MOVE LSW QUOTIENT TO D0
		CLR.W		D3		*CLEARS LSW QUOTIENT FROM D3
		SWAP 		D3		*SWAPS TOTAL REMAINDER TO LSW OF D3
		RTS				*RETURNS BIG QUOTIENT (D0), REMAINDER (D3)*
* Routines to control the Terminal Display
*
* By: Bradford W. Mott
*


BELL_CODE:	EQU	07


*
* This routine sounds the terminal bell
*

BELL:	MOVE.L	D0,-(A7)		* Save D0 on stack
	MOVE.L	#BELL_CODE,D0		* Get the BELL character
	JSR	PUTC			* Transmit it to the TERMINAL
	MOVE.L	(A7)+,D0		* Restore D0
	RTS				* Return to calling procedure


*
* This routine clears the terminal display and homes the cursor
*

CLEAR:	MOVEM.L	D0/A0,-(A7)		* Save A0&D0 on stack
	MOVE.L	#CLRSTR,A0		* Load address of clear screen string
CLLOOP:	MOVE.B	(A0)+,D0		* Get a character
	BEQ	CLEXT			* EXIT on NULL
	JSR	PUTC			* Put character to display
	BRA	CLLOOP			* Loop
CLEXT:	MOVEM.L	(A7)+,D0/A0		* Restore A0&D0
	RTS				* Return to calling procedure

CLRSTR:	DC.B 27,'[;H',27,'[2J',0,0	* Clear Screen code for VT100



*
* This routine moves the display's cursor to the X/Y coordinates
* specified in the D0 & D1 registers (X=D0,Y=D1)
*

POSITION:	MOVEM.L	D0-D7/A0-A6,-(A7)	* Save D0-D1 & A0 on stack

		ANDI.L	#255,D0		* Clear the unused parts of 
		ANDI.L	#255,D1		* data register

		MOVE.L  #POSSTR+2,A0	* Address of row substring
		DIVU	#10,D1		* Let's convert row number to ASCII
		ADDI.B	#48,D1
		MOVE.B	D1,(A0)+	* Put in POSSTR
		SWAP	D1
		ADDI.B	#48,D1
		MOVE.B	D1,(A0)		* Put in POSSTR

		MOVE.B	#48,POSSTR+5
		CMPI.W	#100,D0
		BLT	POS2

		SUBI.W	#100,D0
		MOVE.B  #49,POSSTR+5

POS2:		MOVE.L	#POSSTR+6,A0	* Address of col substring
		DIVU	#10,D0		* Let's convert column number to ASCII
		ADDI.B	#48,D0
		MOVE.B	D0,(A0)+	* Put in POSSTR
		SWAP	D0
		ADDI.B	#48,D0
		MOVE.B	D0,(A0)		* Put in POSSTR

		MOVE.L	#POSSTR,A0	* Get addr of POSITION string
POSLOOP:	MOVE.B	(A0)+,D0	* Get a character
		BEQ	POSEXT		* Exit on NULL
		JSR 	PUTC		* Send to the terminal
		BRA	POSLOOP		* Loop
	
POSEXT:		MOVEM.L	(A7)+,D0-D7/A0-A6	* Restore registers
		RTS			* Return to calling procedure

		DC.W	0,0,0,0
POSSTR: 	DC.B 27,'[00;000;H',0		* Position Cursor code for VT100
		DC.W	0,0,0,0





*
* This routine gets the sizes of the two matrices to multiply
*

GETSIZE:	MOVEM.L	D0-D7/A0-A6,-(SP)

		JSR CLEAR

		MOVE.L	#GS_TIT,A0		Print title
		JSR	PRINT


GS_GAC:		MOVE.L	#GS_CA,A0
		JSR	PRINT

		MOVE.L	#GS_VALID,A1		Get number of columns
		MOVE.L	#GS_BUFFER,A0
		MOVE.W	#2,D0
		JSR	INPUT

		MOVE.L	#GS_BUFFER,A0
		JSR	ATOL
		MOVE.W	D0,MA_C

		CMPI.W	#10,D0
		BGT	GS_ACA
		CMPI.W	#1,D0
		BLT	GS_ACA
		BRA	GS_GAR

GS_ACA:		MOVE.L	#GS_ERROR,A0
		JSR	PRINT
		JMP	GS_GAC



GS_GAR:		MOVE.L	#GS_RA,A0
		JSR	PRINT

		MOVE.L	#GS_VALID,A1		Get number of columns
		MOVE.L	#GS_BUFFER,A0
		MOVE.W	#2,D0
		JSR	INPUT

		MOVE.L	#GS_BUFFER,A0
		JSR	ATOL
		MOVE.W	D0,MA_R

		CMPI.W	#10,D0
		BGT	GS_ARA
		CMPI.W	#1,D0
		BLT	GS_ARA
		BRA	GS_GBC

GS_ARA:		MOVE.L	#GS_ERROR,A0
		JSR	PRINT
		JMP	GS_GAR



GS_GBC:		MOVE.L	#GS_CB,A0
		JSR 	PRINT

		MOVE.L	#GS_VALID,A1		Get number of columns
		MOVE.L	#GS_BUFFER,A0
		MOVE.W	#2,D0
		JSR	INPUT

		MOVE.L	#GS_BUFFER,A0
		JSR	ATOL
		MOVE.W	D0,MB_C

		CMPI.W	#10,D0
		BGT	GS_BCA
		CMPI.W	#1,D0
		BLT	GS_BCA
		BRA	GS_GBR

GS_BCA:		MOVE.L	#GS_ERROR,A0
		JSR	PRINT
		JMP	GS_GBC



GS_GBR:		MOVE.L	#GS_RB,A0
		JSR 	PRINT

		MOVE.L	#GS_VALID,A1		Get number of columns
		MOVE.L	#GS_BUFFER,A0
		MOVE.W	#2,D0
		JSR	INPUT

		MOVE.L	#GS_BUFFER,A0
		JSR	ATOL
		MOVE.W	D0,MB_R

		CMPI.W	#10,D0
		BGT	GS_BRA
		CMPI.W	#1,D0
		BLT	GS_BRA
		BRA	GS_TEST

GS_BRA:		MOVE.L	#GS_ERROR,A0
		JSR	PRINT
		JMP	GS_GBR


GS_TEST:	MOVE.W	MA_C,D0
		CMP.W	MB_R,D0
		BEQ	GS_OK

		MOVE.L	#GS_BERR,A0
		JSR	PRINT
		JMP	GS_GAC

GS_OK:		MOVEM.L	(A7)+,D0-D7/A0-A6
		RTS


GS_TIT:		DC.B	' Matrix Multiply ',10
		DC.B	'-----------------',10,10
		DC.B	'Please enter the sizes of the matrices to multiply.',10,10,0,0

GS_CA:		DC.B	10,'Columns in matrix A : ',0
GS_RA:		DC.B	10,'Rows in matrix A    : ',0
GS_CB:		DC.B	10,10,'Columns in matrix B : ',0,0
GS_RB:		DC.B	10,'Rows in matrix B    : ',0

GS_VALID:	DC.B	'0123456789',0,0
GS_BUFFER:	DC.B	0,0,0,0,0,0

GS_ERROR:	DC.B	10,'Invalid size!!!',10,0	
GS_BERR:	DC.B	10,'Matrix sizes incompatible!!!!!',10,0,0
*
* This routine multiplies two matrices and stores the resulting
* matrix. ( C=AB )
*
* A0 = Matrix A
* A1 = Matrix B
* A2 = Matrix C
*
* D0 = Matrix A number of rows (i)
* D1 = Matrix A number of columns
* D2 = Matrix B number of rows
* D3 = Matrix B number of columns (j)
*

MATMULT:
	MOVEM.L	D0-D7/A0-A6,-(A7)

	MOVE.W	D3,B_COLS		Save Columns in C

I_LOOP:	MOVE.W	B_COLS,D3
	SUBI.W	#1,D0
	BMI	MATFIN

J_LOOP:	MOVE.L	#0,SUM
	MOVE.W	D1,D4		T Counter
	SUBI.W	#1,D4

	SUB.W	#1,D3
	BMI	I_LOOP

T_LOOP:	MOVE.W	D1,D5		Num Columns in A
	MULU	D0,D5		I * Columns in A
	ADD.W	D4,D5		+ T
	MULU	#2,D5		Words
	MOVEQ	#0,D6
	MOVE.W	(A0,D5),D6	Get A(i,t)

	MOVE.W	B_COLS,D5		Num Columns in B
	MULU	D4,D5		T * Columns in B
	ADD.W	D3,D5		+ J
	MULU	#2,D5
	MOVEQ	#0,D7
	MOVE.W	(A1,D5),D7	Get B(t,j)

	MULS	D6,D7		A(i,t)*B(t,j)
	ADD.L	D7,SUM		add to running Sum

	SUBI.W	#1,D4
	BPL	T_LOOP

	MOVE.W	B_COLS,D5	Num Columns in C
	MULU	D0,D5		I * Num Columns in C
	ADD.W	D3,D5		+ J
	MULU	#4,D5
	MOVE.L	SUM,(A2,D5)	Store C(i,j)

	BRA	J_LOOP
	BRA	I_LOOP

MATFIN:	MOVEM.L	(A7)+,D0-D7/A0-A6
	RTS


SUM:	DC.L	$1
B_COLS:	DC.W	$1
*
*	 MATOUT
**
* D0 contains the number of columns
* D1 contains the number of rows
*
* A0 contains a pointer to a buffer to store matrix elements in
* A1 contains a pointer to the a title string
*
* By: Bradford W. Mott
*


MATOUT:		MOVEM.L	A0-A6/D0-D7,-(A7)	Save registers on stack

		MOVE.L	A0,MO_DATA		Store buffer address
		MOVE.W	D0,MO_ROW		Store num of rows
		MOVE.W	D1,MO_COL		Store num of columns


		JSR	CLEAR			Clear the display

		MOVE.L	#MO_32ON,A0		Turn 132 mode on
		JSR	PRINT

		MOVE.W	#57,D0			X position of cursor
		MOVE.W	#2,D1			Y position of cursor
		JSR	POSITION		Position the Cursor

		MOVE.L	A1,A0			Print the Title String
		JSR	PRINT


		MOVE.L	#MO_Y,A0		Pointer to Y position array
		MOVEQ	#0,D0			Clear LCV (Loop Control Var)

MO_L1:		MOVE.W	#20,D1			Calculate Y position
		SUB.W	MO_ROW,D1
		DIVU	#2,D1
		ADD.W	D0,D1			(20-row)/2+counter
		MOVE.B	D1,0(A0,D0)		Store in array

		ADDQ.W	#1,D0
		CMP.W	MO_ROW,D0
		BNE	MO_L1


		MOVE.L	#MO_X,A0		Pointer to X position array
		MOVEQ	#0,D0			Clear LCV (Loop Control Var)

MO_L2:		MOVE.W	#132,D1			Calculate X position
		MOVE.W  MO_COL,D2
		MULU	#10,D2
		SUB.W	D2,D1
		DIVU	#2,D1
		MOVE.W	D0,D2
		MULU	#10,D2
		ADD.W	D2,D1			(132-col*10)/2+counter*10
		MOVE.B	D1,0(A0,D0)		Store in array

		ADDQ.W	#1,D0
		CMP.W	MO_COL,D0
		BNE	MO_L2


* Draw the Matrix outline

		MOVE.L	#MO_Y,A0		Pointer to Y position array
		MOVEQ	#0,D7			Clear LCV (Loop Control Var)

MO_L3:		MOVE.W	#132,D0			Calculate X position
		MOVE.W	MO_COL,D1
		MULU	#10,D1
		SUB.W	D1,D0
		DIVU	#2,D0
		SUBI.W	#2,D0			D0=(132-col*10)/2-2

		MOVE.B	0(A0,D7),D1		Get Y Position

		JSR	POSITION		Position Cursor
		MOVE.B	#$7C,D0			the "|" character
		JSR	PUTC


		MOVE.W	#132,D0			Calculate X position
		MOVE.W	MO_COL,D1
		MULU	#10,D1
		SUB.W	D1,D0
		DIVU	#2,D0
		ADD.W	D1,D0			D0=(132-col*10)/2+col*10

		MOVE.B	0(A0,D7),D1		Get Y Position

		JSR	POSITION		Position Cursor
		MOVE.B	#$7C,D0			the "|" character
		JSR	PUTC

		ADDQ.W	#1,D7
		CMP.W	MO_ROW,D7
		BNE	MO_L3

* Get the data for each element.

		MOVEQ	#0,D7			LCV for row
		MOVE.L	#MO_X,A3
		MOVE.L	#MO_Y,A4

MO_L4:		MOVEQ	#0,D6			LCV for column

MO_L5:		MOVE.B	0(A3,D6),D0
		MOVE.B	0(A4,D7),D1
		JSR	POSITION


		MOVE.L	#$20202020,MO_OUT  	Set output to all spaces
		MOVE.L	#$20202020,MO_OUT+4
		MOVE.L	#$20200000,MO_OUT+8

		MOVE.L	MO_DATA,A0
		MOVE.W	MO_COL,D0
		MULU	D7,D0			D0=row*col
		MULU	#4,D0			*4

		MOVE.W	D6,D1
		MULU	#4,D1
		ADD.W	D1,D0

		MOVE.L	0(A0,D0),D1		Store entry in Matrix Buffer

		MOVE.L	#MO_OUT+10,A0
		JSR	LTOA

		MOVE.L	#MO_OUT,A0
		JSR	PRINT

		ADDQ.W	#1,D6
		CMP.W	MO_COL,D6
		BNE	MO_L5

		ADDQ.W	#1,D7
		CMP.W	MO_ROW,D7
		BNE	MO_L4


* PRINT PROMPT
		MOVE.W	#0,D0
		MOVE.W	#22,D1
		JSR	POSITION
		MOVE.L	#MO_PROMPT,A0
		JSR	PRINT

MO_LKEY:	JSR	GETC
		CMPI.B	#13,D0
		BNE	MO_LKEY	

* Go back to 80 column mode
		MOVE.L	#MO_32OFF,A0
		JSR	PRINT

* Return to calling routine

		MOVEM.L	(A7)+,A0-A6/D0-D7	Restore Registers

		RTS				Return to caller


MO_OUT:         DC.B    '-1000',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
MO_PROMPT:      DC.B    'Press Return Key...',0
MO_HOFF:        DC.B    27,'[0m',0,0
MO_SSR:         DC.B    27,'[20;23r',0,0
MO_SRR:         DC.B    27,'[0;24r',0
MO_Y:           DC.B    0,0,0,0,0,0,0,0,0,0,0,0
MO_X:           DC.B    0,0,0,0,0,0,0,0,0,0,0,0
MO_DATA:        DC.L    0
MO_ROW:         DC.W    0
MO_COL:         DC.W    0
MO_32ON:	DC.B	27,'[?3h',0
MO_32OFF:	DC.B	27,'[?3l',0


*
*	 MATIN
*
* This routine allows the user to enter the elements of a matrix
*
* D0 contains the number of columns
* D1 contains the number of rows
*
* A0 contains a pointer to a buffer to store matrix elements in
* A1 contains a pointer to the a title string
*
* By: Bradford W. Mott
*


MATIN:		MOVEM.L	A0-A6/D0-D7,-(A7)	Save registers on stack

		MOVE.L	A0,MI_DATA		Store buffer address
		MOVE.W	D0,MI_ROW		Store num of rows
		MOVE.W	D1,MI_COL		Store num of columns

		JSR	CLEAR			Clear the display

		MOVE.W	#33,D0			X position of cursor
		MOVE.W	#2,D1			Y position of cursor
		JSR	POSITION		Position the Cursor

		MOVE.L	A1,A0			Print the Title String
		JSR	PRINT


		MOVE.L	#MI_Y,A0		Pointer to Y position array
		MOVEQ	#0,D0			Clear LCV (Loop Control Var)

MI_L1:		MOVE.W	#20,D1			Calculate Y position
		SUB.W	MI_ROW,D1
		DIVU	#2,D1
		ADD.W	D0,D1			(20-row)/2+counter
		MOVE.B	D1,0(A0,D0)		Store in array

		ADDQ.W	#1,D0
		CMP.W	MI_ROW,D0
		BNE	MI_L1


		MOVE.L	#MI_X,A0		Pointer to X position array
		MOVEQ	#0,D0			Clear LCV (Loop Control Var)

MI_L2:		MOVE.W	#80,D1			Calculate X position
		MOVE.W  MI_COL,D2
		MULU	#6,D2
		SUB.W	D2,D1
		DIVU	#2,D1
		MOVE.W	D0,D2
		MULU	#6,D2
		ADD.W	D2,D1			(80-col*6)/2+counter*6
		MOVE.B	D1,0(A0,D0)		Store in array

		ADDQ.W	#1,D0
		CMP.W	MI_COL,D0
		BNE	MI_L2


* Draw the Matrix outline

		MOVE.L	#MI_Y,A0		Pointer to Y position array
		MOVEQ	#0,D7			Clear LCV (Loop Control Var)

MI_L3:		MOVE.W	#80,D0			Calculate X position
		MOVE.W	MI_COL,D1
		MULU	#6,D1
		SUB.W	D1,D0
		DIVU	#2,D0
		SUBI.W	#2,D0			D0=(80-col*6)/2-2

		MOVE.B	0(A0,D7),D1		Get Y Position

		JSR	POSITION		Position Cursor
		MOVE.B	#$7C,D0			the "|" character
		JSR	PUTC


		MOVE.W	#80,D0			Calculate X position
		MOVE.W	MI_COL,D1
		MULU	#6,D1
		SUB.W	D1,D0
		DIVU	#2,D0
		ADD.W	D1,D0			D0=(80-col*6)/2+col*6

		MOVE.B	0(A0,D7),D1		Get Y Position

		JSR	POSITION		Position Cursor
		MOVE.B	#$7C,D0			the "|" character
		JSR	PUTC

		ADDQ.W	#1,D7
		CMP.W	MI_ROW,D7
		BNE	MI_L3



* Set the Display scrolling region

		MOVE.L	#MI_SSR,A0		Addr of string
		JSR	PRINT


* Get the data for each element.

		MOVEQ	#0,D7			LCV for row
		MOVE.L	#MI_X,A3
		MOVE.L	#MI_Y,A4

MI_L4:		MOVEQ	#0,D6			LCV for column

MI_L5:		MOVE.B	0(A3,D6),D0
		MOVE.B	0(A4,D7),D1
		JSR	POSITION

		MOVE.L	#MI_HON,A0		Turn Highlight on
		JSR	PRINT

		MOVE.L	#MI_SPACE,A0		Print Space
		JSR	PRINT

		MOVE.L	#MI_HOFF,A0		Turn Highlight off
		JSR	PRINT

		MOVE.W	#0,D0
		MOVE.W	#23,D1
		JSR	POSITION

MI_AGAIN:	MOVE.L	#MI_PROMPT,A0		Print prompt
		JSR	PRINT

		MOVE.L	#MI_INPUT,A0
		MOVE.L	#MI_VALID,A1
		MOVE.L	#5,D0
		JSR	INPUT			INPUT the number

		MOVE.L	#10,D0			NEWLINE
		JSR	PUTC

		MOVE.L	#MI_INPUT,A0
		JSR	ATOL			Convert ASCII to Long INT
		MOVE.L	D0,MI_INN

		CMPI.W	#1000,D0		Make sure number is right
		BGT	MI_ER			size
		CMPI.W	#-1000,D0
		BLT	MI_ER
		
		BRA	MI_NER

MI_ER:		MOVE.L	#MI_ERROR,A0		Print Error Message
		JSR	PRINT
		JMP	MI_AGAIN		Try Again!!!

MI_NER:		MOVE.L	#$20202020,MI_OUT  	Set output to all spaces
		MOVE.L	#$20000000,MI_OUT+4
		MOVE.L	#MI_OUT+6,A0
		MOVEQ	#0,D1
		MOVE.L	MI_INN,D1
		JSR	LTOA

       		MOVE.B 0(A3,D6),D0
		MOVE.B 0(A4,D7),D1
		JSR	POSITION

		MOVE.L	#MI_OUT,A0		Print Entry
		JSR	PRINT

		MOVE.L	MI_DATA,A0
		MOVE.W	MI_COL,D0
		MULU	D7,D0			D0=row*col
		MULU	#2,D0			*2

		MOVE.W	D6,D1
		MULU	#2,D1
		ADD.W	D1,D0

		MOVE.L	MI_INN+2,0(A0,D0)		Store entry in Matrix Buffer

		ADDQ.W	#1,D6
		CMP.W	MI_COL,D6
		BNE	MI_L5

		ADDQ.W	#1,D7
		CMP.W	MI_ROW,D7
		BNE	MI_L4

* Reset the terminal's scrolling region

		MOVE.L	#MI_SRR,A0
		JSR	PRINT


* Return to calling routine

		MOVEM.L	(A7)+,A0-A6/D0-D7	Restore Registers

		RTS				Return to caller


MI_INN:         DC.L   0,0,0,0,0
MI_OUT:         DC.B    '-1000',0,0,0,0,0,0,0,0,0
MI_VALID:       DC.B    '0123456789-',0
MI_INPUT:       DC.B    0,0,0,0,0,0,0,0,0,0,0,0,0,0
MI_PROMPT:      DC.B    'Enter Element: ',0
MI_ERROR:       DC.B    '*** Problem with entry!!!!',10,10,0,0
MI_SPACE:       DC.B    '     ',0,0
MI_HON:         DC.B    27,'[5;7m',0,0
MI_HOFF:        DC.B    27,'[0m',0,0
MI_SSR:         DC.B    27,'[20;23r',0,0
MI_SRR:         DC.B    27,'[0;24r',0
MI_Y:           DC.B    0,0,0,0,0,0,0,0,0,0,0,0
MI_X:           DC.B    0,0,0,0,0,0,0,0,0,0,0,0
MI_DATA:        DC.L    0
MI_ROW:         DC.W    0
MI_COL:         DC.W    0


EXIT:		MOVE.L		#QUITMSG,A0	*INITIALIZE QUIT PROMPT
		JSR 		PRINT		*PRINT QUIT PROMPT
		JSR		GETC		*GET RESPONSE
		CMP.B		#$59,D0		*A VALID CHARACTER?
		BEQ		QSTRT		*YES, START OVER
		CMP.B		#$79,D0		*A VALID CHARACTER?
		BEQ		QSTRT		*YES, START OVER
		CMP.B		#$4E,D0		*A VALID CHARACTER?
		BEQ		QFIN		*NO, QUIT PROGRAM
		CMP.B		#$6E,D0		*A VALID CHARACTER?
		BEQ		QFIN		*NO, QUIT PROGRAM
		BRA		EXIT		*NOT A VALID CHARACTER

QSTRT:		BRA		MAIN		*RE-EXECUTE MAIN PROGRAM

QFIN:		MOVE.L		#ENDMSG,A0	*INITIALIZE FINAL MESSAGE
		JSR		PRINT		*PRINT FINAL MESSAGE
NOMOE:		BRA		NOMOE		*INFINITE LOOP

QUITMSG:	DC.B		10,'Multiply Another Set? <Y,y,N,n>...',0,0
ENDMSG:		DC.B		10,10,'THANK YOU, GOODBYE..',0


