# compilers and tools
CC=gcc
CCFLAGS=-fPIC -Wall -W -ggdb -O -DDEBUG

# source files
CSOURCES=evf_getkey.c key_parser.c
HEADERS=$(shell ls ./*.h)

# object files
OBJECTS=$(CSOURCES:.c=.o) 

all: evf-getkey

evf-getkey: key_parser.o evf_getkey.o
	$(CC) key_parser.o evf_getkey.o -o evf-getkey

keys.h: keyparser.in do_parser.sh
	./do_parser.sh > keys.h

# rulez for compiling c sources
.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@

# clean!
clean:
	rm -rf $(OBJECTS) evf-getkey

# dependencies
evf_getkey.o: evf_getkey.c key_parser.h
key_parser.o: key_parser.c key_parser.h keys.h
