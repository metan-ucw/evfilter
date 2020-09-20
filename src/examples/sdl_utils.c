// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef __SDL_UTILS_H__
#define __SDL_UTILS_H__

#include "sdl_utils.h"
#include <SDL/SDL_gfxPrimitives.h>

#define CHAR_W 8
#define CHAR_H 8

struct sdl_scroll_buf {
	SDL_Surface *dest;
	SDL_Surface *buf;

	SDL_Rect buf_rect;

	Uint16 sy;
	Uint16 cy;
};

struct sdl_scroll_buf *sdl_scroll_buf_new(SDL_Surface *dest, const char *title, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
	struct sdl_scroll_buf *sb = malloc(sizeof(struct sdl_scroll_buf));
	uint32_t center;
	int len = strlen(title);

	if (sb == NULL)
		return NULL;

	sb->buf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0, 0, 0, 0);

	if (sb->buf == NULL) {
		free(sb);
		return NULL;
	}

	sb->dest = dest;

	sb->buf_rect.x = x;
	sb->buf_rect.y = y;
	sb->buf_rect.w = w;
	sb->buf_rect.h = h;
	sb->sy = CHAR_H + 8;
	sb->cy = sb->sy;

	if ((CHAR_W+1)*len > w)
		center = 0;
	else
		center = (w - (CHAR_W+1)*len)/2;

	SDL_FillRect(sb->buf, NULL, 0xff333333);
	stringColor(sb->buf, center, 3, title, 0x227722ff);
	hlineColor(sb->buf,   0, w,        0, 0x888888ff);
	hlineColor(sb->buf,   0, w,      h-1, 0x888888ff);
	hlineColor(sb->buf,   0, w, CHAR_W+5, 0x666666ff);
	vlineColor(sb->buf,   0, 0,        h, 0x888888ff);
	vlineColor(sb->buf, w-1, 0,        h, 0x888888ff);

	SDL_BlitSurface(sb->buf, NULL, dest, &sb->buf_rect);

	return sb;
}

void sdl_scroll_buf_add(struct sdl_scroll_buf *sb, const char *mesg, Uint32 color)
{
	if (sb->cy + CHAR_H + 1 < sb->buf_rect.h) {
		stringColor(sb->buf, 2, sb->cy, mesg, color);
		sb->cy += CHAR_H + 1;

		SDL_BlitSurface(sb->buf, NULL, sb->dest, &sb->buf_rect);

		return;
	}

	sb->cy = CHAR_H + 8;
	boxColor(sb->buf, 1,  CHAR_H + 8, sb->buf_rect.w - 2, sb->buf_rect.h - 2, 0x333333ff);
}

#endif /* __SDL_UTILS_H__ */
