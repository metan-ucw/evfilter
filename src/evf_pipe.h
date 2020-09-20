// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Pipe can connect filters so that then can "cross-talk" between different
   filter lines. Pipe is identified by it's name, name is not case sensitive.

   There are special filters that can send (subset) of event flows to a pipe,
   these filters feeds input events to a pipe by calling evf_pipe_process().

   There is a from_pipe filter that can mix events from pipe into a filter
   line however any filter can be registered as a pipe sink as long as:

   * on init calls
     evf_pipe_get()
     evf_pipe_add_sink()

   * on exit calls
     evf_pipe_rem_sink()
     evf_pipe_put()

  */

#ifndef EVF_PIPE_H__
#define EVF_PIPE_H__

#define EVF_PIPE_MAX 64
#define EVF_PIPE_OUTPUTS_MAX 6

struct evf_filter;
struct input_event;

struct evf_pipe {
	char name[EVF_PIPE_MAX];
	int refcount;
	struct evf_filter *sinks[EVF_PIPE_OUTPUTS_MAX];
	struct evf_pipe *next;
};

/*
 * Retuns pipe by name and increases refcount. Returns newly allocated pipe if
 * there was no pipe by the name.
 *
 * May return NULL on allocation failure.
 */
struct evf_pipe *evf_pipe_get(const char *name);

/*
 * Decreases ref count, frees pipe when refcount runs out.
 */
void evf_pipe_put(struct evf_pipe *self);

/*
 * Adds a filter as a sink, returns non-zero if all pipe sinks are used.
 */
int evf_pipe_add_sink(struct evf_pipe *self, struct evf_filter *sink);

/*
 * Removes filter from sinks, returns non-zero if filter wasn't registered as a sink.
 */
int evf_pipe_rem_sink(struct evf_pipe *self, struct evf_filter *sink);

/*
 * Sends events to a pipe sinks.
 */
void evf_pipe_process(struct evf_pipe *self, struct input_event *ev);

#endif /* EVF_PIPE_H__ */
