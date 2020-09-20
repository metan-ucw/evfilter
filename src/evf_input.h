// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef EVF_INPUT_H__
#define EVF_INPUT_H__

#include <stdlib.h>
#include <stdio.h>

struct input_event;

/*
 * Returns input version, to print it use:
 *
 * printf("%d %d %d", version>>16, version>>8 & 0xff, version & 0xff);
 */
int evf_input_get_version(int fd, int *version);

/*
 * Returns up to buf_len characters of input device name.
 */
int evf_input_get_name(int fd, char *buf, size_t buf_len);

/*
 * Returns up to buf_len characters of input device phys.
 */
int evf_input_get_phys(int fd, char *buf, size_t buf_len);

/*
 * Compares minor an major number of fd and path.
 *
 * Returns:
 * -1 on error
 *  1 if numbers are the same
 *  0 if minor or major number are different
 */
int evf_input_compare(int fd, const char *path);

/*
 * Prints human readable decompostion of input_event
 * into file.
 */
void evf_input_print(FILE *file, const char *prefix, struct input_event *ev);

/*
 * Event type to string.
 */
const char *evf_input_type(struct input_event *ev);

/*
 * Event code to string.
 */
const char *evf_input_code(struct input_event *ev);

/*
 * Event value to string.
 */
const char *evf_input_value(struct input_event *ev);

/*
 * Grab input device. Returns 0 on success -ERROR on failure.
 *
 * !!! WARNING !!!
 *
 * Grabbed input device sends events only to you (caller of grab ioctl),
 * so if you grab your keyboard it will suddenly stop sending events to xserver.
 * Anyway you have been warned.
 */
int evf_input_grab(int fd);

/*
 * Ungrab input device. Returns 0 on success -ERROR on failure.
 *
 */
int evf_input_ungrab(int fd);


#endif /* EVF_INPUT_H__ */
