// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Constants.

 */

#ifndef EVFD_H__
#define EVFD_H__

static char *evfd_help =
	"\n.................. -=[ EVFD ]=- .................\n"
	"                                                 \n"
	"  -- daemon for modifiying input events          \n"
	"                                                 \n"
	" parameters                                      \n"
	" ----------                                      \n"
	" -v  verbose output                              \n"
	" -q  quiet   output                              \n"
	" -d  do not dameonize                            \n"
	" -h  print this help                             \n"
	"                                                 \n"
	" description                                     \n"
	" -----------                                     \n"
	" Evfd looks for evfilter system configuration in \n"
	" /etc/evfilter/ and creates virtual input devices\n"
	" and routes filtered events from grabbed devices \n"
	" into them.                                      \n"
	"                                                 \n"
	" written by && bugs to                           \n"
	" ---------------------                           \n"
	" Cyril Hrubis metan@ucw.cz                       \n\n";

#endif /* EVFD_H__ */
