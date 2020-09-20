# compilers and tools
CC=gcc
CFLAGS=-Wall -W -ggdb -I.. -L.. -levfilter -ljson-c

BINARIES=test

all: $(BINARIES)
	@./run_test.sh

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BINARIES)

