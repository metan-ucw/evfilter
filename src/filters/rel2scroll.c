// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 *
 * Evfilter rel2scroll:
 *
 * parameters:
 *
 * TriggerButton -- button that switch between scrolling and pointer movements.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <linux/input.h>

#include "key_parser.h"
#include "filter.h"
#include "filters.h"

struct rel2scroll {
	int x;
	int xmod;
	int y;
	int ymod;
	int  trigger_btn;
	bool trigger_on;
	bool eat_next_sync;
};

static void rel2scroll_process(struct evf_filter *self, struct input_event *ev)
{
	struct rel2scroll *data = (void*) self->data;

	if (ev->type == EV_SYN && data->eat_next_sync) {
		data->eat_next_sync = false;
		return;
	}

	data->eat_next_sync = false;

	if (ev->type == EV_KEY && ev->code == data->trigger_btn) {
		if (ev->value == 0)
			data->trigger_on = false;
		else
			data->trigger_on = true;

		data->eat_next_sync = true;

		return;
	}

	if (data->trigger_on) {
		if (ev->type == EV_REL) {
			switch (ev->code) {
				case REL_X:
					data->x += ev->value;

					if (data->x / data->xmod) {
						ev->value = data->x / data->xmod;
						data->x = data->x % data->xmod;
						ev->code = REL_HWHEEL;
					} else {
						data->eat_next_sync = true;
						return;
					}
				break;
				case REL_Y:
					data->y += ev->value;

					if (data->y / data->ymod) {
						ev->value = data->y / data->ymod;
						data->y = data->y % data->ymod;
						ev->code = REL_WHEEL;
					} else {
						data->eat_next_sync = true;
						return;
					}
				break;
			}
		}
	}

	evf_filter_process(self->next, ev);
}

static struct evf_filter *rel2scroll_from_json(json_object *json_data)
{
	int key_val = -1;
	int xmod = 1;
	int ymod = 1;

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "trigger_key")) {
			const char *key_str = json_object_get_string(val);

			key_val = keyparser_getkey(key_str);

			if (key_val == -1) {
				evf_msg(EVF_DEBUG, "Invalid key name %s", key_str);
				return NULL;
			}
		} else if (!strcmp(key, "xmod")) {
			xmod = json_object_get_int(val);
		} else if (!strcmp(key, "ymod")) {
			ymod = json_object_get_int(val);
		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON rel2scroll key %s", key);
			return NULL;
		}

	}

	if (key_val == -1) {
		evf_msg(EVF_DEBUG, "The key trigger must be set");
		return NULL;
	}

	return evf_rel2scroll_alloc(key_val, xmod, ymod);
}

struct evf_filter_ops evf_rel2scroll_ops = {
	.json_id = "rel2scroll",
	.from_json = rel2scroll_from_json,
	.process = rel2scroll_process,
	.desc = "Converts relative events into scroll events"
};

struct evf_filter *evf_rel2scroll_alloc(int trigger_btn, int xmod, int ymod)
{
	struct evf_filter *filter;
	struct rel2scroll *priv;

	filter = evf_filter_alloc("rel2scroll", sizeof(struct rel2scroll));

	if (!filter)
		return NULL;

	priv = (void*)filter->data;

	priv->trigger_on = false;
	priv->eat_next_sync = false;
	priv->x = 0;
	priv->xmod = xmod;
	priv->y = 0;
	priv->ymod = ymod;
	priv->trigger_btn = trigger_btn;

	return filter;
}
