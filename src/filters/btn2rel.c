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

struct btn2rel {
	/* movement keys */
	int key_up;
	int key_down;
	int key_left;
	int key_right;
	/* mouse button keys */
	int right_btn;
	int middle_btn;
	int left_btn;

	int eat_next_syn;
};

static void btn2rel_process(struct evf_filter *self, struct input_event *ev)
{
	struct btn2rel *data = (struct btn2rel*) self->data;

	/* eat syn event after key_up */
	if (ev->type == EV_SYN && data->eat_next_syn) {
		data->eat_next_syn = 0;
		return;
	}

	if (ev->type == EV_KEY) {
		if (ev->code == data->key_up) {

			/* ignore key up events */
			if (ev->value == 0) {
				data->eat_next_syn = 1;
				return;
			}

			ev->type  = EV_REL;
			ev->code  = REL_Y;
			ev->value = 2;
		}

		if (ev->code == data->key_down) {
			/* ignore key up events */
			if (ev->value == 0) {
				data->eat_next_syn = 1;
				return;
			}

			ev->type  = EV_REL;
			ev->code  = REL_Y;
			ev->value = -2;
		}

		if (ev->code == data->key_left) {
			/* ignore key up events */
			if (ev->value == 0) {
				data->eat_next_syn = 1;
				return;
			}

			ev->type  = EV_REL;
			ev->code  = REL_X;
			ev->value = -2;
		}

		if (ev->code == data->key_right) {
			/* ignore key up events */
			if (ev->value == 0) {
				data->eat_next_syn = 1;
				return;
			}

			ev->type  = EV_REL;
			ev->code  = REL_X;
			ev->value = 2;
		}

		if (ev->code == data->left_btn)
			ev->code = BTN_LEFT;

		if (ev->code == data->middle_btn)
			ev->code = BTN_MIDDLE;

		if (ev->code == data->right_btn)
			ev->code = BTN_RIGHT;

	}

	evf_filter_process(self->next, ev);
}

static int get_key(json_object *val)
{
	const char *key_str = json_object_get_string(val);
	int key_val = keyparser_getkey(key_str);

	if (key_val == -1)
		evf_msg(EVF_DEBUG, "Invalid key name %s", key_str);

	return key_val;
}

static struct evf_filter *btn2rel_from_json(json_object *json_data)
{
	int key_down = 0;
	int key_up = 0;
	int key_left = 0;
	int key_right = 0;
	int left_btn = 0;
	int middle_btn = 0;
	int right_btn = 0;

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "key_up")) {
			key_up = get_key(val);
			if (key_up == -1)
				return NULL;
		} else if (!strcmp(key, "key_down")) {
			key_down = get_key(val);
			if (key_down == -1)
				return NULL;
		} else if (!strcmp(key, "key_left")) {
			key_left = get_key(val);
			if (key_left == -1)
				return NULL;
		} else if (!strcmp(key, "key_right")) {
			key_right = get_key(val);
			if (key_right == -1)
				return NULL;
		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON btn2rel key %s", key);
			return NULL;
		}
	}

	return evf_btn2rel_alloc(key_down, key_up, key_left, key_right, left_btn, middle_btn, right_btn);
}

struct evf_filter_ops evf_btn2rel_ops = {
	.json_id = "btn2rel",
	.from_json = btn2rel_from_json,
	.process = btn2rel_process,
	.desc = "Translates key presses into a mouse movements",
};

struct evf_filter *evf_btn2rel_alloc(int key_down, int key_up, int key_left, int key_right, int left_btn, int middle_btn, int right_btn)
{
	struct evf_filter *filter = evf_filter_alloc("btn2rel", sizeof(struct btn2rel));
	struct btn2rel *priv;

	if (!filter)
		return NULL;

	priv = (struct btn2rel*)filter->data;

	priv->key_up = key_up;
	priv->key_down = key_down;
	priv->key_left = key_left;
	priv->key_right = key_right;
	priv->left_btn = left_btn;
	priv->middle_btn = middle_btn;
	priv->right_btn = right_btn;
	priv->eat_next_syn = 0;

	filter->ops = &evf_btn2rel_ops;

	return filter;
}
