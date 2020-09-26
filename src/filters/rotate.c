// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 *
 * Evfilter rotate:
 *
 *  Exchange absolute or relative possitions events.
 *
 * parameters:
 *
 * rotate_abs_coords = bool
 * rotate_rel_coords = bool
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <linux/input.h>

#include "filter.h"
#include "filters.h"

struct rotate {
	int abs_coords;
	int rel_coords;
};

static void rotate_process(struct evf_filter *self, struct input_event *ev)
{
	struct rotate *rotate = (struct rotate*) self->data;

	if (ev->type == EV_REL && rotate->rel_coords) {
		switch (ev->code) {
			case REL_X:
				ev->code = REL_Y;
			break;
			case REL_Y:
				ev->code = REL_X;
			break;
		}
	}

	if (ev->type == EV_ABS && rotate->abs_coords) {
		switch (ev->code) {
			case ABS_X:
				ev->code = ABS_Y;
			break;
			case ABS_Y:
				ev->code = ABS_X;
			break;
		}
	}

	evf_filter_process(self->next, ev);
}

static struct evf_filter *rotate_from_json(json_object *json_data)
{
	int rotate_abs = 0, rotate_rel = 0;

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "rotate_abs")) {
			rotate_abs = json_object_get_boolean(val);
		} else if (!strcmp(key, "rotate_rel")) {
			rotate_rel = json_object_get_boolean(val);
		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON rotate_rel key %s", key);
			return NULL;
		}
	}

	if (!rotate_abs && !rotate_rel) {
		evf_msg(EVF_WARN, "At least one of rotate_abs or rotate_rel shuld be set");
		return NULL;
	}

	return evf_rotate_alloc(rotate_abs, rotate_rel);
}

struct evf_filter_ops evf_rotate_ops = {
	.json_id = "rotate",
	.from_json = rotate_from_json,
	.process = rotate_process,
	.desc = "Rotates relative and/or absolute coordinates",
};

struct evf_filter *evf_rotate_alloc(int rotate_abs_coords, int rotate_rel_coords)
{
	struct evf_filter *self = evf_filter_alloc("rotate", sizeof(struct rotate));
	struct rotate *priv;

	if (!self)
		return NULL;

	self->ops = &evf_rotate_ops;

	priv = (struct rotate*)self->data;

	priv->abs_coords = rotate_abs_coords;
	priv->rel_coords = rotate_rel_coords;

	return self;
}
