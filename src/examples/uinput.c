// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  This is test for evf_uinput interface for user input kernel interface, beware
  that this sends enter keys and if our xserver listens for event input devices
  by default you end up with enter pressing each second.

 */

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "evf_uinput.h"

static int sig_flag = 1;

static void sighandler(int sig __attribute__ ((unused)))
{
	sig_flag = 0;
}

static void pack_event(struct input_event *ev, int type, int code, int value)
{
	gettimeofday(&ev->time, NULL);
	ev->type  = type;
	ev->code  = code;
	ev->value = value;
}

int main(void)
{
	int fd;
	struct uinput_user_dev dev;
	struct input_event ev;

	memset(&dev, 0, sizeof (dev));
	strcpy(dev.name, "test");

	fd = evf_uinput_create(&dev);

	if (fd < 0) {
		fprintf(stderr, "Failed to create uinput %i\n", fd);
		return 1;
	}

	signal(SIGQUIT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGINT, sighandler);

	while (sig_flag) {
		sleep(1);

		/* send enter key down */
		pack_event(&ev, EV_KEY, KEY_ENTER, 1);
		write(fd, &ev, sizeof (ev));

		/* send sync */
		pack_event(&ev, EV_SYN, SYN_REPORT, 0);
		write(fd, &ev, sizeof (ev));

		/* send enter key up */
		pack_event(&ev, EV_KEY, KEY_ENTER, 0);
		write(fd, &ev, sizeof (ev));

		/* send sync */
		pack_event(&ev, EV_SYN, SYN_REPORT, 0);
		write(fd, &ev, sizeof (ev));
	}

	evf_uinput_destroy(fd);
	printf("Got signal, exiting...\n");

	return 0;
}
