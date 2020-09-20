# compilers and tools
CC=gcc
CFLAGS=-Wall -W -ggdb -I.. -L.. -levfilter

BINARIES=draw hotplug hotplug pointer_test uinput

all: $(BINARIES)

draw: draw.c sdl_utils.c sdl_utils.h
	$(CC) $(CFLAGS) `sdl-config --static-libs --cflags` -lSDL_gfx -lSDL draw.c sdl_utils.c -o draw

pointer_test: pointer_test.c
	$(CC) $(CFLAGS) `sdl-config --static-libs --cflags` -lSDL_gfx -lSDL pointer_test.c -o pointer_test

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BINARIES)
