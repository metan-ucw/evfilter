// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  This is simple program using sdl as graphics driver and evfilter for input
  event filtering.

 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include "evfilter.h"

#include "sdl_utils.h"

#define X_RES 480
#define Y_RES 640

static SDL_Surface *scr;
static struct evf_io_queue *queue;
static struct sdl_scroll_buf *events_sb;
static struct sdl_scroll_buf *hotplug_sb;

int init_sdl(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return 0;

	scr = SDL_SetVideoMode(X_RES, Y_RES, 32, SDL_HWSURFACE);
	SDL_WM_SetCaption("Draw", "Draw");

	return 1;
}

struct pointer {
	int x;
	int old_x;
	int y;
	int old_y;
	int color;
};

/*
 * Here we parse event as it came
 */
static int read_event(struct evf_io_queue_memb *self)
{
	struct evf_line *line = self->priv;

	if (evf_line_process(line) < 0) {
		sdl_scroll_buf_add(hotplug_sb, "read failed; removing...", 0xaa2222ff);
		return EVF_IO_QUEUE_REM | EVF_IO_QUEUE_DFREE;
	}

	return EVF_IO_QUEUE_OK;
}

/*
 * Put event into scroll box
 */
static void print_event(struct input_event *ev, uint32_t color)
{
	const char *type, *code, *value;
	char buf[128];

	type  = evf_input_type(ev);
	code  = evf_input_code(ev);
	value = evf_input_value(ev);

	if (value != NULL)
		snprintf(buf, 128, "T: %s C: %s V: %s", type, code, value);
	else
		snprintf(buf, 128, "T: %s C: %s V: %i", type, code, ev->value);

	sdl_scroll_buf_add(events_sb, buf, color);
}

static void commit(struct input_event *ev, void *data)
{
	struct pointer *dev = data;

	switch (ev->type) {
		case EV_REL:
			switch (ev->code) {
				case REL_X:
					dev->x += ev->value;
				break;
				case REL_Y:
					dev->y += ev->value;
				break;
			}
		break;
		case EV_ABS:
			switch (ev->code) {
				case ABS_X:
					dev->x = 1.00 * X_RES * ev->value / INT_MAX;
				break;
				case ABS_Y:
					dev->y = 1.00 * Y_RES * ev->value / INT_MAX;
				break;
			}
		case EV_SYN:
			lineColor(scr, dev->x, dev->y, dev->old_x, dev->old_y, dev->color);
			dev->old_x = dev->x;
			dev->old_y = dev->y;
		break;
	}

	print_event(ev, dev->color);
	SDL_UpdateRect(scr, 0, 0, X_RES, Y_RES);
}


static int read_hotplug(struct evf_io_queue_memb *self)
{
	evf_hotplug_rescan();

	return EVF_IO_QUEUE_OK;
}

static void device_plugged(const char *dev)
{
	struct pointer *ptr;
	struct evf_line *line;
	union evf_err err;
	uint32_t color = 0x000000ff | (random() % 0xff0000);
	char buf[128];

	snprintf(buf, 128, "--> %s", dev);
	sdl_scroll_buf_add(hotplug_sb, buf, color);

	ptr = malloc(sizeof(struct pointer));

	if (ptr == NULL) {
		fprintf(stderr, "Can't allocate memory.\n");
		return;
	}

	ptr->x = X_RES/2;
	ptr->y = Y_RES/2;
	ptr->old_x = X_RES/2;
	ptr->old_y = Y_RES/2;
	ptr->color = color;


	line = evf_line_create(dev, commit, ptr, 10, &err, 1);

	if (err.type != evf_ok) {
		free(ptr);
		evf_err_print(&err);
		return;
	}

	evf_input_get_name(line->fd, buf, 128);
	sdl_scroll_buf_add(hotplug_sb, buf, color);

	SDL_UpdateRect(scr, 0, 0, X_RES, Y_RES);

	evf_io_queue_add(queue, line->fd, read_event, line);

	evf_line_print(line);
}

static void device_unplugged(const char *dev)
{
	char buf[128];

	snprintf(buf, 128, "<-- %s", dev);
	sdl_scroll_buf_add(hotplug_sb, buf, 0xaa2222ff);
	SDL_UpdateRect(scr, 0, 0, X_RES, Y_RES);
}

int main(void)
{
	union evf_err err;
	int fd;

	if (!init_sdl()) {
		fprintf(stderr, "SDL init failed\n");
		return 1;
	}

	events_sb  = sdl_scroll_buf_new(scr, "Events", 10, 10, 300, 250);
	hotplug_sb = sdl_scroll_buf_new(scr, "Hotplug", X_RES - 310, 10, 300, 400);

	queue = evf_io_queue_new();

	if (queue == NULL) {
		fprintf(stderr, "Can't allocate io queue!\n");
		return 1;
	}

	if ((fd = evf_hotplug_init(device_plugged, device_unplugged)) == -1) {
		fprintf(stderr, "Hotplug init failed: %s\n", strerror(errno));
		return 1;
	}

	SDL_UpdateRect(scr, 0, 0, X_RES, Y_RES);

	evf_io_queue_add(queue, fd, read_hotplug, NULL);

	for (;;)
		evf_io_queue_wait(queue, NULL);

	return 0;
}
