// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "evf_input.h"
#include "keyparser/key_parser.h"

int evf_input_get_version(int fd, int *version)
{
	if (ioctl(fd, EVIOCGVERSION, version))
		return -1;

	return 0;
}

/*
 * Okay, input layer in kernel uses copy_to_user kernel
 * macro to send us this string and this macro returns
 * number of bytes that was copied. So values <= 0 are
 * considered error conditions.
 */
int evf_input_get_name(int fd, char *buf, size_t buf_len)
{
	int ret;

	if ((ret = ioctl(fd, EVIOCGNAME(buf_len), buf)) <= 0)
		return -1;

	return ret;
}

/*
 * Same as abowe.
 */
int evf_input_get_phys(int fd, char *buf, size_t buf_len)
{
	int ret;

	if ((ret = ioctl(fd, EVIOCGPHYS(buf_len), buf)) <= 0)
		return -1;

	return ret;
}

/*
 * We compare major and minor number here.
 */
int evf_input_compare(int fd, const char *path)
{
	struct stat st1, st2;

	if (fstat(fd, &st1) == -1)
		return -1;

	if (stat(path, &st2) == -1)
		return -1;

//	printf("st_dev %ix%i %ix%i\n", minor(st1.st_rdev), major(st1.st_rdev), minor(st2.st_rdev), major(st2.st_rdev));

	return st1.st_rdev == st2.st_rdev;
}

static char *ev_key[3] = {"KEY_UP", "KEY_DOWN", "KEY_REPEAT" };
static const int ev_key_cnt = 3;

/*
 * Pretty print for struct input_event.
 */
static void print_key(FILE *file, const char *prefix, struct input_event *ev)
{
	fprintf(file, "%sev->code:  %s (%i)\n", prefix, keyparser_getname(ev->code), ev->code);

	switch (ev->value) {
		case 0:
			fprintf(file, "%sev->value: KEY_UP     (0)\n", prefix);
		break;
		case 1:
			fprintf(file, "%sev->value: KEY_DOWN   (1)\n", prefix);
		break;
		case 2:
			fprintf(file, "%sev->value: KEY_REPEAT (2)\n", prefix);
		break;
		default:
			fprintf(file, "%sev->value: UNKNOWN (%i)\n", prefix, ev->value);
	}
}

#define EV_REL_CNT 10

static char *ev_rel[EV_REL_CNT] = {
"REL_X",      "REL_Y",      "REL_Z",    "REL_RX",   "REL_RY",
"REL_RZ", "REL_HWHEEL",   "REL_DIAL", "REL_WHEEL", "REL_MISC",
};


static void print_rel(FILE *file, const char *prefix, struct input_event *ev)
{
	if (ev->code < EV_REL_CNT)
		fprintf(file, "%sev->code:  %s (%i)\n", prefix, ev_rel[ev->code], ev->code);
	else
		fprintf(file, "%sev->code:  UNKNOWN (%i)\n", prefix, ev->code);

	fprintf(file, "%sev->value: %i\n", prefix, ev->value);
}

#define EV_ABS_CNT 42

static char *ev_abs[EV_ABS_CNT] = {
"ABS_X", "ABS_Y", "ABS_Z", "ABS_RX", "ABS_RY", "ABS_RZ", "ABS_THROTTLE",
"ABS_RUDDER", "ABS_WHEEL", "ABS_GAS", "ABS_BRAKE", "UNKNOWN", "UNKNOWN",
"UNKNOWN", "UNKNOWN", "UNKNOWN", "ABS_HAT0X", "ABS_HAT0Y", "ABS_HAT1X",
"ABS_HAT1Y", "ABS_HAT2X", "ABS_HAT2Y", "ABS_HAT3X", "ABS_HAT3Y", "ABS_PRESSURE",
"ABS_DISTANCE", "ABS_TILT_X", "ABS_TILT_Y", "ABS_TOOL_WIDTH", "UNKNOWN",
"UNKNOWN", "UNKNOWN", "ABS_VOLUME", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN",
"UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "ABS_MISC"};

static void print_abs(FILE* file, const char *prefix, struct input_event *ev)
{
	(void) ev;
	(void) prefix;

	if (ev->code < EV_ABS_CNT)
		fprintf(file, "%sev->code:  %s (%i)\n", prefix, ev_abs[ev->code], ev->code);
	else
		fprintf(file, "%sev->code:  UNKNOWN (%i)\n", prefix, ev->code);
}


static char *ev_type[] = { "EV_SYN", "EV_KEY", "EV_REL", "EV_ABS", "EV_MSC", "EV_SW",
                           "EV_LED", "EV_SND", "EV_REP",  "EV_FF", "EV_PWR", "EV_FF_STAT", };

void evf_input_print(FILE *file, const char *prefix, struct input_event *ev)
{
	fprintf(file, "%sev->type:  ", prefix);

	if (ev->type < 13)
		fprintf(file, "%s\n", ev_type[ev->type]);
	else
		fprintf(file, "UNKNOWN %i\n", ev->type);

	switch (ev->type) {
		case EV_KEY:
			print_key(file, prefix, ev);
		break;
		case EV_REL:
			print_rel(file, prefix, ev);
		break;
		case EV_ABS:
			print_abs(file, prefix, ev);
		break;
	}
}

static const char *ev_unknown = "UNKNOWN";

const char *evf_input_type(struct input_event *ev)
{
	if (ev->type < 13)
		return ev_type[ev->type];

	return ev_unknown;
}

const char *evf_input_code(struct input_event *ev)
{
	switch (ev->type) {
		case EV_SYN:
			return "0";
		break;
		case EV_KEY:
			return keyparser_getname(ev->code);
		break;
		case EV_REL:
			if (ev->code < EV_REL_CNT)
				return ev_rel[ev->code];
		break;
		case EV_ABS:
			if (ev->code < EV_ABS_CNT)
				return ev_abs[ev->code];
	}

	return ev_unknown;
}

const char *evf_input_value(struct input_event *ev)
{
	switch (ev->type) {
		case EV_KEY:
			if (ev->value < ev_key_cnt)
				return ev_key[ev->value];
			else
				return ev_unknown;
		break;
		case EV_SYN:
			return "0";
		break;
	}

	return NULL;
}

int evf_input_grab(int fd)
{
	return ioctl(fd, EVIOCGRAB, 1);
}


int evf_input_ungrab(int fd)
{
	return ioctl(fd, EVIOCGRAB, 0);
}
