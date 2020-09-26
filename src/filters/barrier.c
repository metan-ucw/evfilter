// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Evfilter barrier:
 *
 * holds events until sync event arrives
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/input.h>

#include "filter.h"
#include "filters.h"

struct barrier {
	unsigned int queue_index;
	unsigned int queue_size;
	struct input_event queue[];
};

static void flush_queue(struct evf_filter *self, struct barrier *barrier)
{
	unsigned int i;

	for (i = 0; i < barrier->queue_index; i++)
		evf_filter_process(self->next, &barrier->queue[i]);

	barrier->queue_index = 0;
}

static void insert_queue(struct barrier *barrier, struct input_event *ev)
{
	if (barrier->queue_index >= barrier->queue_size) {
		evf_msg(EVF_WARN, "Queue full, dropping event");
		return;
	}

	barrier->queue[barrier->queue_index++] = *ev;
}

static void barrier_process(struct evf_filter *self, struct input_event *ev)
{
	struct barrier *barrier = (struct barrier*) self->data;

	if (ev->type == EV_SYN && ev->code == SYN_REPORT) {
		flush_queue(self, barrier);
		evf_filter_process(self->next, ev);
	} else {
		insert_queue(barrier, ev);
	}
}

static struct evf_filter *barrier_from_json(json_object *json_data)
{
	int size = 20;

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "size")) {
			size = json_object_get_int(val);

			if (size < 0 || size > 1000) {
				evf_msg(EVF_DEBUG, "Invalid size %i", size);
				return NULL;
			}

		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON key_lock key %s", key);
			return NULL;
		}

	}

	return evf_barrier_alloc(size);
}

struct evf_filter_ops evf_barrier_ops = {
	.json_id = "barrier",
	.from_json = barrier_from_json,
	.process = barrier_process,
	.desc = "Buffers events until a sync event arrives"
};

struct evf_filter *evf_barrier_alloc(unsigned int buffer_size)
{
	struct evf_filter *filter;
	struct barrier *tmp;

	filter = evf_filter_alloc("barrier", sizeof(struct barrier)
	                          + buffer_size * sizeof(struct input_event));

	if (!filter)
		return NULL;

	filter->ops = &evf_barrier_ops;

	tmp = (struct barrier*) filter->data;
	tmp->queue_size = buffer_size;

	return filter;
}
