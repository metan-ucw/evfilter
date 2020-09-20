// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Left hand keyboard switched by caps lock.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>

#include "evf_struct.h"
#include "evf_msg.h"
#include "filters.h"

enum layout {
	LAYOUT_LEFT_HALF,
	LAYOUT_RIGHT_HALF,
	LAYOUT_SPECIAL,
	LAYOUT_MOUSE,
};

struct half_qwerty {
	int enabled;
	/* long press enables/disables the filter */
	int caps_pressed;
	int enabled_changed;
	struct timeval caps_press_time;

	int layout_switch;
	int caps_mod;
	int caps_on;
	int shift_on;

	/* mouse acceleration */
	int accel_x;
	int accel_y;
};

static void layout_right_half(struct input_event *ev)
{
	switch (ev->code) {
	/* Numbers row */
	case KEY_GRAVE:
		ev->code = KEY_MINUS;
	break;
	case KEY_1:
		ev->code = KEY_0;
	break;
	case KEY_2:
		ev->code = KEY_9;
	break;
	case KEY_3:
		ev->code = KEY_8;
	break;
	case KEY_4:
		ev->code = KEY_7;
	break;
	case KEY_5:
		ev->code = KEY_6;
	break;
	/* Top row */
	case KEY_Q:
		ev->code = KEY_P;
	break;
	case KEY_W:
		ev->code = KEY_O;
	break;
	case KEY_E:
		ev->code = KEY_I;
	break;
	case KEY_R:
		ev->code = KEY_U;
	break;
	case KEY_T:
		ev->code = KEY_Y;
	break;
	/* Middle row */
	case KEY_A:
		ev->code = KEY_SEMICOLON;
	break;
	case KEY_S:
		ev->code = KEY_L;
	break;
	case KEY_D:
		ev->code = KEY_K;
	break;
	case KEY_F:
		ev->code = KEY_J;
	break;
	case KEY_G:
		ev->code = KEY_H;
	break;
	/* Bottom row */
	case KEY_Z:
		ev->code = KEY_COMMA;
	break;
	case KEY_X:
		ev->code = KEY_M;
	break;
	case KEY_C:
		ev->code = KEY_N;
	break;
	case KEY_V:
		ev->code = KEY_B;
	break;
	/* Specialities */
	case KEY_SPACE:
		ev->code = KEY_BACKSPACE;
	break;
	}
}

/*
 * Movement keys are mapped to SDFE because we need our little finger free so
 * that it can reach caps and shift.
 */
static int layout_special(struct half_qwerty *priv, struct input_event *ev)
{
	//TODO Keep which keys have been pressed and release correct one regardless of shift key state
	switch (ev->code) {
	case KEY_S:
		if (priv->shift_on)
			ev->code = KEY_PAGEUP;
		else
			ev->code = KEY_LEFT;
	break;
	case KEY_D:
		if (priv->shift_on)
			ev->code = KEY_END;
		else
			ev->code = KEY_DOWN;
	break;
	case KEY_F:
		if (priv->shift_on)
			ev->code = KEY_PAGEDOWN;
		else
			ev->code = KEY_RIGHT;
	break;
	case KEY_E:
		if (priv->shift_on)
			ev->code = KEY_HOME;
		else
			ev->code = KEY_UP;
	break;
	case KEY_SPACE:
		ev->code = KEY_ENTER;
	break;
	case KEY_LEFTSHIFT:
		priv->shift_on = ev->value;
		return 1;
	break;
	}

	return 0;
}

static void rewrite(struct input_event *ev, int type, int code, int value)
{
	ev->type = type;
	ev->code = code;
	ev->value = value;
}

static void mouse_accel_x(struct half_qwerty *priv, int value)
{
	switch (value) {
	case 0:
		priv->accel_x = 1;
	break;
	case 2:
		priv->accel_x = MIN(priv->accel_x + 1, 10);
	break;
	}
}

static void mouse_accel_y(struct half_qwerty *priv, int value)
{
	switch (value) {
	case 0:
		priv->accel_y = 1;
	break;
	case 2:
		priv->accel_y = MIN(priv->accel_y + 1, 10);
	break;
	}
}

/*
 * Mouse emulation.
 */
static void layout_mouse(struct half_qwerty *priv, struct input_event *ev)
{
	switch (ev->code) {
	case KEY_S:
		mouse_accel_x(priv, ev->value);
		rewrite(ev, EV_REL, REL_X, -priv->accel_x);
	break;
	case KEY_D:
		mouse_accel_y(priv, ev->value);
		rewrite(ev, EV_REL, REL_Y, priv->accel_y);
	break;
	case KEY_F:
		mouse_accel_x(priv, ev->value);
		rewrite(ev, EV_REL, REL_X, priv->accel_x);
	break;
	case KEY_E:
		mouse_accel_y(priv, ev->value);
		rewrite(ev, EV_REL, REL_Y, -priv->accel_y);
	break;
	case KEY_SPACE:
		ev->code = BTN_LEFT;
	break;
	}
}

static void half_qwerty_process(struct evf_filter *self, struct input_event *ev)
{
	struct half_qwerty *priv = (struct half_qwerty*)self->data;

	if (ev->type == EV_KEY && ev->code == KEY_CAPSLOCK) {
		switch (ev->value) {
		case 1:
			priv->caps_press_time = ev->time;
		break;
		case 2:
			if (!priv->enabled_changed && ev->time.tv_sec - priv->caps_press_time.tv_sec > 2) {
				priv->enabled = !priv->enabled;

				/* turn off caps so that it's clear that the filter is enabled */
				if (priv->enabled) {
					ev->value = 0;
					evf_filter_process(self->next, ev);
					ev->value = 1;
					evf_filter_process(self->next, ev);
					ev->value = 0;
					evf_filter_process(self->next, ev);
				}

				priv->enabled_changed = 1;

				return;
			}
		break;
		case 0:
			if (priv->enabled_changed) {
				priv->enabled_changed = 0;
				return;
			}
		}
	}

	if (!priv->enabled) {
		evf_filter_process(self->next, ev);
		return;
	}

	if (ev->type == EV_KEY && ev->code == KEY_CAPSLOCK) {
		switch (ev->value) {
		case 1:
			priv->caps_on = 1;
		break;
		case 0:
			priv->caps_on = 0;
		break;
		case 2:
			return;
		}

		if (ev->value == 0) {
			if (priv->caps_mod == KEY_F)
				priv->layout_switch = LAYOUT_SPECIAL;
			else if (priv->caps_mod == KEY_D)
				priv->layout_switch = LAYOUT_MOUSE;
			else if (priv->layout_switch != LAYOUT_LEFT_HALF)
				priv->layout_switch = LAYOUT_LEFT_HALF;
			else
				priv->layout_switch = LAYOUT_RIGHT_HALF;

			priv->caps_mod = 0;
		}

		return;
	}

	if (priv->caps_on) {
		if (ev->type == EV_KEY && ev->value == 1)
			priv->caps_mod = ev->code;
		return;
	}

	if (ev->type == EV_KEY && priv->layout_switch == LAYOUT_RIGHT_HALF)
		layout_right_half(ev);

	if (ev->type == EV_KEY && priv->layout_switch == LAYOUT_SPECIAL) {
		if (layout_special(priv, ev))
			return;
	}

	if (ev->type == EV_KEY && priv->layout_switch == LAYOUT_MOUSE)
		layout_mouse(priv, ev);

	evf_filter_process(self->next, ev);
}

static struct evf_filter *half_qwerty_from_json(json_object *json_data)
{
	json_object_object_foreach(json_data, key, val) {
		evf_msg(EVF_DEBUG, "Invalid JSON half_qwerty key %s", key);
		return NULL;
	}

	return evf_half_qwerty_alloc();
}

struct evf_filter_ops evf_half_qwerty_ops = {
	.json_id = "half_qwerty",
	.process = half_qwerty_process,
	.from_json = half_qwerty_from_json,
	.desc = "Single hand keyboard"
};

struct evf_filter *evf_half_qwerty_alloc(void)
{
	struct evf_filter *self = malloc(sizeof(struct evf_filter) +
	                                 sizeof(struct half_qwerty));
	struct half_qwerty *priv;

	if (!self)
		return NULL;

	self->ops = &evf_half_qwerty_ops;

	priv = (struct half_qwerty*)self->data;

	priv->layout_switch = LAYOUT_LEFT_HALF;
	priv->caps_on = 0;
	priv->caps_mod = 0;
	priv->shift_on = 0;
	priv->accel_x = 1;
	priv->accel_y = 1;

	priv->enabled = 0;
	priv->enabled_changed = 0;

	return self;
}
