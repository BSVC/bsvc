#
# Makefile -*- BASH -*-
#
# DJ (c) 1996
#
# USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
# WITH NO WARRANTY FOR ANY PURPOSE.
#

#-------------------------------------------------------------------------
CPU=68020
#-------------------------------------------------------------------------

CC=vxworks68-gcc
CFLAGS=-g -Wall -m$(CPU) -mnobitfield -I. -U__HAVE_68881__
AS=vxworks68-as
ASFLAGS=-m$(CPU)
LD=vxworks68-ld
LDFLAGS=-M 
RM=/bin/rm -f
GAL=python $(BSVC20)/etc/gal.py

#-------------------------------------------------------------------------
LIBS=-L$(M68K_LIB) -lgcc

.SUFFIXES: .cxx .s .glis

PROGRAM=test4
OBJECTS=boot2.o test4.o duart.o
ASMSRC=boot2.s test4.s duart.s
ASMGLIS=boot2.glis test4.glis duart.glis
MAP=$(PROGRAM).map
#-------------------------------------------------------------------------
.cxx.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.cxx.s:
	$(CC) -c -S $(CFLAGS) -o $@ $<

.c.s:
	$(CC) -c -S $(CFLAGS) -o $@ $<

.s.glis:
	$(AS) -a $(ASFLAGS) -o /dev/null $< > $@

#-------------------------------------------------------------------------
all: $(PROGRAM).H68 $(PROGRAM).LIS

$(PROGRAM).H68 $(MAP): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -T $(PROGRAM).cmd $(LIBS) > $(MAP)

clean:
	$(RM) *.o $(PROGRAM).H68 $(PROGRAM).LIS $(MAP)
	$(RM) *.lis *.glis CPU32.debug
	
$(PROGRAM).LIS: $(ASMGLIS) $(MAP)
	$(GAL) $(MAP) $(ASMGLIS)
	
