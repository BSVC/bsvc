/******************************************************************************
 *
 *		CODE.C
 *		Code Generation Routines for 68000 Assembler
 *
 *    Function: output()
 *		Places the data whose size and value are specified onto
 *		the output stream at the current location contained in
 *		global varible loc. That is, if a listing is being
 *		produced, it calls listObj() to print the data in the
 *		object code field of the current listing line; if an
 *		object file is being produced, it calls outputObj() to
 *		output the data in the form of S-records. 
 *
 *		effAddr()
 *		Computes the 6-bit effective address code used by the
 *		68000 in most cases to specify address modes. This code
 *		is returned as the value of effAddr(). The desired
 *		addressing mode is determined by the field of the
 *		opDescriptor which is pointed to by the operand
 *		argument. The lower 3 bits of the output contain the
 *		register code and the upper 3 bits the mode code. 
 *
 *		extWords()
 *		Computes and outputs (using output()) the extension 
 *		words for the specified operand. The generated
 *		extension words are determined from the data contained
 *		in the opDescriptor pointed to by the op argument and
 *		from the size code of the instruction, passed in 
 *		the size argument. The errorPtr argument is used to
 *		return an error code by the standard mechanism. 
 *
 *	 Usage: output(data, size)
 *		int data, size;
 *
 *		effAddr(operand)
 *		opDescriptor *operand;
 *
 *		extWords(op, size, errorPtr)
 *		opDescriptor *op;
 *		int size, *errorPtr;
 *
 *      Author: Paul McKee
 *		ECE492    North Carolina State University
 *
 *        Date:	12/13/86
 *
 *   Copyright 1990-1991 North Carolina State University. All Rights Reserved.
 *
 ******************************************************************************
 * $Id: codegen.c,v 1.1 1996/08/02 14:40:56 bwmott Exp $
 *****************************************************************************/


#include <stdio.h>
#include "asm.h"

extern int loc;
extern char pass2;
extern FILE *listFile;

extern char listFlag;	/* True if a listing is desired */
extern char objFlag;	/* True if an object code file is desired */


output(data, size)
int data, size;
{
/*	switch (size) {
		case BYTE : printf("Output: Byte           %02X put into output stream at location %08X\n", data, loc); break;
		case WORD : printf("Output: Word         %04X put into output stream at location %08X\n", data, loc); break;
		case LONG : printf("Output: Longword %08X put into output stream at location %08X\n", data, loc); break;
		default   : printf("OUTPUT: INVALID SIZE CODE!\n"); exit();
		}  */
	if (listFlag)
		listObj(data, size);
	if (objFlag)
		outputObj(loc, data, size);
}


effAddr(operand)
opDescriptor *operand;
{
	switch (operand->mode) {
		case DnDirect	: return 0x00 | operand->reg; break;
		case AnDirect	: return 0x08 | operand->reg; break;
		case AnInd	: return 0x10 | operand->reg; break;
		case AnIndPost	: return 0x18 | operand->reg; break;
		case AnIndPre	: return 0x20 | operand->reg; break;
		case AnIndDisp	: return 0x28 | operand->reg; break;
		case AnIndIndex : return 0x30 | operand->reg; break;
		case AbsShort	: return 0x38; break;
		case AbsLong	: return 0x39; break;
		case PCDisp	: return 0x3A; break;
		case PCIndex	: return 0x3B; break;
		case Immediate	: return 0x3C; break;
		default		: printf("INVALID EFFECTIVE ADDRESSING MODE!\n"); exit (0);
		}
}


extWords(op, size, errorPtr)
opDescriptor *op;
int size, *errorPtr;
{
int disp;

	switch (op->mode) {
		case DnDirect	: 
		case AnDirect	: 		/* These modes take	*/
		case AnInd	: 		/* no extension words.	*/
		case AnIndPost	: 
		case AnIndPre	: break;
		case AnIndDisp	: 
		case PCDisp	: if (pass2) {
					disp = op->data;
					if (op->mode == PCDisp)
						disp -= loc;
					output(disp & 0xFFFF, WORD);
					if (disp < -32768 || disp > 32767)
						NEWERROR(*errorPtr, INV_DISP);
					}
				  loc += 2;
				  break;
		case AnIndIndex : 
		case PCIndex	: if (pass2) {
					disp = op->data;
					if (op->mode == PCIndex)
						disp -= loc;
					output(((op->size == LONG) ? 0x800 : 0)
					       | (op->index << 12) | (disp & 0xFF), WORD);
					if (disp < -128 || disp > 127)
						NEWERROR(*errorPtr, INV_DISP);
					}
				  loc += 2;
				  break;
		case AbsShort	: if (pass2) {
					output(op->data & 0xFFFF, WORD);
					if (op->data < -32768 || op->data > 32767)
						NEWERROR(*errorPtr, INV_ABS_ADDRESS);
					}
				  loc += 2;
				  break;
		case AbsLong	: if (pass2)
					output(op->data, LONG);
				  loc += 4;
				  break;
		case Immediate	: if (!size || size == WORD) {
					if (pass2) {
						output(op->data & 0xFFFF, WORD);
						if (op->data < -32768 || op->data > 65535)
							NEWERROR(*errorPtr, INV_16_BIT_DATA);
						}
					loc += 2;
					}
				  else if (size == BYTE) {
					if (pass2) {
						output(op->data & 0xFF, WORD);
						if (op->data < -32768 || op->data > 32767)
							NEWERROR(*errorPtr, INV_8_BIT_DATA);
						}
					loc += 2;
					}
				  else if (size == LONG) {
					if (pass2)
						output(op->data, LONG);
					loc += 4;
					}
				  break;
		default		: printf("INVALID EFFECTIVE ADDRESSING MODE!\n"); exit();
		}
}
