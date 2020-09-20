// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "evf_filter.h"
#include "filters/filters.h"
#include "evf_err.h"
#include "evf_struct.h"
#include "filters/evf_msg.h"

const char *evf_filter_get_name(struct evf_filter *filter)
{
	return filter->ops->json_id;
}

const char *evf_filter_get_desc(struct evf_filter *filter)
{
	return filter->ops->desc;
}

void evf_filter_free(struct evf_filter *filter)
{
	if (!filter)
		return;

	if (filter->ops->free)
		filter->ops->free(filter);

	free(filter);
}

void evf_filters_free(struct evf_filter *root)
{
	struct evf_filter *tmp = root, *del;

	while (tmp) {
		del = tmp;
		tmp = tmp->next;
		evf_filter_free(del);
	}
}

struct evf_filter *evf_filters_last(struct evf_filter *root)
{
	struct evf_filter *tmp;

	if (!root)
		return NULL;

	for (tmp = root; tmp->next; tmp = tmp->next);

	return tmp;
}

struct evf_filter *evf_filters_append(struct evf_filter *root,
                                      struct evf_filter *filters)
{
	struct evf_filter *last;

	if (!root)
		return filters;

	last = evf_filters_last(root);
	last->next = filters;

	return root;
}

void evf_filters_print(struct evf_filter *root)
{
	struct evf_filter *tmp;

	for (tmp = root; tmp; tmp = tmp->next)
		evf_msg(EVF_INFO," -> %s", evf_filter_get_name(tmp));
}
