// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
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

static void to_pipe_process(struct evf_filter *self, struct input_event *ev)
{
	struct priv *priv = (struct priv*)self->data;

	evf_pipe_process(priv->pipe, ev);
	evf_filter_process(self->next, ev);
}

static void to_pipe_free(struct evf_filter *self)
{
	struct priv *priv = (struct priv*)self->data;

	evf_pipe_put(priv->pipe);
}

static struct evf_filter *to_pipe_from_json(json_object *json_data)
{
	const char *pipe = NULL;

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "pipe")) {
			pipe = json_object_get_string(val);
		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON to_pipe key %s", key);
			return NULL;
		}
	}

	if (!pipe) {
		evf_msg(EVF_ERR, "pipe has to be set!");
		return NULL;
	}

	return evf_to_pipe_alloc(pipe);
}

struct evf_filter_ops evf_to_pipe_ops = {
	.json_id = "to_pipe",
	.from_json = to_pipe_from_json,
	.process = to_pipe_process,
	.free = to_pipe_free,
	.desc = "Sends events to a pipe",
};

struct evf_filter *evf_to_pipe_alloc(const char *name)
{
	struct evf_filter *filter = malloc(sizeof(struct evf_filter) +
	                                   sizeof(struct priv));
	struct priv *priv;

	if (!filter)
		return NULL;

	priv = (struct priv*)filter->data;

	priv->pipe = evf_pipe_get(name);

	if (!priv->pipe) {
		free(filter);
		return NULL;
	}

	return filter;
}
