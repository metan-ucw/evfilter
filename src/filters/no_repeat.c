// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 *
 * Evfilter no_repeat:
 *
 * Ignores repeat events on all keys.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>

#include "filters/filters.h"
#include "evf_struct.h"
#include "evf_msg.h"

struct no_repeat {
	int eat_next_sync;
};

static void no_repeat_process(struct evf_filter *self, struct input_event *ev)
{
	struct no_repeat *priv = (void*)self->data;

	if (priv->eat_next_sync) {
		priv->eat_next_sync = 0;

		if (ev->type == 0 && ev->code == 0 && ev->value == 0)
			return;
	}

	if (ev->type == EV_KEY && ev->value == 2) {
		priv->eat_next_sync = 1;
		return;
	}

	evf_filter_process(self->next, ev);
}

static struct evf_filter *no_repeat_from_json(json_object *json_data)
{
	json_object_object_foreach(json_data, key, val) {
		evf_msg(EVF_DEBUG, "Invalid JSON no_repeat key %s", key);
		return NULL;
	}

	return evf_no_repeat_alloc();
}

struct evf_filter_ops evf_no_repeat_ops = {
	.json_id = "no_repeat",
	.process = no_repeat_process,
	.from_json = no_repeat_from_json,
	.desc = "Removes all repeat events",
};

struct evf_filter *evf_no_repeat_alloc(void)
{
	struct evf_filter *evf = malloc(sizeof(struct evf_filter) +
	                                sizeof(struct no_repeat));
	struct no_repeat *priv;

	if (!evf)
		return NULL;

	priv = (void*)evf->data;

	priv->eat_next_sync = 0;
	evf->ops = &evf_no_repeat_ops;

	return evf;
}
