// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  This program is testing io queue and hotplug interface by printing out every
  event from every input device plugged into computer.

 */

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "evfilter.h"

static struct evf_io_queue *queue;

static int read_event(struct evf_io_queue_memb *self)
{
	struct input_event ev;

	if (read(self->fd, &ev, sizeof (struct input_event)) < 0) {
		printf("read failed, closing fd %i\n", self->fd);

		return EVF_IO_QUEUE_REM | EVF_IO_QUEUE_CLOSE;
	}

	printf("event from fd %i\n", self->fd);

	evf_input_print(stdout, " *** ", &ev);

	return EVF_IO_QUEUE_OK;
}

static void device_plugged(const char *dev)
{
	int fd;

	printf("device %s plugged.\n", dev);

	fd = open(dev, O_RDONLY);

	if (fd < 0) {
		perror("Can't open device");
		return;
	}

	evf_io_queue_add(queue, fd, read_event, NULL);
}

static int hotplug_rescan(struct evf_io_queue_memb *self
                          __attribute__ ((unused)))
{
	evf_hotplug_rescan();

	return 0;
}

int main(void)
{
	int fd;
	queue = evf_io_queue_new();

	if (queue == NULL) {
		fprintf(stderr, "Can't allocate io queue.\n");
		exit(EXIT_FAILURE);
	}

	if ((fd = evf_hotplug_init(device_plugged, NULL)) < 0) {
		fprintf(stderr, "Can't initalize hotplug: %s\n",
		        strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* add hotplug into queue */
	if (!evf_io_queue_add(queue, fd, hotplug_rescan, NULL)) {
		fprintf(stderr, "Can't allocate hotplug queue handler.\n");
		exit(EXIT_FAILURE);
	}

	printf("Sleeping in io queue for events.\n");

	for (;;)
		evf_io_queue_wait(queue, NULL);

	return 0;
}

