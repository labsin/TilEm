AR = ar
CC = gcc
CFLAGS = -g -W -Wall -DD1
DEFS = -DPACKAGE_VERSION=\"pre-release\"
RANLIB = ranlib

srcdir = .
top_srcdir = ../..
top_builddir = ../..

GTK_CFLAGS = `pkg-config --cflags gtk+-2.0 gthread-2.0 ticalcs2`
GTK_LIBS = `pkg-config --libs gtk+-2.0 gthread-2.0 ticalcs2`
	
TICONV_CFLAGS = `pkg-config --cflags ticonv`
TICONV_LIBS = `pkg-config --libs ticonv`


objects = testemu2.o memory.o screen.o event.o skinops.o skin.o tool.o config.o romconfig.o debugger.o link.o ../db/disasm.o macro.o args.o


libs = -L../core -ltilemcore -lreadline $(GTK_LIBS) $(TICONV_LIBS) $(LIBS)

compile = $(CC) -I$(srcdir) -I$(srcdir)/../core -I$(srcdir)/../db $(CFLAGS) $(DEFS) $(GTK_CFLAGS) 

all: testemu2

#Main emulator GUI
testemu2: $(objects) ../core/libtilemcore.a gui.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o testemu2 $(objects) $(libs)
	
testemu2.o: testemu2.c 
	$(compile) -c $(srcdir)/testemu2.c

#Debugger
debugger.o: debugger.c debugger.h gui.h 
	$(compile) -c $(srcdir)/debugger.c 

# Memory management and messages
memory.o: memory.c 
	$(compile) -c $(srcdir)/memory.c

# Print the skin and add the events. 
screen.o: screen.c gui.h 
	$(compile) -c $(srcdir)/screen.c

# Handle events
event.o: event.c event.h 
	$(compile) -c $(srcdir)/event.c

# Open skin (skn format file) originally created by Julien Blache and Romain Lievins
skinops.o: skinops.c skinops.h 
	$(compile) -c $(srcdir)/skinops.c

# Manage skins
skin.o: skin.c skin.h
	$(compile) -c $(srcdir)/skin.c 

# Popups and other stuff
tool.o: tool.c gui.h  
	$(compile) -c $(srcdir)/tool.c
	
# Manage skinconfig.dat
config.o: config.c gui.h 
	$(compile) -c $(srcdir)/config.c

# Manage romconfig.dat
romconfig.o: romconfig.c gui.h 
	$(compile) -c $(srcdir)/romconfig.c

# Handle internal link
link.o: link.c gui.h 
	$(compile) -c $(srcdir)/link.c
	
# Handle macro
macro.o: macro.c gui.h 
	$(compile) -c $(srcdir)/macro.c

# Handle args
args.o: args.c gui.h 
	$(compile) -c $(srcdir)/args.c

clean:
	rm -f *.o
	rm -f testemu2

.PHONY: all clean
