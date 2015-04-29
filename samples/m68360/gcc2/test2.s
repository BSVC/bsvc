#NO_APP
gcc2_compiled.:
___gnu_compiled_c:
.stabs "/home/jorand/bsvc-2.0/samples/cpu32/gcc2/",100,0,0,Ltext0
.stabs "test2.c",100,0,0,Ltext0
.text
Ltext0:
.stabs "int:t1=r1;-2147483648;2147483647;",128,0,0,0
.stabs "char:t2=r2;0;127;",128,0,0,0
.stabs "long int:t3=r1;-2147483648;2147483647;",128,0,0,0
.stabs "unsigned int:t4=r1;0;-1;",128,0,0,0
.stabs "long unsigned int:t5=r1;0;-1;",128,0,0,0
.stabs "long long int:t6=r1;01000000000000000000000;0777777777777777777777;",128,0,0,0
.stabs "long long unsigned int:t7=r1;0000000000000;01777777777777777777777;",128,0,0,0
.stabs "short int:t8=r1;-32768;32767;",128,0,0,0
.stabs "short unsigned int:t9=r1;0;65535;",128,0,0,0
.stabs "signed char:t10=r1;-128;127;",128,0,0,0
.stabs "unsigned char:t11=r1;0;255;",128,0,0,0
.stabs "float:t12=r1;4;0;",128,0,0,0
.stabs "double:t13=r1;8;0;",128,0,0,0
.stabs "long double:t14=r1;12;0;",128,0,0,0
.stabs "complex int:t15=s8real:1,0,32;imag:1,32,32;;",128,0,0,0
.stabs "complex float:t16=r16;4;0;",128,0,0,0
.stabs "complex double:t17=r17;8;0;",128,0,0,0
.stabs "complex long double:t18=r18;12;0;",128,0,0,0
.stabs "void:t19=19",128,0,0,0
	.even
.globl _puts
_puts:
	.stabd 68,0,20
	link a6,#0
	.stabd 68,0,21
	nop
L2:
	movel a6@(8),a0
	tstb a0@
	jne L4
	jra L3
L4:
	movel a6@(8),a0
	moveb a0@,d0
	extbl d0
	movel d0,sp@-
	addql #1,a6@(8)
	jbsr _DUART_PUTC
	addqw #4,sp
	jra L2
L3:
	.stabd 68,0,22
L1:
	unlk a6
	rts
.stabs "puts:F19",36,0,20,_puts
.stabs "s:p20=*2",160,0,19,8
LC0:
	.ascii "Hello world!\12\0"
LC1:
	.ascii "Done.\12\0"
	.even
.globl _main
_main:
	.stabd 68,0,28
	link a6,#0
	jbsr ___main
	.stabd 68,0,29
	jbsr _DUART_INIT
	.stabd 68,0,30
	pea LC0
	jbsr _puts
	addqw #4,sp
	.stabd 68,0,31
	jbsr _DUART_GETC
	.stabd 68,0,32
	pea LC1
	jbsr _puts
	addqw #4,sp
	.stabd 68,0,33
L6:
	jra L8
	jra L7
L8:
	jra L6
L7:
	.stabd 68,0,34
L5:
	unlk a6
	rts
.stabs "main:F19",36,0,28,_main
