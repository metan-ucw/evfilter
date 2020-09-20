// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/uinput.h>
#include <stdio.h>

#include "config.h"
#include "evf_uinput.h"

static int try_open(const char *paths[], int mode)
{
	int fd, i;

	for (i = 0; paths[i] != NULL; i++) {
		fd = open(paths[i], mode);

		if (fd != -1)
			break;
	}

	return fd;
}

int evf_uinput_create(struct uinput_user_dev *ui_dev_info)
{
	int fd, i, ret;

	fd = try_open(EVFILTER_UINPUT_DEV_PATHS, O_WRONLY);

	if (fd < 0)
		return -1;

	/* just enable all by default */
	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_EVBIT, EV_REL);
	ioctl(fd, UI_SET_RELBIT, REL_X);
	ioctl(fd, UI_SET_RELBIT, REL_Y);
	ioctl(fd, UI_SET_RELBIT, REL_WHEEL);
	ioctl(fd, UI_SET_RELBIT, REL_HWHEEL);

	ioctl(fd, UI_SET_KEYBIT, BTN_MOUSE);

	for (i = 0; i < KEY_MAX; i++)
		ioctl(fd, UI_SET_KEYBIT, i);

	ret = write(fd, ui_dev_info, sizeof(*ui_dev_info));

	if (ret != sizeof(*ui_dev_info)) {
		close(fd);
		return -2;
	}

	/* create uinput device */
	ret = ioctl(fd, UI_DEV_CREATE);

	if (ret != 0) {
		close(fd);
		return -3;
	}

	return fd;
}

void evf_uinput_destroy(int fd)
{
	ioctl(fd, UI_DEV_DESTROY);
	close(fd);
}
