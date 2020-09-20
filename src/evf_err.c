// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <stdio.h>

#include "evf_err.h"

//#define DPRINT(...) { fprintf(stderr, "%s: %i: ", __FILE__, __LINE__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#define DPRINT(...)

/*
 * Print error message according to union evf_err
 * TODO: convert error message to string
 */
void evf_err_print(union evf_err *err)
{
	if (err->type == evf_ok) {
		printf("Operation was succesfull.\n");
		return;
	}

	if (err->type == evf_errno) {
		printf("Errno: %s\n", strerror(err->err_no.err_no));
		return;
	}

	switch (err->param.etype) {
		case evf_emissing:
			printf("Parameter `%s' is missing.\n", err->param.name);
		break;
		case evf_epname:
			printf("Invalid parameter `%s'.\n", err->param.name);
		break;
		case evf_efname:
			printf("Invalid filter name `%s'.\n", err->param.name);
		break;
		case evf_einval:
		//	printf("Invalid value `%s' for parameter `%s'. "
		//	       "Expected %s.\n",
		//	       err->param.value, err->param.name,
		//	       evf_get_type_name(err->param.ptype));
		break;
		case evf_erange:
			//TODO: print range
			printf("Parameter `%s' out of range.\n",
			       err->param.name);
		break;
		case evf_eredef:
			printf("Parameter `%s' redefined.\n", err->param.name);
		break;
		case evf_nofname:
			printf("Missing expected FilterName directive.\n");
		break;
		case evf_noparams:
			printf("Missing expected filter parameters section ended by EndFilter.\n");
		break;
	}
}
