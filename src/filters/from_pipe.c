// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Evfilter GetFromHandle:
 *
 * Parameters:
 *
 * HandleName string
 *
 * Note that the order of the input events may get messed up. Use barrier
 * filters to avoid this.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/input.h>

#include "evf_struct.h"
#include "evf_pipe.h"
#include "evf_msg.h"
#include "filters.h"

struct priv {
	struct evf_pipe *pipe;
};

static void from_pipe_process(struct evf_filter *self, struct input_event *ev)
{
	evf_filter_process(self->next, ev);
}

static void from_pipe_free(struct evf_filter *self)
{
	struct priv *priv = (struct priv*)self->data;

	evf_pipe_rem_sink(priv->pipe, self);
	evf_pipe_put(priv->pipe);

}
static struct evf_filter *from_pipe_from_json(json_object *json_data)
{
	const char *pipe = NULL;

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "pipe")) {
			pipe = json_object_get_string(val);
		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON from_pipe key %s", key);
			return NULL;
		}
	}

	if (!pipe) {
		evf_msg(EVF_ERR, "pipe has to be set!");
		return NULL;
	}

	return evf_from_pipe_alloc(pipe);
}

struct evf_filter_ops evf_from_pipe_ops = {
	.json_id = "from_pipe",
	.from_json = from_pipe_from_json,
	.process = from_pipe_process,
	.free = from_pipe_free,
	.desc = "Gets events from pipe",
};

struct evf_filter *evf_from_pipe_alloc(const char *name)
{
	struct evf_filter *self = malloc(sizeof(struct evf_filter)
	                                 + sizeof(struct priv));

	if (!self)
		return NULL;

	self->ops = &evf_from_pipe_ops;

	struct priv *priv = (struct priv*)self->data;

	priv->pipe = evf_pipe_get(name);
	if (!priv->pipe) {
		free(self);
		return NULL;
	}

	if (evf_pipe_add_sink(priv->pipe, self)) {
		evf_pipe_put(priv->pipe);
		free(self);
		return NULL;
	}

	return self;
}
