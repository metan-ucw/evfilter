// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <strings.h>
#include <stdlib.h>
#include "key_parser.h"
#include "keys.h"

int keyparser_getkey(const char *name)
{
	int left  = 0;
	int right = key_count;
	int oleft, oright;
	int cmp;

	do {
		if (!(cmp = strcasecmp(name, key_names[(left+right)/2]))) {
			return key_values[(left+right)/2];
		}

		oleft  = left;
		oright = right;

		if (cmp > 0) {
			left = (left+right)/2;
		} else {
			right = (left+right)/2;
		}

	} while (left != oleft || right != oright);

	return -1;
}

const char *keyparser_getname(const int i)
{
	int j;

	for (j = 0; j < key_count; j++)
		if (key_values[j] == i)
			return key_names[j];

	return NULL;
}
