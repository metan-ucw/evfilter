// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Evfilter: pressure_to_key
 *
 * Generates button events from pressure.
 *
 * parameters:
 *
 * treshold = Integer
 *  treshold value, when pressure exceeds this value BTN_DOWN is generated
 *  If pressure is goes under treshold BTN_UP is generated.
 *
 * key = key
 *  Name of the key, to be generated.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>

#include "evf_struct.h"
#include "evf_msg.h"
#include "key_parser.h"
#include "filters.h"

struct pressure {
	int treshold;
	int key;

	int pressed;
};

static int check_treshold(int value, int treshold)
{
	if (treshold < 0) {
		if (value == 0)
			return 0;
		return value < abs(treshold);
	}

	return value > treshold;
}

/*
 * Generate key down, up when treshold was reached.
 * TODO: should we eat pressure events?
 *       generate sync event!
 */
static void pressure_to_key_process(struct evf_filter *self, struct input_event *ev)
{
	struct pressure *p = (struct pressure*) self->data;

	evf_filter_process(self->next, ev);

	if (ev->type == EV_ABS && ev->code == ABS_PRESSURE) {
		if (check_treshold(ev->value, p->treshold)) {
			if (!p->pressed) {

				p->pressed = 1;
				ev->type   = EV_KEY;
				ev->code   = p->key;
				ev->value  = 1;

				evf_filter_process(self->next, ev);
			}
		} else {
			if (p->pressed) {
				p->pressed = 0;
				ev->type   = EV_KEY;
				ev->code   = p->key;
				ev->value  = 0;

				evf_filter_process(self->next, ev);
			}
		}
	}
}

static struct evf_filter *pressure_to_key_from_json(json_object *json_data)
{
	int key_val = -1;
	int threshold = 0;

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "key")) {
			const char *key_str = json_object_get_string(val);

			key_val = keyparser_getkey(key_str);

			if (key_val == -1) {
				evf_msg(EVF_DEBUG, "Invalid key name %s", key_str);
				return NULL;
			}
		} else if (!strcmp(key, "threshold")) {
			threshold = json_object_get_int(val);
		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON key_lock key %s", key);
			return NULL;
		}

	}

	return evf_pressure_to_key_alloc(threshold, key_val);
}

struct evf_filter_ops evf_pressure_to_key_ops = {
	.json_id = "pressure_to_key",
	.from_json = pressure_to_key_from_json,
	.process = pressure_to_key_process,
	.desc = "Converts touchscreen pressure to key events"
};

struct evf_filter *evf_pressure_to_key_alloc(int treshold, int key)
{
	struct evf_filter *filter = malloc(sizeof(struct evf_filter) + sizeof(struct pressure));
	struct pressure *priv;

	if (!filter)
		return NULL;

	filter->ops = &evf_pressure_to_key_ops;

	priv = (struct pressure*)filter->data;

	priv->treshold = treshold;
	priv->key      = key;
	priv->pressed  = 0;

	return filter;
}
