AR = ar
CC = gcc
CFLAGS = -g -O2 -W -Wall -Wwrite-strings
DEFS = -DPACKAGE_VERSION=\"pre-release\"
RANLIB = ranlib

srcdir = .
top_srcdir = ..
top_builddir = ..

objects = disasm.o listing.o lstfile.o

compile = $(CC) -I$(top_builddir) -I$(top_srcdir)/emu -I$(srcdir) $(CFLAGS) $(CPPFLAGS) $(DEFS)

all: libtilemdb.a

libtilemdb.a: $(objects)
	$(AR) cru libtilemdb.a $(objects)
	$(RANLIB) libtilemdb.a

# Disassembler

disasm.o: disasm.c tilemdb.h ../emu/tilem.h
	$(compile) -c $(srcdir)/disasm.c

# Listing file management

listing.o: listing.c tilemdb.h ../emu/tilem.h
	$(compile) -c $(srcdir)/listing.c

lstfile.o: lstfile.c tilemdb.h ../emu/tilem.h
	$(compile) -c $(srcdir)/lstfile.c


clean:
	rm -f *.o
	rm -f libtilemdb.a

.PHONY: all clean
