CC=gcc
CFLAGS=-fPIC -W -Wall -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -ggdb -DDEBUG -Ikeyparser

# source files
CSOURCES=evf_err.c evf_profile.c evf_filter.c \
         evf_loader.c evf_line.c evf_input.c \
         evf_hotplug.c evf_uinput.c evf_pipe.c config.c

KEYPARSER_HEADERS=keyparser/key_parser.h keyparser/keys.h
HEADERS=$(shell ls ./*.h)

# object files
OBJECTS=$(CSOURCES:.c=.o)

all: libevfilter.so make_evfd tests

.PHONY: tests

make_keyparser:
	@cd keyparser && $(MAKE)

make_filters:
	@cd filters && $(MAKE)

make_evfd:
	@cd evfd && $(MAKE)

tests:
	@cd tests && $(MAKE)

libevfilter.so: make_filters make_keyparser $(OBJECTS) $(HEADERS) $(KEYPARSER_HEADERS)
	$(CC) --shared -Wl,-soname -Wl,libevfilter.so.0 $(OBJECTS) keyparser/*.o filters/*.o -ljson-c -o libevfilter.so

# rules for compiling c sources
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

# install
INSTALL_HEADERS=evfilter.h evf_err.h evf_filter.h evf_hotplug.h      \
                evf_line.h evf_loader.h evf_param.h evf_profile.h    \
                evf_io_queue.h evf_struct.h evf_input.h evf_uinput.h \
		evf_msg.h evf_func.h

install: libevfilter.so
	install -d $(RPM_BUILD_ROOT)/usr/lib
	install libevfilter.so $(RPM_BUILD_ROOT)/usr/lib/
	if ! [ -e /usr/lib/libevfilter.so.0 ]; then \
		ln -s libevfilter.so $(RPM_BUILD_ROOT)/usr/lib/libevfilter.so.0; \
	fi
	install -d $(RPM_BUILD_ROOT)/usr/include/evfilter/
	install -m 644 $(INSTALL_HEADERS) $(RPM_BUILD_ROOT)/usr/include/evfilter/
	@cd evfd && $(MAKE) install

clean:
	rm -rf $(OBJECTS) libevfilter.so
	@cd keyparser && $(MAKE) clean
	@cd filters && $(MAKE) clean
	@cd evfd && $(MAKE) clean
	@cd tests && $(MAKE) clean
