// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 *
 * Evfilter abs2rel:
 *
 * Translates absolute events to relative accordingly to difference of
 * abs coordinates.
 *
 * parameters:
 *
 * none
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>

#include "filters/filters.h"
#include "evf_struct.h"
#include "evf_msg.h"

#define INCIF(x) ((x) < 2 ? ((x)++) : (x));

struct abs2rel {
	int xstate;
	int ystate;
	int relx;
	int rely;
	int last_absx;
	int last_absy;
};

static void abs2rel_process(struct evf_filter *self, struct input_event *ev)
{
	struct abs2rel *data = (struct abs2rel*) self->data;

	/* flush what we have */
	if (ev->type == EV_SYN) {
		if (data->xstate == 2) {
			ev->code  = REL_X;
			ev->value = data->relx;
			evf_filter_process(self->next, ev);
		}

		if (data->ystate == 2) {
			ev->code  = REL_Y;
			ev->value = data->rely;
			evf_filter_process(self->next, ev);
		}

		ev->type  = EV_SYN;
		ev->value = 0;
		evf_filter_process(self->next, ev);
	}

	if (ev->type == EV_ABS)
		switch (ev->code) {
			case ABS_X:
				data->relx = ev->value - data->last_absx;
				data->last_absx = ev->value;
				INCIF(data->xstate);
			break;
			case ABS_Y:
				data->rely = ev->value - data->last_absy;
				data->last_absy = ev->value;
				INCIF(data->ystate);
			break;
			case ABS_PRESSURE:
				/* pen up */
				if (ev->value == 0) {
					data->xstate = 0;
					data->ystate = 0;
				}
			break;
	} else
		evf_filter_process(self->next, ev);
}

static struct evf_filter *abs2rel_from_json(json_object *json_data)
{
	json_object_object_foreach(json_data, key, val) {
		evf_msg(EVF_DEBUG, "Invalid JSON no_repeat key %s", key);
		return NULL;
	}

	return evf_abs2rel_alloc();
}

struct evf_filter_ops evf_abs2rel_ops = {
	.json_id = "abs2rel",
	.process = abs2rel_process,
	.from_json = abs2rel_from_json,
	.desc = "Causes absolute touchscreen to behave as relative mouse"
};

struct evf_filter *evf_abs2rel_alloc(void)
{
	struct evf_filter *evf = malloc(sizeof (struct evf_filter) + sizeof (struct abs2rel));
	struct abs2rel *data;

	if (!evf)
		return NULL;

	evf->ops = &evf_abs2rel_ops;

	data = (struct abs2rel*)evf->data;

	data->xstate = 0;
	data->ystate = 0;

	return evf;
}
