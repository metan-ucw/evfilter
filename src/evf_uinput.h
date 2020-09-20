// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Kernel user input library.

 */

#ifndef EVF_UINPUT_H__
#define EVF_UINPUT_H__

#include <linux/uinput.h>

/*
 * Returns file descriptor or -1 in case of failure.
 */
int evf_uinput_create(struct uinput_user_dev *ui_dev);

/*
 * Destroy user input.
 */
void evf_uinput_destroy(int fd);

#endif /* EVF_UINPUT_H__ */
