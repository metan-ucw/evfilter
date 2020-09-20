# compilers and tools
CC=gcc
CFLAGS=-W -Wall -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -ggdb -I.. -L.. -levfilter

BINARY=evfd

$(BINARY): evfd.o evfd_lock.o evf_io_queue.o

all: $(BINARY)

%: %.o
	$(CC) $(CFLAGS) $^ -o $@

%: %.c
	$(CC) $(CFLAGS) $< -o $@

install:
	install -d $(RPM_BUILD_ROOT)/usr/sbin
	install -m 775 $(BINARY) $(RPM_BUILD_ROOT)/usr/sbin/

clean:
	rm -rf $(BINARY) *.o

evfd_lock.o: evfd_lock.c evfd_lock.h
evfd.o: evfd.c evfd_lock.h evfd.h
evfd_lock.o: evfd_lock.c evfd_lock.h
