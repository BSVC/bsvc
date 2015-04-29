| boot1.s
| Set the initial stack and reset vector
|
| DJ (c) 1996
| 
| USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
| WITH NO WARRANTY FOR ANY PURPOSE.
|


.equ STACK,0xF000
.equ START,0x1000

| this is absolute address 0 (see linker command file)
.data
.long STACK			| Vector 0 (SSP)
.long START			| Vector - (PC)
.space 1016			| (256-2)*4 (space for exception vectors)
.even
