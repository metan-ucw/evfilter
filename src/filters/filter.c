// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

struct evf_filter *evf_filter_alloc(const char *id, size_t size)
{
	struct evf_filter *filter = malloc(sizeof(struct evf_filter) + size);

	if (!filter) {
		evf_msg(EVF_WARN, "%s: malloc() failed :-(", id);
		return NULL;
	}

	memset(filter, 0, sizeof(struct evf_filter) + size);

	return filter;
}
