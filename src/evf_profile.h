// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Profile in evfilter library is way to do either system wide configuration,
  eg. touchscreen calibration or specify some configuration for your own tool
  eg. slow down all relative pointers for console mouse daemon.

  Basically profile is directory with profile file that contains matching rules
  and some profile files where any such file contains list of modules to load.

  Profile file consist of several lines where each one defines one of following
  matching rules:

  Device /path/to/device               //we compare minor and major number here
  Name Input Hardware Name
  Phys /device/phys/from/kernel
  Bits Hardware capabilities
  File evfilter configuration to load

  See /proc/bus/input/devices for identifying your hardware.

  System wide configuration should live in /etc/evfilter/profile/profilerc or
  something close.

*/

#ifndef EVF_PROFILE_H__
#define EVF_PROFILE_H__

struct evf_filter;
union evf_err;

/*
 * Load system wide configuration.
 *
 * Takes file descriptor to input device, returns list of filters.
 *
 * You should allways check evf_err_t in err, because NULL here is
 * correct return value.
 */
struct evf_filter *evf_load_system_profile(int fd, union evf_err *err);

/*
 * Same as above, but for user defined profile directory.
 */
struct evf_filter *evf_load_profile(const char *path, int fd,
                                    union evf_err *err);


#endif /* EVF_PROFILE_H__ */
