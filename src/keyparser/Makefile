TOPDIR=../../

CSOURCES=evf_getkey.c key_parser.c
HEADERS=$(shell ls ./*.h)
BINARIES=evf-getkey
INSTALL_BINARIES=evf-getkey

#TODO: rebuild keys.h?
evf-getkey: key_parser.o evf_getkey.o

include $(TOPDIR)/common.mk

CFLAGS+=-fPIC -DDEBUG

keys.h: keyparser.in do_parser.sh
	./do_parser.sh > keys.h
