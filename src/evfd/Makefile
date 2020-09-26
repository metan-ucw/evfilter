TOPDIR=../../

BINARIES=evfd
CSOURCES=evfd.c evfd_lock.c evf_io_queue.c
INSTALL_BINARIES=evfd

evfd: evfd.o evfd_lock.o evf_io_queue.o

include $(TOPDIR)/common.mk

CFLAGS+=-I..
LDFLAGS+=-L..
LDLIBS+=-levfilter
