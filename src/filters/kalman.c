// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 *
 * Evfilter kalman:
 *
 * Uses kalman filter with gain kx and ky.
 *
 * kx -- gain for x
 * ky -- gain for y
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/input.h>

#include "evf_struct.h"
#include "evf_msg.h"
#include "filters.h"

struct kalman {
	float kx;
	float ky;

	float x, vx;
	float y, vy;

	int reset_x;
	int reset_y;
};

static void kalman_process(struct evf_filter *self, struct input_event *ev)
{
	struct kalman *kalman = (struct kalman*) self->data;

	if (ev->type == EV_ABS) {
		switch (ev->code) {
			case ABS_X:
				if (kalman->reset_x) {
					kalman->x = ev->value;
					kalman->reset_x = 0;
				} else {
					float est_x = kalman->x + kalman->vx;
					float new_x = est_x + kalman->kx * (ev->value - est_x);
					kalman->vx  = kalman->vx/10 - 0.1 * (est_x - ev->value);
					kalman->x = new_x;
					ev->value = (int) new_x;
				}
			break;
			case ABS_Y:
				if (kalman->reset_y) {
					kalman->y = ev->value;
					kalman->reset_y = 0;
				} else {
					float est_y = kalman->y + kalman->vy;
					float new_y = est_y + kalman->ky * (ev->value - est_y);
					kalman->vy = kalman->vy/10 - 0.1 * (est_y - ev->value);
					kalman->y = new_y;
					ev->value = (int) new_y;
				}
			break;
			case ABS_PRESSURE:
				/* pen up */
				if (ev->value == 0) {
					kalman->reset_x = 1;
					kalman->reset_y = 1;
					kalman->vx = 0;
					kalman->vy = 0;
				}
			break;
		}
	}

	evf_filter_process(self->next, ev);
}

static struct evf_filter *kalman_from_json(json_object *json_data)
{
	float kx = 1, ky = 1;

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "gain_x")) {
			kx = json_object_get_double(val);
		} else if (!strcmp(key, "gain_y")) {
			ky = json_object_get_double(val);
		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON kalman key %s", key);
			return NULL;
		}

	}

	return evf_kalman_alloc(kx, ky);
}

struct evf_filter_ops evf_kalman_ops = {
	.json_id = "kalman",
	.process = kalman_process,
	.from_json = kalman_from_json,
	.desc = "Kalman filter for touchscreen"
};

struct evf_filter *evf_kalman_alloc(float gain_x, float gain_y)
{
	struct evf_filter *evf = malloc(sizeof (struct evf_filter) + sizeof (struct kalman));
	struct kalman *kalman;

	if (!evf)
		return NULL;

	kalman = (struct kalman*)evf->data;

	kalman->kx = gain_x;
	kalman->ky = gain_y;
	kalman->vx = 0;
	kalman->vy = 0;
	kalman->reset_x = 1;
	kalman->reset_y = 1;

	evf->ops = &evf_kalman_ops;

	return evf;
}
