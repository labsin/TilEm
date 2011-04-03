AR = ar
CC = gcc
CFLAGS = -g -W -Wall -DD1
DEFS = -DPACKAGE_VERSION=\"pre-release\"
RANLIB = ranlib

srcdir = .
top_srcdir = ..
top_builddir = ..

GTK_CFLAGS = `pkg-config --cflags gtk+-2.0 gthread-2.0 glib-2.0`
GTK_LIBS = `pkg-config --libs gtk+-2.0 gthread-2.0 glib-2.0`

TICALCS_CFLAGS = `pkg-config --cflags ticalcs2 ticonv`
TICALCS_LIBS = `pkg-config --libs ticalcs2 ticonv`

TILEMCORE_CFLAGS = -I$(top_srcdir)/core
TILEMCORE_LIBS = -L$(top_builddir)/core -ltilemcore

TILEMDB_CFLAGS = -I$(top_srcdir)/db
TILEMDB_LIBS = -L$(top_builddir)/db -ltilemdb

objects = tilem2.o \
	animatedgif.o \
	args.o \
	config.o \
	debugger.o \
	emulator.o \
	event.o \
	gifencod.o \
	keybindings.o \
	link.o \
	macro.o \
	memmodel.o \
	memory.o \
	pbar.o \
	screen.o \
	screenshot.o \
	skin.o \
	skinops.o \
	ti81prg.o \
	tool.o

libs = $(TILEMDB_LIBS) $(TILEMCORE_LIBS) $(GTK_LIBS) $(TICALCS_LIBS) $(LIBS)

compile = $(CC) -I$(srcdir) $(CFLAGS) $(DEFS) $(TILEMCORE_CFLAGS) $(TILEMDB_CFLAGS) $(GTK_CFLAGS) $(TICALCS_CFLAGS)

all: tilem2

#Main emulator GUI
tilem2: $(objects) ../core/libtilemcore.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o tilem2 $(objects) $(libs)

tilem2.o: tilem2.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/tilem2.c

#Debugger
debugger.o: debugger.c debugger.h gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/debugger.c

# Tree model interface for calc memory
memmodel.o: memmodel.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/memmodel.c

# Memory management and messages
memory.o: memory.c ../core/tilem.h
	$(compile) -c $(srcdir)/memory.c

# Main emulation loop
emulator.o: emulator.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/emulator.c

# Print the skin and add the events. 
screen.o: screen.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/screen.c

# Handle events
event.o: event.c event.h gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/event.c

# Open skin (skn format file) originally created by Julien Blache and Romain Lievins
skinops.o: skinops.c skinops.h 
	$(compile) -c $(srcdir)/skinops.c

# Manage skins
skin.o: skin.c skin.h gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/skin.c 

# Popups and other stuff
tool.o: tool.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/tool.c

# Manage config.ini 
config.o: config.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/config.c

# Handle internal link
link.o: link.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/link.c

# Handle macro
macro.o: macro.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/macro.c

# Handle args
args.o: args.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/args.c

# Create and modify animated gif
gifencod.o: gifencod.c gifencod.h
	$(compile) -c $(srcdir)/gifencod.c

# Handle screenshot anim (animated gif)
animatedgif.o: animatedgif.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/animatedgif.c

# Screenshot widget
screenshot.o: screenshot.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/screenshot.c

# Progress bar widget
pbar.o: pbar.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/pbar.c


# Keybindings
keybindings.o: keybindings.c gui.h *.h ../core/tilem.h
	$(compile) -c $(srcdir)/keybindings.c


# TI-81 program file functions
ti81prg.o: ti81prg.c ti81prg.h ../core/tilem.h
	$(compile) -c $(srcdir)/ti81prg.c



install: testemu2
	chmod +x install.sh
	./install.sh
clean:
	rm -f *.o
	rm -f tilem2

.PHONY: all clean
