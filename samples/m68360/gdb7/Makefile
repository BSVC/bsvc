#
# Makefile -*- BASH -*-
#
# DJ (c) 1996
#
# USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
# WITH NO WARRANTY FOR ANY PURPOSE.
#

#-------------------------------------------------------------------------
CPU=68000
#-------------------------------------------------------------------------

CC=vxworks68-gcc
CFLAGS=-g -Wall -m$(CPU) -mnobitfield -I. -U__HAVE_68881__ -DDEBUG
AS=vxworks68-as
ASFLAGS=-m$(CPU)
LD=vxworks68-ld
LDFLAGS=-M 
RM=/bin/rm -f
GAL=python $(BSVC20)/etc/gal.py

#-------------------------------------------------------------------------
LIBS=-L$(M68K_LIB) -lgcc

.SUFFIXES: .cxx .s .glis

PROGRAM=test7
GDB_STUB_OBJECTS=astub.o mstub.o ustub.o xstub.o
GDB_STUB_ASMSRC=astub.s mstub.s ustub.s xstub.s
GDB_STUB_ASMGLIS=astub.glis mstub.glis ustub.glis xstub.glis
OBJECTS=boot2.o test7.o duart.o libc.o jump.o $(GDB_STUB_OBJECTS)
ASMSRC=boot2.s test7.s duart.s libc.s $(GDB_STUB_ASMSRC)
ASMGLIS=boot2.glis test7.glis duart.glis libc.glis jump.glis $(GDB_STUB_ASMGLIS)
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
all: $(PROGRAM).H68 $(PROGRAM).bin $(PROGRAM).LIS

$(PROGRAM).H68 $(MAP): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -T $(PROGRAM).cmd $(LIBS) > $(MAP)

$(PROGRAM).bin: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -T $(PROGRAM).bin.cmd $(LIBS) > $(PROGRAM).bin.map

clean:
	$(RM) *.o $(PROGRAM).H68 $(PROGRAM).LIS $(MAP)
	$(RM) $(PROGRAM).bin $(PROGRAM).bin.map
	$(RM) *.lis *.glis CPU32.debug
	$(RM) test7.s libc.s astub.s mstub.s ustub.s xstub.s
	
$(PROGRAM).LIS: $(ASMGLIS) $(MAP)
	$(GAL) $(MAP) $(ASMGLIS)
	
