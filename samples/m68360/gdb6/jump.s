
|
| jump.s
|
| DJ 1996/11
| 
| USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
| WITH NO WARRANTY FOR ANY PURPOSE.
|
| Assembly language functions providing the 'setjmp/longjmp'
| functionality
|
| Implements:
| - _setjmp
| - _longjmp

| jump buffer is
| octet 0 to 4: a0
| octet 4 to 60: a1-a7,d1-d7 (movem)
| octet 60 to 64: tmp value (return value for longjmp)
| octet 64 to 67: PC

.global _setjmp
_setjmp:
	| use freely D0
	| get jump-buffer address from stack in a0
	| stack contains:
	| 0-4 bytes: return address
	| 4-8 bytes: parameter
	| save a0 in d0
	movel a0,d0
	movel (4,a7),a0
	| save registers
	movel d0,(a0)
	moveml d1-d7/a1-a7,(4,a0) 
	| save PC (return address is in the stack)
	movel (a7),a4
	movel a4,(64,a0)
	
	| return 0
	movel #0,d0
	rts	

.global _longjmp
_longjmp:
	| use freely D0
	| get value to be returned from stack	
	movel (8,a7),d0	
	| get jump-buffer address from stack in a0
	movel (4,a7),a0
	| put return value in jump-buffer
	movel d0,(60,a0)
	| restore registers (especially SP)
	moveml (4,a0),d1-d7/a1-a7
	| restore PC
	movel (64,a0),d0
	movel d0,(a7)
	| restore d0
	movel (60,a0),d0
	| restore a0
	movel (a0),a0
	| return value in d0 has been set above.	
	rts	

