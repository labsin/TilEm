
all: gif_reader2


gif_reader2: gif_reader2.c
	gcc `pkg-config --libs --cflags gtk+-2.0` gif_reader2.c -o gif_reader2
	
	
clean:

	rm -f gif_reader

.PHONY: all clean
