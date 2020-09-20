// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef KEY_PARSER_H__
#define KEY_PARSER_H__

/*
 * Returns key number as defined in linux/input.h.
 */
int keyparser_getkey(const char *name);

/*
 * Returns key name accordingly to key number.
 */
const char* keyparser_getname(int i);

#endif /* KEY_PARSER_H__ */
