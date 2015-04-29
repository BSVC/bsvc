
	ORG	$0 
	DC.L	$8000           Stack pointer value after a reset
	DC.L	START           Program counter value after a reset


	ORG	$2000		Start at location 2000 Hex

START	CLR.W	SUM		Clear variable SUM
	MOVE.W	COUNT,D0	Load COUNT value
LOOP	ADD.W	D0,SUM		ADD D0 to SUM
	SUB.W	#1,D0		Decrement counter
	BNE	LOOP		Loop if counter not zero
	BREAK			Tell the simulator to BREAK if we're running

SUM	DS.W	1		Reserve one word for SUM
COUNT	DC.W	25		Initial value for COUNT

