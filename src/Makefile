TOPDIR=../

CSOURCES=evf_err.c evf_profile.c evf_filter.c \
         evf_loader.c evf_line.c evf_input.c \
         evf_hotplug.c evf_uinput.c evf_pipe.c config.c

TARGETS=libevfilter.so
SUBDIRS=filters keyparser evfd examples tests
INSTALL_HEADERS=$(wildcard *.h)
INSTALL_LIBRARIES=libevfilter.so libevfilter.so.0 libevfilter.so.0.0
CLEAN=libevfilter.so libevfilter.so.0 libevfilter.so.0.0

include $(TOPDIR)/common.mk

ifneq ($(MAKECMDGOALS),clean)
tests: libevfilter.so
examples: libevfilter.so
evfd: libevfilter.so
endif

CFLAGS+=-fPIC -DDEBUG -Ikeyparser

KEYPARSER_HEADERS=keyparser/key_parser.h keyparser/keys.h

libevfilter.so: filters keyparser $(OBJECTS) $(HEADERS) $(KEYPARSER_HEADERS)
ifdef VERBOSE
	$(CC) --shared -Wl,-soname -Wl,libevfilter.so.0 $(OBJECTS) keyparser/*.o filters/*.o -ljson-c -o libevfilter.so.0.0
	rm -f libevfilter.so.0 && ln -s libevfilter.so.0.0 libevfilter.so.0
	rm -f libevfilter.so && ln -s libevfilter.so.0.0 libevfilter.so
else
	@echo "LD  $@"
	@$(CC) --shared -Wl,-soname -Wl,libevfilter.so.0 $(OBJECTS) keyparser/*.o filters/*.o -ljson-c -o libevfilter.so.0.0
	@rm -f libevfilter.so.0 && ln -s libevfilter.so.0.0 libevfilter.so.0
	@rm -f libevfilter.so && ln -s libevfilter.so.0.0 libevfilter.so
endif
