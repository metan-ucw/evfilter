TOPDIR=../../

BINARIES=evfd
CSOURCES=evfd.c evfd_lock.c
INSTALL_BINARIES=evfd

evfd: evfd.o evfd_lock.o

include $(TOPDIR)/common.mk

CFLAGS+=-I..
LDFLAGS+=-L..
LDLIBS+=-levfilter
