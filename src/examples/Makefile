TOPDIR=../../

CSOURCES=uinput.c #hotplug.c
BINARIES=uinput# hotplug

include $(TOPDIR)/config.mk

ifeq ($(HAVE_LIBSDL),yes)
CSOURCES += sdl_utils.c pointer_test.c draw.c
BINARIES += pointer_test draw
endif

include $(TOPDIR)/common.mk

LDFLAGS+=-L../
LDLIBS+=-levfilter
CFLAGS+=-I../

ifeq ($(HAVE_LIBSDL),yes)
draw: CFLAGS += $(shell sdl-config --static-libs --cflags)
pointer_test: CFLAGS += $(shell sdl-config --static-libs --cflags)
draw: LDLIBS += -lSDL_gfx -lSDL
draw: sdl_utils.o
pointer_test: LDLIBS += -lSDL_gfx -lSDL
endif
