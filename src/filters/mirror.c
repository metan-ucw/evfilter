// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 *
 * Evfilter mirror:
 *
 *  Mirror absolute or relative possitions events.
 *
 *  NOTE: if you want mirror absolute possitions this filter must be loaded after scale_abs.
 *
 * parameters:
 *
 * mir_rel_x = bool
 * mir_rel_y = bool
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/input.h>

#include "filter.h"
#include "filters.h"

struct mirror {
	int rel_x;
	int rel_y;
};

static void mirror_process(struct evf_filter *self, struct input_event *ev)
{
	struct mirror *mirror = (struct mirror*)self->data;

	if (ev->type == EV_REL) {
		switch (ev->code) {
			case REL_X:
				if (mirror->rel_x)
					ev->value *= -1;
			break;
			case REL_Y:
				if (mirror->rel_y)
					ev->value *= -1;
			break;
		}
	}

	evf_filter_process(self->next, ev);
}

static struct evf_filter *mirror_from_json(json_object *json_data)
{
	int mirror_x = 0, mirror_y = 0;

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "mirror_x")) {
			mirror_x = json_object_get_boolean(val);
		} else if (!strcmp(key, "mirror_y")) {
			mirror_y = json_object_get_boolean(val);
		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON rotate_rel key %s", key);
			return NULL;
		}
	}

	if (!mirror_x && !mirror_y) {
		evf_msg(EVF_WARN, "At least one of rotate_abs or rotate_rel shuld be set");
		return NULL;
	}

	return evf_mirror_alloc(mirror_x, mirror_y);
}

struct evf_filter_ops evf_mirror_ops = {
	.json_id = "mirror",
	.from_json = mirror_from_json,
	.process = mirror_process,
	.desc   = "Mirrors relative events"
};

struct evf_filter *evf_mirror_alloc(int mirror_x, int mirror_y)
{
	struct evf_filter *filter = evf_filter_alloc("mirror", sizeof(struct mirror));
	struct mirror *tmp;

	if (!filter)
		return NULL;

	tmp = (struct mirror*)filter->data;

	tmp->rel_x = mirror_x;
	tmp->rel_y = mirror_y;

	return filter;
}
