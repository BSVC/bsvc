###############################################################################
#
# Makefile.common - System Independent Makefile
#
# Bradford W. Mott
# July 3,1994
#
###############################################################################
# $Id:$
###############################################################################

## Library to build
LIBRARY = libfw.a

## List of object files that need to be made
OBJS = AddressSpace.o BasicCPU.o BasicDevice.o BasicDeviceRegistry.o \
       BreakpointList.o Event.o RegInfo.o StatInfo.o Time.o Tools.o \
       Interface.o

all: $(LIBRARY)

install:

$(LIBRARY): $(OBJS)
	$(AR) $(AR_CREATE_FLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

.SUFFIXES: .cxx .a

.cxx.o:
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $*.cxx

clean:
	rm -f *.o $(LIBRARY)


