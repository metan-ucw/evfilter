// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Here is defined basics filter loading function that can load filter
  accordingly to name and parameters.

  Also some basics functionality to destroy filters as well as api to
  get some filter parameters.

 */

#ifndef EVF_FILTER_H__
#define EVF_FILTER_H__

struct input_event;
struct evf_filter;
union evf_err;

/*
 * Returns filter name.
 */
const char *evf_filter_get_name(struct evf_filter *filter);

/*
 * Returns filter descriptions.
 */
const char *evf_filter_get_desc(struct evf_filter *filter);

/*
 * Free filter.
 */
void evf_filter_free(struct evf_filter *filter);

/*
 * Free linked list of filters.
 */
void evf_filters_free(struct evf_filter *root);

/*
 * Returns last filter in linked list.
 */
struct evf_filter *evf_filters_last(struct evf_filter *root);

/*
 * Merges two linked lists of filters together. Second list is appended to the
 * end of first one. Returned value is begining of the new list.
 */
struct evf_filter *evf_filters_append(struct evf_filter *root,
                                      struct evf_filter *filters);

/*
 * Debug function, prints linked list of filters to stdout.
 */
void evf_filters_print(struct evf_filter *root);

#endif /* EVF_FILTER_H__ */
