// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Error messages redirecting.

 */

#ifndef EVF_MSG_H__
#define EVF_MSG_H__

#include <stdbool.h>

enum evf_msg_t {
	EVF_ERR,
	EVF_WARN,
	EVF_NOTICE,
	EVF_INFO,
	EVF_DEBUG,
	EVF_MAX = EVF_DEBUG,
};

enum evf_msg_out {
	EVF_STDERR,
	EVF_SYSLOG,
};

/*
 * Initalize messages.
 */
void evf_msg_init(char *process_name);

/*
 * Prints message.
 */
void evf_msg(enum evf_msg_t type, const char *fmt, ...);

/*
 * Turn on/off output.
 */
void evf_msg_output(enum evf_msg_out output, bool on_off);

/*
 * Set verbosity level.
 */
void evf_msg_verbosity_set(enum evf_msg_t level);

/*
 * Deinitalize messages.
 */
void evf_msg_exit(void);

#endif /* EVF_MSG_H__ */
