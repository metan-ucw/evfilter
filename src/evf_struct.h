// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Here comes struct evfilter definition, do not inlucde this file into any
  code outside of this library. To acces it's mebers use functions defined
  in evf_filter.h.

 */

#ifndef EVF_STRUCT_H__
#define EVF_STRUCT_H__

#include <json-c/json.h>

struct input_event;
struct evf_filter;

struct evf_filter_ops {
	const char *json_id;
	struct evf_filter* (*from_json)(json_object *json_data);
	void (*process)(struct evf_filter *self, struct input_event *ev);
	void (*free)(struct evf_filter *self);
	const char *desc;
};

struct evf_filter {
	const struct evf_filter_ops *ops;

	/* Next filter in filter line. */
	struct evf_filter *next;
	/*
	 * Iternal filter data, usually structure holding filter state.
	 * In case filter is not stateless.
	 */
	char data[0];
};

static inline void evf_filter_process(struct evf_filter *self, struct input_event *ev)
{
	self->ops->process(self, ev);
}

#endif /* EVF_STRUCT_H__ */
