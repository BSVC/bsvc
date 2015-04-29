#
#  FILE:	Makefile
#  PURPOSE:	Create the simulator assembler 
#  INVOKATION:	make
#
#  Copyright 1990-1991 North Carolina State University. All Rights Reserved.
#
#  Send suggestions or comments to:
#	Paul Franzon  paulf@csl.ncsu.edu
#   or  Tom Miller  tkm@csl.ncsu.edu

#
#  History:  10/28/91 (Tan Phan)        -- Added -DTAN_DEBUG flag
#                                          to fix segmentation bug.
#                                          (due to mis-definition)
#                                       -- Debugged for Ultrix
#
#            07/08/94 (Bradford Mott)   -- Modified object.c and listing.c
#                                          so they would compile under Linux
#                                       -- Modified to be part of the BSVC
#                                          package

#  This is a list of all the files and their dependency order as listed in
#  the file asm.bld.  I tried to put them together but can not get it to
#  work.  I do not know if some changes need to be made in the code for
#  this to work on UNIX or not.   

PROGRAM = 68kasm

all: $(PROGRAM)

$(PROGRAM): asm.h movem.o object.o globals.o directive.o build.o \
		insttable.o error.o symbol.o eval.o opparse.o codegen.o \
		listing.o instlookup.o assemble.o main.o include.o
	$(CC) movem.o object.o globals.o directive.o build.o \
		insttable.o error.o symbol.o eval.o opparse.o \
		codegen.o listing.o instlookup.o assemble.o main.o include.o \
		-o $(PROGRAM)

clean:: 
	rm -f *.o $(PROGRAM)

install: $(PROGRAM)
	$(INSTALL) $(PROGRAM) $(INSTALL_DIR)
	$(STRIP) $(INSTALL_DIR)/$(PROGRAM)

	
main.o		: main.c asm.h
include.o	: main.c 
assemble.o	: assemble.c asm.h
build.o		: build.c asm.h
codegen.o	: codegen.c asm.h
directive.o	: directive.c asm.h
error.o		: error.c asm.h
eval.o		: eval.c asm.h
globals.o	: globals.c asm.h
instlookup.o	: instlookup.c asm.h
insttable.o	: insttable.c asm.h
listing.o	: listing.c asm.h
movem.o		: movem.c asm.h
object.o	: object.c asm.h
opparse.o	: opparse.c asm.h
symbol.o	: symbol.c asm.h

.c.o:
	$(CC) $(CCFLAGS) -c $*.c

