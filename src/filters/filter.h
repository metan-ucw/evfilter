// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef EVF_FILTER_H__
#define EVF_FILTER_H__

#include "evf_struct.h"
#include "evf_msg.h"

struct evf_filter *evf_filter_alloc(const char *id, size_t size);

#endif /* EVF_FILTER_H__ */
