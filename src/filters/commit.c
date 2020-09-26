// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 *
 * Evfilter commit:
 *
 * Sends events back to the higher layer.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>

#include "filter.h"

struct commit {
	void (*commit)(struct input_event*, void *data);
	void *data;
};

static void commit_process(struct evf_filter *self, struct input_event *ev)
{
	struct commit *tmp = (struct commit*) self->data;

	tmp->commit(ev, tmp->data);

	if (self->next)
		evf_filter_process(self->next, ev);
}

static struct evf_filter_ops evf_commit_ops = {
	.json_id = "commit",
	.process = commit_process,
	.desc = "Commit filter",
};

struct evf_filter *evf_commit_alloc(void (*commit)(struct input_event*, void *data), void *data)
{
	struct evf_filter *filter = evf_filter_alloc("commit", sizeof(struct commit));
	struct commit *tmp;

	if (!filter)
		return NULL;

	filter->ops = &evf_commit_ops;

	tmp = (struct commit*)filter->data;

	tmp->commit = commit;
	tmp->data = data;

	return filter;
}
