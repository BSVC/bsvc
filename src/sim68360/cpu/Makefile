###############################################################################
#
# Makefile - Sim68000 Makefile
#
# Bradford W. Mott
# July 3,1994
#
# Adapted to 68360 - DJ - 11/1996
# - m68000 renamed cpu32
#
###############################################################################
# $Id:$
###############################################################################

LIBRARY = libcpu.a

## List of object files that need to be made
OBJS = cpu32.o decode.o exec.o

all: $(LIBRARY)

install:

$(LIBRARY): $(OBJS)
	$(AR) $(AR_CREATE_FLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

decode.o: cpu32DecodeTable.hxx

instruction: instruction.c
	$(CC) $(INCLUDES) -o instruction instruction.c

cpu32DecodeTable.hxx: instruction instruction.list
	$(PROJECT_BASE)/src/sim68360/cpu/instruction  

clean:
	rm -f *.o $(LIBRARY) instruction


.SUFFIXES: .cxx .a

.cxx.o:
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $*.cxx


