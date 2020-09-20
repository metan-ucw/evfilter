// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "evf_filter.h"
#include "evf_loader.h"
#include "evf_err.h"
#include "evf_profile.h"
#include "evf_input.h"
#include "filters/evf_msg.h"

/*
 * suppose input device names are short enough
 */
#define STR_MAX 2048

/*
 * Check for matching rules.
 *
 *  0  = rule wasn't matched
 *  1  = rule matched
 * -1  = invalid type
 * -2  = ioctl() has failed
 * -3  = stat() has failed
 */
static int check_for_matching_rule(int fd, const char *type, const char *value)
{
	char str_buf[STR_MAX];

	/* inputX, do not use */
	if (!strcasecmp("Device", type)) {
		int res = evf_input_compare(fd, value);

		if (res == -1)
			return -3;

		return res;
	}

	/* Name eg. device name */
	if (!strcasecmp("Name", type)) {
		if (evf_input_get_name(fd, str_buf, STR_MAX) < 0)
			return -2;

		return !strcmp(str_buf, value);
	}

	/* Physical device, eg. port */
	if (!strcasecmp("Phys", type)) {

		if (evf_input_get_phys(fd, str_buf, STR_MAX) < 0)
			return -2;

		return !strcmp(str_buf, value);
	}

	/* Device abilities */
	if (!strcasecmp("Bits", type)) {
		//TODO
		return 0;
	}

	/* Invalid type */
	return -1;
}

/*
 * Compose path to profile file, try to open it.
 */
static FILE *open_profilerc(const char *path, union evf_err *err)
{
	char *str_buf;
	FILE *f;

	str_buf = malloc(strlen(path) + strlen(EVFILTER_PROFILE_FILE) + 1);

	if (!str_buf) {
		err->type = evf_errno;
		err->err_no.err_no = errno;
		return NULL;
	}

	strcpy(str_buf, path);
	strcpy(str_buf + strlen(path), EVFILTER_PROFILE_FILE);

	evf_msg(EVF_DEBUG, "Opening profile %s.", str_buf );

	f = fopen(str_buf, "r");

	if (!f) {
		//TODO: context? coudn't open file is not enough
		err->type = evf_errno;
		err->err_no.err_no = errno;
	}

	free(str_buf);

	return f;
}

/*
 * Just instance of evf_load_profile
 */
struct evf_filter *evf_load_system_profile(int fd, union evf_err *err)
{
	return evf_load_profile(EVFILTER_PROFILE_DIR, fd, err);
}

/*
 * Try to open prifile file.
 */
struct evf_filter *evf_load_profile(const char *path, int fd,
                                    union evf_err *err)
{
	FILE *profile;
	char type[256];
	char value[2048];
	int has_matched = 0, val;
	struct evf_filter *filters = NULL, *tmp;

	profile = open_profilerc(path, err);

	if (!profile)
		return NULL;

	for (;;) {
		fscanf(profile, "%256s", type);
		fscanf(profile, " %2048[^\n]", value);

		if (!feof(profile)) {
			//printf("name=`%s' value=`%s'\n", type, value);

			/* load file, if there was an match */
			if (!strcasecmp(type, "File")) {

				if (has_matched) {
					evf_msg(EVF_DEBUG, "Loading config file '%s' in directory '%s'.", value, path);
					tmp = evf_load_filters_compose(path,
					                               value,
								       err);
					/*
					 * err is filled by
					 * evf_load_filters_compose
					 */
					if (err->type != evf_ok) {
						evf_filters_free(filters);
						fclose(profile);
						evf_msg(EVF_ERR,"Failed to load '%s%s'", path, value);
						return NULL;
					}

					/*  append all filters into one list */
					filters = evf_filters_append(filters,
					                             tmp);

					has_matched = -1;
				}
			} else {
				/*
				 * After some rule was matched, system loads
				 * several files (at least one) and we should
				 * clean has_matched flag right after something
				 * else than "File ...." rule came.
				 */
				if (has_matched == -1)
					has_matched = 0;

				val = check_for_matching_rule(fd, type, value);
				//TODO: here we are ingoring all errors
				if (val == 1)
					has_matched  = 1;
			}
		} else
			break;
	}

	fclose(profile);
	err->type = evf_ok;
	return filters;
}
