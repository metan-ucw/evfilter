// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Lock we want only one instance of evfd running.

 */

#ifndef EVFD_LOCK_H__
#define EVFD_LOCK_H__

#include <stdbool.h>

/*
 * Try to create lock.
 */
bool evfd_try_lock(void);

/*
 * Release lock.
 */
void evfd_release_lock(void);

#endif /* EVFD_LOCK_H__ */
