AR = ar
CC = gcc
CFLAGS = -g -O3 -W -Wall -Wwrite-strings -Werror
DEFS = -DPACKAGE_VERSION=\"pre-release\"
RANLIB = ranlib

srcdir = .
top_srcdir = ../..
top_builddir = ../..

core_objects = calcs.o z80.o state.o rom.o flash.o link.o keypad.o lcd.o

x1_objects = x1_init.o x1_io.o x1_memory.o x1_subcore.o
x2_objects = x2_init.o x2_io.o x2_memory.o x2_subcore.o
x3_objects = x3_init.o x3_io.o x3_memory.o x3_subcore.o
xp_objects = xp_init.o xp_io.o xp_memory.o xp_subcore.o

objects = $(core_objects) $(x1_objects) $(x2_objects) $(x3_objects) $(xp_objects)

compile = $(CC) -I$(top_builddir) -I$(srcdir) $(CFLAGS) $(CPPFLAGS) $(DEFS)

all: libtilemcore.a

libtilemcore.a: $(objects)
	$(AR) cru libtilemcore.a $(objects)
	$(RANLIB) libtilemcore.a

# Main emulator core

calcs.o: calcs.c tilem.h z80.h
	$(compile) -c $(srcdir)/calcs.c
z80.o: z80.c z80.h z80cmds.h z80main.h z80cb.h z80ddfd.h z80ed.h tilem.h
	$(compile) -c $(srcdir)/z80.c
state.o: state.c tilem.h z80.h
	$(compile) -c $(srcdir)/state.c
rom.o: rom.c tilem.h
	$(compile) -c $(srcdir)/rom.c
flash.o: flash.c tilem.h
	$(compile) -c $(srcdir)/flash.c
link.o: link.c tilem.h
	$(compile) -c $(srcdir)/link.c
keypad.o: keypad.c tilem.h scancodes.h
	$(compile) -c $(srcdir)/keypad.c
lcd.o: lcd.c tilem.h
	$(compile) -c $(srcdir)/lcd.c

# TI-81

x1_init.o: x1/x1_init.c x1/x1.h tilem.h
	$(compile) -c $(srcdir)/x1/x1_init.c
x1_io.o: x1/x1_io.c x1/x1.h tilem.h
	$(compile) -c $(srcdir)/x1/x1_io.c
x1_memory.o: x1/x1_memory.c x1/x1.h tilem.h
	$(compile) -c $(srcdir)/x1/x1_memory.c
x1_subcore.o: x1/x1_subcore.c x1/x1.h tilem.h
	$(compile) -c $(srcdir)/x1/x1_subcore.c

# TI-82

x2_init.o: x2/x2_init.c x2/x2.h tilem.h
	$(compile) -c $(srcdir)/x2/x2_init.c
x2_io.o: x2/x2_io.c x2/x2.h tilem.h
	$(compile) -c $(srcdir)/x2/x2_io.c
x2_memory.o: x2/x2_memory.c x2/x2.h tilem.h
	$(compile) -c $(srcdir)/x2/x2_memory.c
x2_subcore.o: x2/x2_subcore.c x2/x2.h tilem.h
	$(compile) -c $(srcdir)/x2/x2_subcore.c

# TI-83

x3_init.o: x3/x3_init.c x3/x3.h tilem.h
	$(compile) -c $(srcdir)/x3/x3_init.c
x3_io.o: x3/x3_io.c x3/x3.h tilem.h
	$(compile) -c $(srcdir)/x3/x3_io.c
x3_memory.o: x3/x3_memory.c x3/x3.h tilem.h
	$(compile) -c $(srcdir)/x3/x3_memory.c
x3_subcore.o: x3/x3_subcore.c x3/x3.h tilem.h
	$(compile) -c $(srcdir)/x3/x3_subcore.c

# TI-83 Plus

xp_init.o: xp/xp_init.c xp/xp.h tilem.h
	$(compile) -c $(srcdir)/xp/xp_init.c
xp_io.o: xp/xp_io.c xp/xp.h tilem.h
	$(compile) -c $(srcdir)/xp/xp_io.c
xp_memory.o: xp/xp_memory.c xp/xp.h tilem.h
	$(compile) -c $(srcdir)/xp/xp_memory.c
xp_subcore.o: xp/xp_subcore.c xp/xp.h tilem.h
	$(compile) -c $(srcdir)/xp/xp_subcore.c


clean:
	rm -f *.o
	rm -f libtilemcore.a

.PHONY: clean all
