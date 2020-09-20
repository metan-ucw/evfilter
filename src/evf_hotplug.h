// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Here is implemented hotplug support for evfilter library.

  Currently the implementation uses inofity framework.

 */

#ifndef EVF_HOTPLUG_H__
#define EVF_HOTPLUG_H__

struct evf_io_queue;

/*
 * Register callback function and reset list of known devices.
 */
int evf_hotplug_init(void (*device_plugged)(const char *dev),
                     void (*device_unplugged)(const char *dev));

/*
 * Initalize hotplug and add it into io_queue.
 */
int evf_hotplug_io_queue_init(struct evf_io_queue *queue);

/*
 * Parse /proc/bus/input/devices and call callbacks. Returns
 * number of hotplug events. Can return -1 if parsing has failed.
 */
int evf_hotplug_rescan(void);

/*
 * Exit the hotplug.
 */
void evf_hotplug_exit(void);

#endif /* EVF_HOTPLUG_H__ */
