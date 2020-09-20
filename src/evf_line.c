// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <stdio.h>

#include "evf_line.h"

#include "filters/filters.h"
#include "evf_filter.h"
#include "evf_err.h"
#include "evf_profile.h"
#include "evf_input.h"

#include "evf_struct.h"
#include "filters/evf_msg.h"

/*
 * Read events from file descriptor and pass them to filter line
 */
int evf_line_process(struct evf_line *line)
{
	struct input_event ev;

	/*
	 * Read can teoretically throw sigpipe, but here it IMHO shouldn't
	 * happend unless kernel has crashed.
	 */
	if (read(line->fd, &ev, sizeof (struct input_event)) < 0) {
		if (errno == ENODEV)	{
			evf_msg(EVF_NOTICE,"Device %s is gone", line->input_device);
			return -1;
		}
		else if (errno != EAGAIN )	{
			evf_msg(EVF_ERR,"Error reading config file: %s", strerror(errno));
			return -1;
		}
	}

	evf_line_process_event(line->begin, &ev);

	return 0;
}

/*
 * Opens input device, tries ioctl.
 */
static int open_input_device(const char *input_device, union evf_err *err)
{
	int fd, version;

	/* open input device */
	fd = open(input_device, O_RDONLY);

	if (fd < 0) {
		//TODO: context
		err->type = evf_errno;
		err->err_no.err_no = errno;
		evf_msg(EVF_ERR,"Cannot open device '%s': %s", input_device, strerror(errno));
	}

	/* doesn't understand ioctl => not an input device */
	if (evf_input_get_version(fd, &version)) {
		close(fd);
		//TODO: fill evf_err
	}

	return fd;
}


/*
 * Create input line
 */
struct evf_line *evf_line_create(const char *input_device,
                                 void (*commit)(struct input_event *ev,
				                void *priv),
				 void *priv, unsigned int use_barriers,
				 union evf_err *err, int even_empty)
{
	struct evf_line   *line;
	struct evf_filter *fcommit;
	struct evf_filter *begin;
	struct evf_filter *fbarrier = NULL;
	int fd;

	fd = open_input_device(input_device, err);

	if (fd < 0)
		return NULL;

	begin = evf_load_system_profile(fd, err);

	/* err is filled from evf_load_system_profile */
	if (err->type != evf_ok) {
		close(fd);
		return NULL;
	}

	/* we should not create empty line */
	if (!even_empty && begin == NULL) {
		err->type = evf_ok;
		close(fd);
		return NULL;
	}

	/* let's the allocation begins ;) */
	line = malloc(sizeof (struct evf_line) + strlen(input_device) + 1);

	if (line == NULL) {
		err->type = evf_errno;
		err->err_no.err_no = errno;
		evf_filters_free(begin);
		close(fd);
		evf_msg(EVF_ERR, "Allocating error");
		return NULL;
	}

	/* load system profiles */
	line->begin = begin;
	line->end   = evf_filters_last(begin);

	fcommit = evf_commit_alloc(commit, priv);

	if (use_barriers)
		fbarrier = evf_barrier_alloc(use_barriers);

	if (fcommit == NULL || (use_barriers && fbarrier == NULL)) {
		err->type = evf_errno;
		err->err_no.err_no = errno;
		free(fcommit);
		free(fbarrier);
		evf_filters_free(line->begin);
		free(line);
		close(fd);
		return NULL;
	}

	/* build the structure in memory */
	line->fd = fd;

	strcpy(line->input_device, input_device);

	if (use_barriers)
		fbarrier->next = fcommit;
	else
		fbarrier       = fcommit;

	if (line->begin == NULL)
		line->begin     = fbarrier;
	else
		line->end->next = fbarrier;

	return line;
}

/*
 * Inserts filter at the end of linked list of filters, but before commit and barrier.
 */
void evf_line_attach_filter(struct evf_line *line, struct evf_filter *filter)
{
	/* just commit and poissibly barrier */
	if (line->end == NULL) {
		filter->next    = line->begin;
		line->begin     = filter;
		line->end       = filter;
	} else {
		filter->next    = line->end->next;
		line->end->next = filter;
	}
}


/*
 * Start processing for event ev
 */
void evf_line_process_event(struct evf_filter *root, struct input_event *ev)
{
	evf_filter_process(root, ev);
}

/*
 * Free all evfilters in line
 */
void *evf_line_destroy(struct evf_line *line)
{
	evf_filters_free(line->begin);
	close(line->fd);
	free(line);

	return NULL;
}

int evf_line_fd(struct evf_line *line)
{
	return line->fd;
}

/*
 * Debug function, prints all filters in line to stdout.
 */
void evf_line_print(struct evf_line *line)
{
	char name[256];

	evf_input_get_name(line->fd, name, 256);

	evf_msg(EVF_INFO,"Filter line for input %s (%s)", line->input_device, name);

	evf_filters_print(line->begin);
}
