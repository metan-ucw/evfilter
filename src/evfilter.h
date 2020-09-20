// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef EVFILTER_H__
#define EVFILTER_H__

/*
 * Basic linux input interface.
 */
#include <linux/input.h>

/*  Error reporting, functions to translate error codes to error messages. */
#include "evf_err.h"

/*
 * Basic filter interface. Can load filters, destroy them, get parameters.
 *
 * NOTE: Most of the interface are low level calls, do not use unless you know
 *       what are you doing.
 */
#include "evf_filter.h"

/* Load filters accroding to config file and returns handle to it. */
#include "evf_loader.h"

/*
 * Implements profiles. Profile is directory with special file that contains rules
 * and many more files with informations witch filters to load.
 */
#include "evf_profile.h"

/*
 * Evfilter line. High level interface, you can call just create line and
 * library looks for system wide profiles, loads filter and many more. This is
 * application preffered interface.
 */
#include "evf_line.h"

/* Hotplug implementation. */
#include "evf_hotplug.h"

/* Linux input interface */
#include "evf_input.h"

/* Linux user input interface */
#include "evf_uinput.h"

#endif /* EVFILTER_H__ */
