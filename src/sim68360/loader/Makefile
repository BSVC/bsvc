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

LIBRARY = libloader.a

## List of object files that need to be made
OBJS = Loader.o

all: $(LIBRARY)

install:

$(LIBRARY): $(OBJS)
	$(AR) $(AR_CREATE_FLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

clean:
	rm -f *.o $(LIBRARY)


.SUFFIXES: .cxx .a

.cxx.o:
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $*.cxx


