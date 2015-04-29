###############################################################################
#
# Makefile - Sim68000 Makefile
#
# Bradford W. Mott
# July 3,1994
#
# @DJ@ Gdbsock device added.
#
###############################################################################
# $Id:$
###############################################################################

LIBRARY = libdevices.a

## List of object files that need to be made
OBJS = RAM.o M68681.o DeviceRegistry.o Gdbsock.o Timer.o

TCL = RAM.tcl M68681.tcl Gdbsock.tcl Timer.tcl
SCR = RAM.scr M68681.scr Gdbsock.scr Timer.scr

all: $(LIBRARY)

install:

$(LIBRARY): $(OBJS)
	$(AR) $(AR_CREATE_FLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

clean:
	rm -f *.o $(LIBRARY)

.SUFFIXES: .cxx .a .tcl .scr

.tcl.scr:
	tcl2scr $*.tcl > $*.scr

.cxx.o:
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $*.cxx


DeviceRegistry.o: $(SCR)
