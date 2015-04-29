###############################################################################
#
# Makefile - xtermpipe
#
# Bradford W. Mott
# July 23,1994
#
###############################################################################
# $Id:$
###############################################################################

PROGRAM = xtermpipe
PROGRAM_LIBS =

## List of object files that need to be made
OBJS = xtermpipe.o

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CXX) -o $(PROGRAM) $(OBJS) $(PROGRAM_LIBS) $(LIBS)

install: $(PROGRAM)
	$(INSTALL) $(PROGRAM) $(INSTALL_DIR)
	$(STRIP) $(INSTALL_DIR)/$(PROGRAM)

clean:
	rm -f *.o $(PROGRAM)

.SUFFIXES: .cxx .a

.cxx.o:
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $*.cxx


