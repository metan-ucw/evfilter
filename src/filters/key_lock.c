// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/input.h>

#include "key_parser.h"
#include "filter.h"
#include "filters.h"

struct priv {
	int key;
	int state;
	int eat_next_sync;
};

static void key_lock_process(struct evf_filter *self, struct input_event *ev)
{
	struct priv *priv = (struct priv*) self->data;

	if (priv->eat_next_sync) {
		priv->eat_next_sync = 0;

		if (ev->type == 0 && ev->code == 0 && ev->value == 0)
			return;
	}

	if (ev->type == EV_KEY && ev->code == priv->key) {
		/* key released */
		if (ev->value == 0) {
			priv->eat_next_sync = 1;
			return;
		}

		/* key pressed */
		if (ev->value == 1) {
			if (priv->state == 0)
				priv->state = 1;
			else {
				priv->state = 0;
				ev->value = 0;
			}
		}
	}

	evf_filter_process(self->next, ev);
}

static struct evf_filter *key_lock_from_json(json_object *json_data)
{
	int key_val = -1;

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "key")) {
			const char *key_str = json_object_get_string(val);

			key_val = keyparser_getkey(key_str);

			if (key_val == -1) {
				evf_msg(EVF_DEBUG, "Invalid key name %s", key_str);
				return NULL;
			}
		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON key_lock key %s", key);
			return NULL;
		}

	}

	return evf_key_lock_alloc(key_val);
}

struct evf_filter_ops evf_key_lock_ops = {
	.json_id = "key_lock",
	.from_json = key_lock_from_json,
	.process = key_lock_process,
	.desc = "Locks key pressed on press, releases it on second press",
};

struct evf_filter *evf_key_lock_alloc(int key)
{
	struct evf_filter *filter = evf_filter_alloc("key_lock", sizeof(struct priv));
	struct priv *priv;

	if (!filter)
		return NULL;

	filter->ops = &evf_key_lock_ops;

	priv = (struct priv*)filter->data;

	priv->key = key;
	priv->state = 0;
	priv->eat_next_sync = 0;

	return filter;
}
