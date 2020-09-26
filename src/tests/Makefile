TOPDIR=../../

CSOURCES=test.c
BINARIES=test
TARGETS=run_tests

include $(TOPDIR)/common.mk

test: test.o

CFLAGS+=-I../
LDFLAGS+=-L../
LDLIBS+=-levfilter -ljson-c

run_tests: test
	@./run_test.sh

