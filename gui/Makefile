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

objects = testemu.o

libs = -L../core -ltilemcore -lreadline $(GTK_LIBS) $(TICONV_LIBS) $(LIBS)

compile = $(CC) -I$(top_builddir) -I$(srcdir) -I$(srcdir)/../core $(CFLAGS) $(CPPFLAGS) $(DEFS) $(GTK_CFLAGS) $(TICONV_CFLAGS)

all: testemu

testemu: $(objects) ../core/libtilemcore.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o testemu $(objects) $(libs)

testemu.o: testemu.c ../core/tilem.h
	$(compile) -c $(srcdir)/testemu.c

clean:
	rm -f *.o
	rm -f testemu

.PHONY: all clean
