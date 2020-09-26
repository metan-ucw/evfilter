TOPDIR=../../

CSOURCES=uinput.c hotplug.c
BINARIES=uinput hotplug

include $(TOPDIR)/config.mk

ifeq ($(HAVE_LIBSDL),yes)
CSOURCES += sdl_utils.c pointer_test.c draw.c
BINARIES += pointer_test draw
endif

include $(TOPDIR)/common.mk

uinput: uinput.o
hotplug: hotplug.o
pointer_test: pointer_test.o
draw: draw.o sdl_utils.o

LDFLAGS+=-L../
LDLIBS+=-levfilter
CFLAGS+=-I../

ifeq ($(HAVE_LIBSDL),yes)
draw: CFLAGS += $(shell sdl-config --libs --cflags)
pointer_test: CFLAGS += $(shell sdl-config --libs --cflags)
draw: LDLIBS += -lSDL_gfx -lSDL
draw: sdl_utils.o
pointer_test: LDLIBS += -lSDL_gfx -lSDL
endif
