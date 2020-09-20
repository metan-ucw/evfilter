// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Simple utility to get key names from /dev/input/eventX
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <linux/input.h>
#include <linux/uinput.h>
#include "key_parser.h"

#define PRINT_ERR(...) { fprintf(stderr, "evf-getkey: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#define PRINT_MSG(...) { printf("evf-getkey: "); printf(__VA_ARGS__); printf("\n"); }

static int open_input_fd(char *dev)
{
	int e, fd = open(dev, O_RDONLY);
	int version;
	char name[256] = "Unknown";

	if (fd < 0) {
		e = errno;
		PRINT_ERR("Can't open %s: %s.", dev, strerror(e))
		return -1;
	}

	if (ioctl(fd, EVIOCGVERSION, &version)) {
		PRINT_ERR("`%s' is not an input device.", dev)
		close(fd);
		return -1;
	}

	PRINT_MSG("Input driver version %d.%d.%d",
	          version >> 16, version >> 8 & 0xff, version & 0xff);

	ioctl(fd, EVIOCGNAME(sizeof(name)), name);

	PRINT_MSG("Device name `%s'", name);

	return fd;
}

int main(int argc, char *argv[])
{
	int fd;
	struct input_event ev;

	if (argc < 2) {
		fprintf(stderr, "Usage: evf-getkey /dev/input/eventX\n");
		exit(EXIT_FAILURE);
	}

	if ((fd = open_input_fd(argv[1]))<0) {
		exit(EXIT_FAILURE);
	}

	for(;;) {
		int size = sizeof (struct input_event);

		if (read(fd, &ev, size) < size) {
			fprintf(stderr, "Error reading from fd: %s\n",
			        strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (ev.type == EV_KEY) {
			printf("key %s\n", keyparser_getname(ev.code));
		}
	}

	exit(EXIT_SUCCESS);
}
