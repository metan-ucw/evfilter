// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Evfilter loader gives you simple api to load all filters from one
  configuration file.

 */

#ifndef EVF_LOADER_H__
#define EVF_LOADER_H__

struct evf_filter;
union evf_err;

/*
 * Create filter line from configuration file.
 *
 * On succesfull operation pointer to filter line is returned. NULL is valid
 * value for empty file, so don't forget to check err in this case unless you
 * say empty file is invalid configuration.
 */
struct evf_filter *evf_load_filters(const char *path, union evf_err *err);

/*
 * Dtto, but compose path from path and filename.
 */
struct evf_filter *evf_load_filters_compose(const char *path, const char *file,
                                            union evf_err *err);


#endif /* EVF_LOADER_H__ */
