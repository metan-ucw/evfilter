// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef EVF_ERR_H__
#define EVF_ERR_H__

/*
 * Error types.
 */
enum evf_err_t {
	evf_ok,      /* all ok                                     */
	evf_errno,   /* errno from linux call                      */
};

/*
 * Parse error types.
 */
enum evf_err_par_t {
	evf_efname,   /* ivalid filter name; fills: err and name             */
	evf_epname,   /* ivalid parameter name; fills: err and name          */
	evf_emissing, /* parameter missing; fills: err and name              */
	evf_einval,   /* invalid value; fills: value, name, err and type     */
	evf_erange,   /* value out of range; fills: value, name, err and lim */
	evf_eredef,   /* value redefined; fills: value, name and err         */
	evf_nofname,  /* missing expected filter name                        */
	evf_noparams, /* missing expected parameters section                 */
};

/*
 * Errno from underlying linux call.
 *
 * Currently EMALLOC and errors from fopen()
 */
struct evf_err_errno {
	enum evf_err_t type;
	int err_no;
};

/*
 * Contains errors caused parse configuration errors.
 */
struct evf_err_param {
	enum evf_err_t type;
	enum evf_err_par_t etype;
	const char *name;
	const char *value;
	void *lim;
};

/*
 * This is the real err structure, use this one.
 */
union evf_err {
	enum evf_err_t type;
	struct evf_err_errno err_no;
	struct evf_err_param param;
};

/*
 * Prints error to stdout.
 */
void evf_err_print(union evf_err *err);

#endif /* EVF_ERR_H__ */
