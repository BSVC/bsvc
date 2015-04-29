###############################################################################
#
# Makefile - Assemblers Makefile
#
# Bradford W. Mott
# July 8,1994
#
###############################################################################
## $Id:$
###############################################################################

## List of subdirectories that need to be built
SUBDIRS = 68kasm

SECTION = Assemblers

all: subdirectories

subdirectories:
	@for dir in $(SUBDIRS); \
	do \
		echo ""; \
		echo "------ Building: $(SECTION)/$$dir"; \
		cd $$dir; \
		make $(MAKEPARMS) MAKEPARMS="$(MAKEPARMS)" all; \
		if [ $$? -eq 0 ] ; \
		then \
			cd ..; \
		else \
			exit 1; \
		fi \
	done

install:
	@for dir in $(SUBDIRS); \
	do \
		echo ""; \
		echo "------ Installing: $(SECTION)/$$dir"; \
		cd $$dir; \
		make $(MAKEPARMS) MAKEPARMS="$(MAKEPARMS)" install; \
		if [ $$? -eq 0 ] ; \
		then \
			cd ..; \
		else \
			exit 1; \
		fi \
	done

clean:
	@for dir in $(SUBDIRS); \
	do \
		echo ""; \
		echo "------ Cleaning: $(SECTION)/$$dir"; \
		cd $$dir; \
		make $(MAKEPARMS) MAKEPARMS="$(MAKEPARMS)" clean; \
		if [ $$? -eq 0 ] ; \
		then \
			cd ..; \
		else \
			exit 1; \
		fi \
	done


