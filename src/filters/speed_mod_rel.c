// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Evfilter speed_mod_rel:
 *
 * lower speed on relative pointer
 *
 * parameters:
 *
 * xmod = Integer
 *  modifier on x
 *
 * ymod = Integer
 *  modifier on y
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <errno.h>

#include "evf_struct.h"
#include "evf_msg.h"

#define sgn(x) ((x)>0?1:(-1))

struct relspeed {
	int x;
	int y;
	int xmod;
	int ymod;

	int eat_next_syn;
};

static void modify(struct evf_filter *self, struct input_event *ev)
{
	struct relspeed *relspeed = (struct relspeed*)self->data;

	/* eat right syn events */
	if (ev->type == EV_SYN && relspeed->eat_next_syn) {
		relspeed->eat_next_syn = 0;
		return;
	}

	if (ev->type == EV_REL)
		switch (ev->code) {
			case REL_X:
				relspeed->x += ev->value;
				if (abs(relspeed->x) >= relspeed->xmod) {
					ev->value = relspeed->x / relspeed->xmod;
					relspeed->x = sgn(relspeed->x)*(abs(relspeed->x) % relspeed->xmod);
				} else {
					relspeed->eat_next_syn = 1;
					return;
				}
			break;
			case REL_Y:
				relspeed->y += ev->value;
				if (abs(relspeed->y) >= relspeed->ymod) {
					ev->value = relspeed->y / relspeed->ymod;
					relspeed->y = sgn(relspeed->y)*(abs(relspeed->y) % relspeed->ymod);
				} else {
					relspeed->eat_next_syn = 1;
					return;
				}
			break;

	}

	evf_filter_process(self->next, ev);
}

struct evf_filter *evf_speed_mod_rel_alloc(int xmod, int ymod)
{
	struct evf_filter *filter = malloc(sizeof(struct evf_filter) +
	                                   sizeof(struct relspeed));
	struct relspeed *priv;

	if (!filter)
		return NULL;

	priv = (struct relspeed*)filter->data;

	priv->x = 0;
	priv->y = 0;
	priv->xmod = xmod;
	priv->ymod = ymod;

	priv->eat_next_syn = 0;
/*
	evf->modify = modify;
	evf->free   = NULL;
	evf->status = status;
	evf->name   = "SpeedMod";
	evf->desc   = "Slows down your mouse.";
*/
	return filter;
}
