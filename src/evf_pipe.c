// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
#include <string.h>

#include "evf_filter.h"
#include "evf_struct.h"
#include "evf_pipe.h"

static struct evf_pipe *pipes = NULL;

static struct evf_pipe *pipe_alloc(const char *name)
{
	struct evf_pipe *new = malloc(sizeof(struct evf_pipe));

	if (!new)
		return NULL;

	memset(new, 0, sizeof(struct evf_pipe));

	strncpy(new->name, name, EVF_PIPE_MAX);
	new->name[EVF_PIPE_MAX - 1] = '\0';

	new->next = pipes;
	pipes = new;

	return new;
}

void pipe_free(struct evf_pipe *self)
{
	struct evf_pipe *i;

	if (!pipes)
		return;

	for (i = pipes; i; i = i->next) {
		if (i->next == self)
			break;
	}

	if (i == NULL)
		return;

	if (i == pipes)
		pipes = self->next;
	else
		i->next = self->next;

	free(self);
}

static struct evf_pipe *pipe_lookup(const char *name)
{
	struct evf_pipe *i;

	for (i = pipes; i; i = i->next)
		if (!strcasecmp(i->name, name))
			return i;

	return NULL;
}

struct evf_pipe *evf_pipe_get(const char *name)
{
	struct evf_pipe *ret = pipe_lookup(name);

	if (!ret)
		ret = pipe_alloc(name);

	if (!ret)
		return NULL;

	ret->refcount++;

	return ret;
}

void evf_pipe_put(struct evf_pipe *self)
{
	self->refcount--;

	if (self->refcount <= 0)
		pipe_free(self);
}

int evf_pipe_add_sink(struct evf_pipe *self, struct evf_filter *sink)
{
	unsigned int i;

	for (i = 0; i < EVF_PIPE_OUTPUTS_MAX && self->sinks[i]; i++);

	if (i >= EVF_PIPE_OUTPUTS_MAX)
		return 1;

	self->sinks[i] = sink;
	return 0;
}

int evf_pipe_rem_sink(struct evf_pipe *self, struct evf_filter *sink)
{
	int i, found = -1;

	if (!self->sinks[0])
		return 1;

	for (i = 0; i < EVF_PIPE_OUTPUTS_MAX && self->sinks[i]; i++) {
		if (self->sinks[i] == sink)
			found = i;
	}

	if (found == -1)
		return 1;

	self->sinks[found] = self->sinks[i - 1];
	self->sinks[i - 1] = NULL;

	return 0;
}

void evf_pipe_process(struct evf_pipe *self, struct input_event *ev)
{
	unsigned int i;

	for (i = 0; i < EVF_PIPE_OUTPUTS_MAX && self->sinks[i]; i++)
		evf_filter_process(self->sinks[i], ev);
}
