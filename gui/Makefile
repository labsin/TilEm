AR = ar
CC = gcc
CFLAGS = -g -W  -Wall -DG_ERRORCHECK_MUTEXES -DDEBUGGINGGLOBAL_L0 -DDEBUGGINGGLOBAL_L2 -DDEBUGGINGSKIN_L0 -DDEBUGGINGCLICK_L0 -DDEBUGGINGDEBUGGER_L0 -DDEBUGGINGCONFIG_FILE_L0#-DDEBUGGINGCLICK_L2 -DDEBUGGINGLCD_L0 #-DDEBUGGINGLCD_L2
DEFS = -DPACKAGE_VERSION=\"pre-release\"
RANLIB = ranlib

srcdir = .
top_srcdir = ../..
top_builddir = ../..

GTK_CFLAGS = `pkg-config --cflags gtk+-2.0 gthread-2.0 ticalcs2`
GTK_LIBS = `pkg-config --libs gtk+-2.0 gthread-2.0 ticalcs2`
	
TICONV_CFLAGS = `pkg-config --cflags ticonv`
TICONV_LIBS = `pkg-config --libs ticonv`


objects = testemu2.o memory.o screen.o event.o skinops.o skin.o tool.o config.o debugger.o


libs = -L../core -ltilemcore -lreadline $(GTK_LIBS) $(TICONV_LIBS) $(LIBS)

compile = $(CC) -I$(top_builddir) -I$(srcdir) -I$(srcdir)/../core $(CFLAGS) $(CPPFLAGS) $(DEFS) $(GTK_CFLAGS) $(TICONV_CFLAGS)

all: testemu2

#Main emulator GUI
testemu2: $(objects) ../core/libtilemcore.a gui.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o testemu2 $(objects) $(libs)
	
testemu2.o: testemu2.c ../core/tilem.h gui.h
	$(compile) -c $(srcdir)/testemu2.c

#Debugger
debugger.o: debugger.c ../core/tilem.h gui.h
	$(compile) -c $(srcdir)/debugger.c

# Memory management and messages
memory.o: memory.c ../core/tilem.h gui.h
	$(compile) -c $(srcdir)/memory.c

# Build the Skin set and define the key coord and the key list
screen.o: screen.c ../core/tilem.h gui.h
	$(compile) -c $(srcdir)/screen.c

# Handle an GDKevent
event.o: event.c ../core/tilem.h gui.h
	$(compile) -c $(srcdir)/event.c

# Open skin
skinops.o: skinops.c ../core/tilem.h skinops.h 
	$(compile) -c $(srcdir)/skinops.c

# Manage skins
skin.o: skin.c ../core/tilem.h gui.h 
	$(compile) -c $(srcdir)/skin.c

# Popups and other stuff
tool.o: tool.c ../core/tilem.h gui.h 
	$(compile) -c $(srcdir)/tool.c
	
# Popups and other stuff
config.o: config.c ../core/tilem.h gui.h 
	$(compile) -c $(srcdir)/config.c

	
clean:
	rm -f *.o
	rm -f testemu2

.PHONY: all clean
