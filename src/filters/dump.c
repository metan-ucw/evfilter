// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Evfilter dump:
 *
 * Writes nice decomposition of event to the FILE.
 *
 * parameters:
 *
 * prefix = string
 *  prefix that is printed in every print
 *
 * file = path
 *  path to file to print to, there are two special files stdout and stderr
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <linux/input.h>

#include "evf_input.h"
#include "filter.h"
#include "filters.h"

struct dump {
	FILE *f;
	char prefix[];
};

static void dump_process(struct evf_filter *self, struct input_event *ev)
{
	struct dump *dump = (struct dump*) self->data;

	evf_input_print(dump->f, dump->prefix, ev);
	fprintf(dump->f, "\n");

	evf_filter_process(self->next, ev);
}

static struct evf_filter *dump_from_json(json_object *json_data)
{
	FILE *f = NULL;
	const char *fname = NULL;
	const char *prefix = "";

	json_object_object_foreach(json_data, key, val) {
		if (!strcmp(key, "file")) {
			fname = json_object_get_string(val);

			if (!strcmp(fname, "stdout"))
				f = stdout;
			else if (!strcmp(fname, "stderr"))
				f = stderr;
			else
				f = fopen(fname, "w");

		} else if (!strcmp(key, "prefix")) {
			prefix = json_object_get_string(val);
		} else {
			evf_msg(EVF_DEBUG, "Invalid JSON dump filter key %s", key);
			return NULL;
		}

	}

	if (!fname)
		f = stdout;

	if (!f) {
		evf_msg(EVF_ERR, "Failed to open '%s': %s", fname, strerror(errno));
		return NULL;
	}

	return evf_dump_alloc(prefix, f);
}

struct evf_filter_ops evf_dump_ops = {
	.json_id = "dump",
	.from_json = dump_from_json,
	.process = dump_process,
	.desc = "Dumps human readable decomposition of the events into a file",
};

struct evf_filter *evf_dump_alloc(const char *prefix, FILE *f)
{
	struct evf_filter *filter;
	struct dump *tmp;

	filter = evf_filter_alloc("dump", sizeof(struct evf_filter) + sizeof(struct dump) + strlen(prefix) + 1);

	if (!filter)
		return NULL;

	filter->ops = &evf_dump_ops;

	tmp = (struct dump*)filter->data;

	strcpy(tmp->prefix, prefix);

	tmp->f = f;

	return filter;
}
