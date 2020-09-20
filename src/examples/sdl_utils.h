/******************************************************************************
 * This file is part of evfilter library.                                     *
 *                                                                            *
 * Evfilter library is free software; you can redistribute it and/or modify   *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * Evfilter library is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with evfilter library; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 *                                                                            *
 * Copyright (C) 2008-2010 Cyril Hrubis <metan@ucw.cz>                        *
 *                                                                            *
 ******************************************************************************/

#ifndef __SDL_UTILS_H__
#define __SDL_UTILS_H__

#include <stdint.h>
#include <SDL/SDL.h>

struct sdl_scroll_buf;

struct sdl_scroll_buf *sdl_scroll_buf_new(SDL_Surface *dest, const char *title, Sint16 x, Sint16 y, Uint16 w, Uint16 h);
void   sdl_scroll_buf_add(struct sdl_scroll_buf *sb, const char *mesg, Uint32 color);


#endif /* __SDL_UTILS_H__ */
