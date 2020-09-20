// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Evfilter daemon. Run filters on input devices accordingly to evfilter
  configuration.

 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include "evfilter.h"

#include "filters/evf_msg.h"
#include "evfd_lock.h"
#include "evf_io_queue.h"
#include "evfd.h"

static struct evf_io_queue *queue;

static void input_commit(struct input_event *ev, void *data)
{
	int uinput_fd = (int) data;
	write(uinput_fd, ev, sizeof (struct input_event));
}

/*
 * Data on input line, process them (may call line_commit callback).
 */
static int line_data(struct evf_io_queue_memb *self)
{
	if (evf_line_process(self->priv)) {
		evf_line_destroy(self->priv);
		return EVF_IO_QUEUE_REM;
	}

	return EVF_IO_QUEUE_OK;
}

/*
 * Returns 1 if device is created by evfd.
 */
static int our_input_device(const char *dev, char *name, int name_len)
{
	int fd = open(dev, O_RDONLY);

	if (fd < 0)
		return 1;

	evf_input_get_name(fd, name, name_len);

	close(fd);

	if (!strcmp(name, "uinput device"))
		return 1;

	return 0;
}

static void device_plugged(const char *dev)
{
	int fd, ret;
	struct uinput_user_dev dev_info;
	struct evf_line *line;
	union evf_err err;
	char name[128];

	if (our_input_device(dev, name, 128))
		return;

	evf_msg(EVF_INFO, "Device %s\t(%s).", dev, name);

	/*
	 * Create new input device for the other end of input line.
	 */
	//TODO: do this correctly
	memset(&dev_info, 0, sizeof(dev_info));
	strcpy(dev_info.name, "uinput device");

	fd = evf_uinput_create(&dev_info);

	if (fd < 0) {
		evf_msg(EVF_ERR, "evf_uinput_create() failed."
		                   "Do you have kernel capable of uinput and "
		                   "rights to write /dev/{input/,}uinput? : %i", fd);
		return;
	}

	/*
	 * Create input line (accordingly to evfilter configuration)
	 * without barrier filter.
	 *
	 * This should load all filters specified by evfilter system
	 * configuration into evfilter input line.
	 */
	//TODO: hack
	line = evf_line_create(dev, input_commit, (void*)fd, 0, &err, 0);

	if (line == NULL) {
		/* no filter configured for this input device */
		if (err.type == evf_ok) {
			evf_msg(EVF_DEBUG, "No evfilter configuration found.");
			evf_uinput_destroy(fd);
			return;
		} else {
			evf_err_print(&err); //TODO
			evf_uinput_destroy(fd);
			return;
		}
	}

	/* we have line add it into the queue */
	if (!evf_io_queue_add(queue, evf_line_fd(line), line_data, line)) {
		evf_msg(EVF_ERR, "Failed to add input line (%s, %s) to io queue.",
				dev, name);
		evf_uinput_destroy(fd);
		evf_line_destroy(line);
		return;
	}

	if ((ret = evf_input_grab(evf_line_fd(line))) != 0)
		evf_msg(EVF_ERR, "Failed to grab device %s '%s' (%i).",
				dev, name, ret);

	evf_msg(EVF_DEBUG, "Evfilter line for %s (%s) has been created.", dev, name);
	evf_filters_print(line->begin);
}

/*
 * Data on hotplug fd => generated hotplug events (calls device_plugged callback).
 */
static int hotplug_data(struct evf_io_queue_memb *self
                        __attribute__ ((unused)))
{
	//TODO: errors from evf_hotplug_rescan()
	evf_hotplug_rescan();

	return EVF_IO_QUEUE_OK;
}

static int looping = 1;

static void sighandler(int sig __attribute__ ((unused)))
{
	looping = 0;
}

int main(int argc, char *argv[])
{
	int fd;
	struct evf_io_queue_memb *i;
	int opt, daemonize = 1;

	evf_msg_init("evfd");

	while ((opt = getopt(argc, argv, "vdqh")) != -1) {
		switch (opt) {
			case 'h':
				puts(evfd_help);
				return 0;
			break;
			case 'v':
				evf_msg_verbosity_set(EVF_DEBUG);
			break;
			case 'q':
				evf_msg_verbosity_set(EVF_WARN);
			break;
			case 'd':
				daemonize = 0;
			break;
			default:
				puts(evfd_help);
				return 1;
		}
	}

	/* if there is evfd allready running exit */
	if (!evfd_try_lock())
		return 1;

	/* register handlers */
	signal(SIGQUIT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGINT, sighandler);

	/* create io queue for all fds */
	queue = evf_io_queue_new();

	if (queue == NULL) {
		evf_msg(EVF_ERR, "Can't allocate io queue.");
		return 1;
	}

	/* initalize hotplug input device watching */
	if ((fd = evf_hotplug_init(device_plugged, NULL)) < 0) {
		evf_msg(EVF_ERR, "Can't initalize hotplug: %s.",
		         strerror(errno));
		evf_io_queue_destroy(queue, 0);
		return 1;
	}

	/* create hotplug handler */
	if (!evf_io_queue_add(queue, fd, hotplug_data, NULL)) {
		evf_msg(EVF_ERR, "Can't allocate hotplug queue handler.");
		evf_io_queue_destroy(queue, 0);
		return 1;
	}

	/* initalization done, dameonize */
	if (daemonize) {
		evf_msg_output(EVF_SYSLOG, true);
		if (daemon(0,0) == -1) {
			evf_msg(EVF_ERR, "daemon() call failed: %s",
			         strerror(errno));
			return 1;
		}
		evf_msg_output(EVF_STDERR, false);
	}

	while (looping)
		evf_io_queue_wait(queue, NULL);

	evf_msg(EVF_NOTICE, "Got signal, exitting ...");

	/* cleanup */
	EVF_IO_QUEUE_MEMB_LOOP(queue, i) {
		if (i->priv != NULL) {
			struct evf_line *line = i->priv;
			int fd;
			/* ungrab device */
			evf_input_ungrab(line->fd);
			/* destroy evf line and get commit priv pointer */
			fd = (int) evf_line_destroy(line);
			evf_uinput_destroy(fd);
		}
	}

	evf_msg(EVF_INFO, "Exitting");
	evfd_release_lock();
	evf_msg_exit();

	return 0;
}
