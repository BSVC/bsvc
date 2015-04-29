#
# Makefile -*- BASH -*-
#
# USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
# WITH NO WARRANTY FOR ANY PURPOSE.
#

#-------------------------------------------------------------------------
CPU=68020
#-------------------------------------------------------------------------

CC=m68k-none-elf-gcc
CFLAGS=-g -Wall -m$(CPU) -mnobitfield -I. -U__HAVE_68881__
AS=m68k-none-elf-as --register-prefix-optional
ASFLAGS=-m$(CPU)
LD=m68k-none-elf-ld
LDFLAGS=-M 
RM=/bin/rm -f
GAL=python $(BSVC)/bin/gal.py

#-------------------------------------------------------------------------
LIBS=-L$(M68K_LIB) -lgcc

.SUFFIXES: .s .glis

PROGRAM=test2
OBJECTS=boot2.o test2.o duart.o
ASMSRC=boot2.s test2.s duart.s
ASMGLIS=boot2.glis test2.glis duart.glis
MAP=$(PROGRAM).map
#-------------------------------------------------------------------------
.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

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
	
