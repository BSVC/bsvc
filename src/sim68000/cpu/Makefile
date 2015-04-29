###############################################################################
#
# Makefile - Sim68000 Makefile
#
# Bradford W. Mott
# July 3,1994
#
###############################################################################
# $Id:$
###############################################################################

LIBRARY = libcpu.a

## List of object files that need to be made
OBJS = m68000.o decode.o exec.o

all: $(LIBRARY)

install:

$(LIBRARY): $(OBJS)
	$(AR) $(AR_CREATE_FLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

decode.o: m68000DecodeTable.hxx

instruction: instruction.c
	$(CC) $(INCLUDES) -o instruction instruction.c

m68000DecodeTable.hxx: instruction instruction.list
	$(PROJECT_BASE)/src/sim68000/cpu/instruction  

clean:
	rm -f *.o $(LIBRARY) instruction


.SUFFIXES: .cxx .a

.cxx.o:
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $*.cxx


