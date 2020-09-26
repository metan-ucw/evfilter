// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 *
 * Evfilter FilterToHandle:
 *
 * Send one type of events to pipe.
 *
 * Parameters:
 * EventType  evtype
 * EventCode  int
 * HandleName string
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <linux/input.h>

#include "evf_pipe.h"
#include "filter.h"
#include "filters.h"

struct priv {
	int type;
	int code;
	int sync_flag;
	int ev_flag;
	struct evf_pipe *pipe;
};

static void to_pipe_process(struct evf_filter *self, struct input_event *ev)
{
	struct priv *priv = (struct priv*)self->data;

	if (ev->type == 0 && ev->code == 0 && ev->value == 0
	    && priv->sync_flag) {

		evf_pipe_process(priv->pipe, ev);

		if (priv->ev_flag)
			evf_filter_process(self->next, ev);

		return;
	}

	if (ev->type == priv->type && ev->code == priv->code) {
		evf_pipe_process(priv->pipe, ev);
		priv->sync_flag = 1;
		priv->ev_flag = 0;
	} else {
		evf_filter_process(self->next, ev);
		priv->ev_flag = 1;
	}
}

static struct evf_filter *to_pipe_from_json(json_object *json_data)
{
	const char *pipe = NULL;

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "pipe")) {
			pipe = json_object_get_string(val);
		} else if (!strcmp(key, "type")) {
		//	key_val = keyparser_getkey(key_str);

//			if (key_val == -1) {
//				evf_msg(EVF_DEBUG, "Invalid key name %s", key_str);
//				return NULL;
//			}
		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON to_pipe key %s", key);
			return NULL;
		}
	}

	if (!pipe) {
		evf_msg(EVF_ERR, "pipe has to be set!");
		return NULL;
	}

	return evf_filter_to_pipe_alloc(pipe, 0, 0);
}

static void to_pipe_free(struct evf_filter *self)
{
	struct priv *priv = (struct priv*)self->data;

	evf_pipe_put(priv->pipe);
}

struct evf_filter_ops evf_filter_to_pipe_ops = {
	.json_id = "filter_to_pipe",
	.from_json = to_pipe_from_json,
	.process = to_pipe_process,
	.free = to_pipe_free,
	.desc = "Sends subset of events to a pipe",
};

struct evf_filter *evf_filter_to_pipe_alloc(const char *name, int type, int code)
{
	struct evf_filter *filter = evf_filter_alloc("filter_to_pipe", sizeof(struct priv));
	struct priv *priv;

	if (!filter)
		return NULL;

	priv = (struct priv*)filter->data;

	priv->pipe = evf_pipe_get(name);

	if (!priv->pipe) {
		free(filter);
		return NULL;
	}

	filter->ops = &evf_to_pipe_ops;

	priv->type = type;
	priv->code = code;
	priv->sync_flag = 0;

	return filter;
}
