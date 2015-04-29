###############################################################################
#
# Makefile - Tools Makefile
#
# Bradford W. Mott
# July 3,1994
#
###############################################################################
# $Id:$
###############################################################################

## List of subdirectories that need to be built
SUBDIRS = xtermpipe

SECTION = Tools

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

