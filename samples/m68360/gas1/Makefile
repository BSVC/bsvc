#
# Makefile -*- BASH -*-
#
# USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
# WITH NO WARRANTY FOR ANY PURPOSE.
#

#-------------------------------------------------------------------------
CPU=68020
#-------------------------------------------------------------------------

AS=vxworks68-as
ASFLAGS=-m$(CPU)
LD=vxworks68-ld
LDFLAGS=-M 
RM=/bin/rm -f
GAL=python $(BSVC20)/etc/gal.py

#-------------------------------------------------------------------------
.SUFFIXES: .s .glis

PROGRAM=test1
OBJECTS=boot1.o test1.o duart.o
ASMSRC=boot1.s test1.s duart.s
ASMGLIS=boot1.glis test1.glis duart.glis
MAP=$(PROGRAM).map
#-------------------------------------------------------------------------
.s.glis:
	$(AS) -a $(ASFLAGS) -o /dev/null $< > $@

#-------------------------------------------------------------------------
all: $(PROGRAM).H68 $(PROGRAM).LIS

$(PROGRAM).H68 $(MAP): $(OBJECTS)
	$(LD) $(LDFLAGS) -T $(PROGRAM).cmd  > $(MAP)

clean:
	$(RM) *.o $(PROGRAM).H68 $(PROGRAM).LIS $(MAP)
	$(RM) *.lis *.glis CPU32.debug
	
$(PROGRAM).LIS: $(ASMGLIS) $(MAP)
	$(GAL) $(MAP) $(ASMGLIS)
	
