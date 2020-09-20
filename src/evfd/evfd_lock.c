// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "evfd_lock.h"
#include "filters/evf_msg.h"

#define LOCKFILE "/var/run/evfd.pid"

/*
 * Save current pid.
 */
static bool pid_save(void)
{
	FILE *f;
	unsigned long long int i;

	f = fopen(LOCKFILE, "w");

	if (f == NULL) {
		evf_msg(EVF_ERR, "Can't open lockfile %s: %s",
		         LOCKFILE, strerror(errno));
		return false;
	}

	i = getpid();

	if (fprintf(f, "%llu\n", i) <= 0) {
		evf_msg(EVF_ERR, "fprintf into lockfile %s has failed",
		         LOCKFILE);
		fclose(f);
		return false;
	}

	if (fclose(f) != 0) {
		evf_msg(EVF_ERR, "fclose for lockfile has failed with %s",
		         strerror(errno));
		return false;
	}

	return true;
}

/*
 * Loads pid from file.
 */
static pid_t load_pid(void)
{
	FILE *f = fopen(LOCKFILE, "r");
	unsigned long long int i;

	if (f == NULL) {
		if (errno != ENOENT) {
			evf_msg(EVF_ERR, "Can't open pidfile %s: %s",
			         LOCKFILE, strerror(errno));
		}

		return 0;
	}

	if (fscanf(f, "%llu", &i) != 1) {
		evf_msg(EVF_ERR, "Invalid pidfile format, expected number.");
		fclose(f);
		return 0;
	}

	fclose(f);
	return i;
}

/*
 * Check if pid is running.
 */
static bool pid_is_running(pid_t pid)
{
	char path[128];
	struct stat buf;

	snprintf(path, 128, "/proc/%i", pid);

	/* no running process for pid */
	if (stat(path, &buf) == -1)
		return false;

	return true;
}

bool evfd_try_lock(void)
{
	pid_t pid = load_pid();

	evf_msg(EVF_DEBUG, "Trying to create lockfile %s", LOCKFILE);

	if (pid == 0)
		return pid_save();

	if (pid_is_running(pid)) {
		evf_msg(EVF_NOTICE, "Found lockfile with running pid. If no evfd is "
		                      "running, delete %s and try again", LOCKFILE);
		return false;
	}

	return pid_save();
}

void evfd_release_lock(void)
{
	evf_msg(EVF_DEBUG, "Releasing lockfile %s", LOCKFILE);

	if (unlink(LOCKFILE) == -1)	{
		if (errno == ENOENT)
			return;

		evf_msg(EVF_ERR, "Can't unlink lockfile %s: %s", LOCKFILE,
				strerror(errno));
	}
}
