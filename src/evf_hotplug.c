// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/inotify.h>

#include "evf_hotplug.h"
#include "filters/evf_msg.h"

#define EVF_DEV_PATH "/dev/input/"
#define EVF_BUF_LEN 1024

static void (*dev_plug)(const char *dev) = NULL;
static void (*dev_unplug)(const char *dev) = NULL;
static int select_fd;

/*
 * Walk trough EVF_DEV_PATH directory and send initial hotplug events.
 */
static int walk(void)
{
	DIR *dir = opendir(EVF_DEV_PATH);
	struct dirent *dir_ent;
	char buf[EVF_BUF_LEN];

	if (dir == NULL)
		return 1;

	errno = 0;

	while ((dir_ent = readdir(dir)) != NULL) {
		if (!strncmp(dir_ent->d_name, "event", 5) && dev_plug) {
			snprintf(buf, EVF_BUF_LEN, "%s%s", EVF_DEV_PATH,
			         dir_ent->d_name);
			dev_plug(buf);
		}
	}

	if (errno)
		return 1;

	return 0;
}

/*
 * We watch EVF_DEV_PATH for changes and filter out everything that's not
 * eventX file creation/deletion. To accomplish that we are using inotify
 * interface.
 */
int evf_hotplug_init(void (*device_plugged)(const char *dev),
                     void (*device_unplugged)(const char *dev))
{
	int fd, wd;

	dev_plug   = device_plugged;
	dev_unplug = device_unplugged;

	if (walk())
		return -1;

	fd = inotify_init();

	if (fd < 0)
		return -1;

	wd = inotify_add_watch(fd, EVF_DEV_PATH, IN_CREATE | IN_DELETE);

	if (wd < 0) {
		close(fd);
		return -1;
	}

	select_fd  = fd;

	return fd;
}

int evf_hotplug_rescan(void)
{
	char buf[EVF_BUF_LEN], str[EVF_BUF_LEN];
	int len, i, nr = 0;
	struct inotify_event *ev;

	len = read(select_fd, buf, EVF_BUF_LEN);

	if (len < 0)
		return -1;

	for (i = 0; i < len;) {

		ev = (struct inotify_event*) &buf[i];

		if (ev->len && !strncmp(ev->name, "event", 5)) {

			snprintf(str, EVF_BUF_LEN, "%s%s", EVF_DEV_PATH,
			         ev->name);

			if (dev_plug && ev->mask & IN_CREATE) {
				evf_msg(EVF_DEBUG, "Plugging new device '%s'.", str);
				dev_plug(str);
				nr++;
			}

			if (dev_unplug && ev->mask & IN_DELETE) {
				evf_msg(EVF_DEBUG, "Unplugging removed device '%s'.", str);
				dev_unplug(str);
				nr++;
			}
		}

		i += ev->len + sizeof(struct inotify_event);
	}

	return nr;
}

void evf_hotplug_exit(void)
{
	close(select_fd);
}
