AR = ar
CC = gcc
CFLAGS = -g -O2 -W -Wall -Wwrite-strings -Werror
DEFS = -DPACKAGE_VERSION=\"pre-release\"
RANLIB = ranlib

srcdir = .
top_srcdir = ../..
top_builddir = ../..

GTK_CFLAGS = `pkg-config --cflags gtk+-2.0 gthread-2.0 ticalcs2`
GTK_LIBS = `pkg-config --libs gtk+-2.0 gthread-2.0 ticalcs2`

TICONV_CFLAGS = `pkg-config --cflags ticonv`
TICONV_LIBS = `pkg-config --libs ticonv`


objects = testemu2.o memory.o


libs = -L../core -ltilemcore -lreadline $(GTK_LIBS) $(TICONV_LIBS) $(LIBS)

compile = $(CC) -I$(top_builddir) -I$(srcdir) -I$(srcdir)/../core $(CFLAGS) $(CPPFLAGS) $(DEFS) $(GTK_CFLAGS) $(TICONV_CFLAGS)

all: testemu2

#Main emulator GUI

testemu2: $(objects) ../core/libtilemcore.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o testemu2 $(objects) $(libs)
	
testemu2.o: testemu2.c ../core/tilem.h
	$(compile) -c $(srcdir)/testemu2.c


# Memory management and messages (maybe move it in the core directory ??)

memory.o: memory.c ../core/tilem.h
	$(compile) -c $(srcdir)/memory.c

	
clean:
	rm -f *.o
	rm -f testemu

.PHONY: all clean
