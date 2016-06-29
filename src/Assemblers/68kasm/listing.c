/******************************************************************************
 *
 *		LISTING.C
 *		Listing File Routines for 68000 Assembler
 *
 *    Function: initList()
 *		Opens the specified listing file for writing. If the
 *		file cannot be opened, then the routine prints a
 *		message and exits.
 *
 *		listLine()
 *		Writes the current listing line to the listing file. If
 *		the line is not a continuation, then the routine
 *		includes the source line as the last part of the
 *		listing line. If an error occurs during the writing,
 *		the routine prints a message and exits.
 *
 *		listLoc()
 *		Starts the process of assembling a listing line by
 *		printing the location counter value into listData and
 *		initializing listPtr.
 *
 *		listObj()
 *		Prints the data whose size and value are specified in
 *		the object field of the current listing line. Bytes are
 *		printed as two digits, words as four digits, and
 *		longwords as eight digits, and a space follows each
 *		group of digits.
 *		     If the item to be printed will not fit in the
 *		object code field, one of two things will occur. If
 *		cexFlag is TRUE, then the current listing line will be
 *		output to the listing file and the data will be printed
 *		on a continuation line. Otherwise, elipses ("...") will
 *		be printed to indicate the omission of values from the
 *		listing, and the data will not be added to the file.
 *
 *	 Usage: initList(name)
 *		char *name;
 *
 *		listLine()
 *
 *		listLoc()
 *
 *		listObj(data, size)
 *		int data, size;
 *
 *      Author: Paul McKee
 *		ECE492    North Carolina State University
 *
 *        Date:	12/13/86
 *
 *   Copyright 1990-1991 North Carolina State University. All Rights Reserved.
 *
 *****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"

/* Declarations of global variables */
extern int loc;
extern char pass2, cexFlag, continuation;
extern char line[256];
extern FILE *listFile;
extern int lineNum;

static char listData[49];	// Buffer in which listing lines are assembled
extern char *listPtr;		// Pointer to above buffer (this pointer is
				// global because it is actually manipulated
				// by equ() and set() to put specially formatted
				// information in the listing)

void
initList(char *name)
{

	listFile = fopen(name, "w");
	if (!listFile) {
		puts("Can't open listing file");
		exit(1);
	}
}

void
listLine(void)
{

	fprintf(listFile, "%-41.41s", listData);
	if (!continuation)
		fprintf(listFile, "%5d  %s", lineNum, line);
	else
		putc('\n', listFile);
	if (ferror(listFile)) {
		fputs("Error writing to listing file\n", stderr);
		exit(1);
	}
}


void
listLoc(void)
{
	sprintf(listData, "%08X  ", loc);
	listPtr = listData + 10;
}

void
listObj(int data, int size)
{
	if (!cexFlag && (listPtr - listData + size > 40)) {
		strcpy(listData + ((size == WORD) ? 35 : 37), "...");
		return;
	}
	if (cexFlag && (listPtr - listData + size > 40)) {
		listLine();
		strcpy(listData, "          ");
		listPtr = listData + 10;
		continuation = TRUE;
	}
	switch (size) {
	case BYTE:
		sprintf(listPtr, "%02X ", data & 0xFF);
		listPtr += 3;
		break;
	case WORD:
		sprintf(listPtr, "%04X ", data & 0xFFFF);
		listPtr += 5;
		break;
	case LONG:
		sprintf(listPtr, "%08X ", data);
		listPtr += 9;
		break;
	default:
		printf("LISTOBJ: INVALID SIZE CODE!\n");
		exit(1);
	}
}
