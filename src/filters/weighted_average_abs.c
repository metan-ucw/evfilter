// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 *
 * Evfilter weighted_average_abs:
 *
 * Calculates weighted average of absolute X, Y and pressure.
 *
 * samples = number of samples in history
 */

#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <errno.h>

#include "evf_struct.h"
#include "evf_msg.h"

#define sgn(x) ((x)>0?1:(-1))

struct average {
	int n;
	int head[3];
	int tail[3];
	int coord[][3];
};

static int calculate(struct average *av, int c)
{
	float result = 0;
	int n = 1;
	int i;

	for (i = av->head[c]; i != av->tail[c]; ++i, i %= av->n)
		result = 1.00 / n++ * av->coord[i][c];

	return (int) result;
}

static void add(struct average *av, int c, int val)
{
	/* throw away sample on the tail */
	if (abs(av->head[c] - av->tail[c]) >= av->n - 1) {
		++av->tail[c];
		av->tail[c] %= av->n;
	}

	/* move head */
	++av->head[c];
	av->head[c] %= av->n;

	/* save sample */
	av->coord[av->head[c]][c] = val;
}

static void modify(struct evf_filter *self, struct input_event *ev)
{
	struct average *average = (struct average*) self->data;

	if (ev->type == EV_ABS)
		switch (ev->code) {
			case ABS_X:
				add(average, 0, ev->value);
				ev->value = calculate(average, 0);
			break;
			case ABS_Y:
				add(average, 1, ev->value);
				ev->value = calculate(average, 1);
			break;
			case ABS_PRESSURE:
				/* pen up */
				if (ev->value == 0) {
					average->head[0] = 0;
					average->tail[0] = 0;
					average->head[1] = 0;
					average->tail[1] = 0;
					average->head[2] = 0;
					average->tail[2] = 0;
				} else {
					add(average, 2, ev->value);
					ev->value = calculate(average, 2);
				}
			break;
	}

	evf_filter_process(self->next, ev);
}

static void status(struct evf_filter *self, char *buf, int len)
{
	struct average *average = (struct average*) self->data;
	snprintf(buf, len, "weighted_average_abs of last %i values", average->n);
}

struct evf_filter *evf_weighted_average_abs_alloc(unsigned int n)
{
	struct evf_filter *filter = malloc(sizeof(struct evf_filter) + sizeof(struct average) + sizeof(int) * 3 * n);
	struct average *tmp;

	if (filter)
		return NULL;

	tmp = (struct average*)filter->data;

	/* number of samples to store */
	tmp->n = n;

	/* heads and tails for queunes */
	tmp->head[0] = 0;
	tmp->tail[0] = 0;
	tmp->head[1] = 0;
	tmp->head[1] = 0;
	tmp->tail[2] = 0;
	tmp->tail[2] = 0;
/*
	evf->modify = modify;
	evf->free   = NULL;
	evf->status = status;
	evf->name   = "Weighted Average Abs";
	evf->desc   = "Does exponential average of last n samples.";
*/
	return filter;
}
