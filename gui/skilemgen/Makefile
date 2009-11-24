AR = ar
CC = gcc
CFLAGS = -g -O2 -W -Wall
DEFS = -DPACKAGE_VERSION=\"pre-release\"
RANLIB = ranlib

srcdir = .
top_srcdir = ../..
top_builddir = ../..

GTK_CFLAGS = `pkg-config --cflags gtk+-2.0 gthread-2.0`
GTK_LIBS = `pkg-config --libs gtk+-2.0 gthread-2.0`


compile = $(CC) -I$(top_builddir) -I$(srcdir) -I$(srcdir) $(CFLAGS) $(CPPFLAGS) $(DEFS) $(GTK_CFLAGS)

libs = -L -lreadline $(GTK_LIBS) $(LIBS)


all: skintools

#Skintools
skintools:  $(objects) skintools.o skintools.h
	$(CC) $(GTK_CFLAGS) -o skintools skintools.o skintools.h $(objects) $(libs)
	
skintools.o: $(objects) skintools.c
	$(CC) $(GTK_CFLAGS) -o skintools.o -c skintools.c $(objects) $(libs)
	

	
clean:
	rm skintools

.PHONY: all clean

