// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef EVFILTER_CONFIG_H__
#define EVFILTER_CONFIG_H__

/*
 * Default directory for system wide profiles.
 */
#define EVFILTER_PROFILE_DIR "/etc/evfilter/"

/*
 * Default filename for file containing profile rules.
 */
#define EVFILTER_PROFILE_FILE "profilerc"

/*
 * Path to the user input dev file.
 */
extern const char *EVFILTER_UINPUT_DEV_PATHS[];

#endif /* EVFILTER_CONFIG_H__ */
