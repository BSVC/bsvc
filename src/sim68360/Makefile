###############################################################################
#
# Makefile - sim68000 Makefile
#
# Bradford W. Mott
# July 3,1994
#
# Adapted to 68360 - DJ - 11/1996
# - sim68000 renamed sim68360.
# - $(PROGRAM_LIBS) added to program dependencies.
#
###############################################################################
# $Id:$
###############################################################################

## List of subdirectories that need to be built
SUBDIRS = cpu devices loader

PROGRAM = sim68360
PROGRAM_LIBS = ./cpu/libcpu.a ./devices/libdevices.a \
	./loader/libloader.a ../Framework/libfw.a

## List of object files that need to be made
OBJS = main.o

all: subdirectories $(PROGRAM)

$(PROGRAM): $(OBJS) $(PROGRAM_LIBS)
	$(CXX) -o $(PROGRAM) $(OBJS) $(PROGRAM_LIBS) $(LIBS) 

subdirectories:
	@for dir in $(SUBDIRS); \
	do \
		echo ""; \
		echo "------ Building: $(PROGRAM)/$$dir"; \
		cd $$dir; \
		make $(MAKEPARMS) MAKEPARMS="$(MAKEPARMS)" all; \
		if [ $$? -eq 0 ] ; \
		then \
			cd ..; \
		else \
			exit 1; \
		fi \
	done

install: $(PROGRAM)
	@for dir in $(SUBDIRS); \
	do \
		echo ""; \
		echo "------ Installing: $$dir"; \
		cd $$dir; \
		make $(MAKEPARMS) MAKEPARMS="$(MAKEPARMS)" install; \
		if [ $$? -eq 0 ] ; \
		then \
			cd ..; \
		else \
			exit 1; \
		fi \
	done
	$(INSTALL) $(PROGRAM) $(INSTALL_DIR)
	$(STRIP) $(INSTALL_DIR)/$(PROGRAM)

clean:
	@for dir in $(SUBDIRS); \
	do \
		echo ""; \
		echo "------ Cleaning: $(PROGRAM)/$$dir"; \
		cd $$dir; \
		make $(MAKEPARMS) MAKEPARMS="$(MAKEPARMS)" clean; \
		if [ $$? -eq 0 ] ; \
		then \
			cd ..; \
		else \
			exit 1; \
		fi \
	done; \
	rm -f *.o $(PROGRAM)


.SUFFIXES: .cxx .a

.cxx.o:
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $*.cxx


