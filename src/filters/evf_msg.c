// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>

#include "evf_msg.h"

static char *evf_process_name;

static char *evf_msg_t_names[] = {
	"ERROR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG",
};

static int evf_msg_t_levels[] = {
	LOG_ERR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_INFO,
	LOG_DEBUG,
};

/* messages verbosity */
static enum evf_msg_t evf_verbosity = EVF_INFO;

/* messages sinks */
static bool evf_stderr = true;
static bool evf_syslog = false;

void evf_msg_init(char *process_name)
{
	evf_process_name = process_name;
}

static void evf_msg_stderr(enum evf_msg_t type, const char *fmt, va_list va)
{
	char *type_name = "UNKNOWN";

	if (type <= EVF_MAX)
		type_name = evf_msg_t_names[type];

	fprintf(stderr, "%s\t", type_name);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
}

static void evf_msg_syslog(enum evf_msg_t type, const char *fmt, va_list va)
{
	int level = LOG_ERR;

	openlog(evf_process_name, LOG_PID, LOG_DAEMON);

	if (type <= EVF_MAX)
		level = evf_msg_t_levels[type];
	else
		syslog(LOG_DEBUG, "invalid log type, probably bug in %s.",
		       evf_process_name);

	vsyslog(level, fmt, va);

	closelog();
}

void evf_msg(enum evf_msg_t type, const char *fmt, ...)
{
	va_list va;

	/* not enough verbosity to print message */
	if (type > evf_verbosity)
		return;

	if (evf_stderr) {
		va_start(va, fmt);
		evf_msg_stderr(type, fmt, va);
		va_end(va);
	}

	if (evf_syslog) {
		va_start(va, fmt);
		evf_msg_syslog(type, fmt, va);
		va_end(va);
	}
}

void evf_msg_verbosity_set(enum evf_msg_t level)
{
	evf_verbosity = level;
}

void evf_msg_output(enum evf_msg_out output, bool on_off)
{

	switch (output) {
		case EVF_STDERR:
			evf_stderr = on_off;
		break;
		case EVF_SYSLOG:
			evf_syslog = on_off;
		break;
	}

	return;
}

void evf_msg_exit(void)
{
	return;
}
