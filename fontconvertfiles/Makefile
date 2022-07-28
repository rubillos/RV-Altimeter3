all: fontconvert

CC     = gcc
CFLAGS = -Wall -I/opt/homebrew/include/freetype2 -I/opt/homebrew/freetype2 -I/opt/homebrew
LIBS   = -L/opt/homebrew/lib -lfreetype

fontconvert: fontconvert.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@
	strip $@

clean:
	rm -f fontconvert
