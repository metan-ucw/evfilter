// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <json-c/json.h>

#include "evf_struct.h"
#include "evf_filter.h"
#include "evf_err.h"
#include "evf_loader.h"
#include "filters/evf_msg.h"
#include "filters/filters.h"

static const struct evf_filter_ops *filters_ops[] = {
	&evf_key_lock_ops,
	&evf_no_repeat_ops,
	&evf_btn2rel_ops,
	&evf_abs2rel_ops,
	&evf_barrier_ops,
	&evf_dump_ops,
	&evf_kalman_ops,
	&evf_key_map_ops,
	&evf_to_pipe_ops,
	&evf_filter_to_pipe_ops,
	&evf_from_pipe_ops,
	&evf_rel2scroll_ops,
	&evf_half_qwerty_ops,
	NULL,
};

static const struct evf_filter_ops *filter_ops_by_json_id(const char *json_id)
{
	const struct evf_filter_ops **i;

	for (i = filters_ops; *i; i++) {
		if (!strcmp((*i)->json_id, json_id))
			return *i;
	}

	return NULL;
}

static struct evf_filter *filter_from_json(json_object *json_data)
{
	const struct evf_filter_ops *ops;
	json_object *json_filter_id;

	if (!json_object_object_get_ex(json_data, "filter", &json_filter_id)) {
		evf_msg(EVF_ERR, "JSON object key 'filter' is missing");
		return NULL;
	}

	const char *filter_id = json_object_get_string(json_filter_id);

	if (!filter_id) {
		evf_msg(EVF_ERR, "JSON key filter has wrong type");
		return NULL;
	}

	ops = filter_ops_by_json_id(filter_id);

	if (!ops) {
		evf_msg(EVF_ERR, "Filter '%s' does not exist", filter_id);
		return NULL;
	}

	json_object_object_del(json_data, "filter");

	return ops->from_json(json_data);
}

static struct evf_filter *filters_from_json(json_object *json_data)
{
	size_t i;
	struct evf_filter *ret = NULL, *last = NULL;

	if (!json_object_is_type(json_data, json_type_array))
		return filter_from_json(json_data);

	for (i = 0; i < json_object_array_length(json_data); i++) {
		json_object *json_filter_data = json_object_array_get_idx(json_data, i);
		struct evf_filter *tmp = filter_from_json(json_filter_data);

		if (!tmp) {
			evf_filters_free(ret);
			return NULL;
		}

		tmp->next = NULL;

		if (!last) {
			last = tmp;
			ret = tmp;
		} else {
			last->next = tmp;
			last = tmp;
		}
	}

	return ret;
}

struct evf_filter *evf_load_filters(const char *path, union evf_err *err)
{
	int fd;
	char buf[2048];
	ssize_t size;
	json_object *json;
	json_tokener *tok;
	struct evf_filter *ret = NULL;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		//TODO: context to err
		err->type = evf_errno;
		err->err_no.err_no = ENOENT;
		return NULL;
	}

	tok = json_tokener_new();
	if (!tok) {
		evf_msg(EVF_ERR, "Failed to create JSON tokener");
		goto err1;
	}

	enum json_tokener_error jerr;

	while ((size = read(fd, buf, sizeof(buf))) > 0) {
		json = json_tokener_parse_ex(tok, buf, size);
		jerr = json_tokener_get_error(tok);

		switch (jerr) {
		case json_tokener_continue:
		case json_tokener_success:
		break;
		default:
			goto done;
		}
	}

done:
	jerr = json_tokener_get_error(tok);
	if (jerr) {
		evf_msg(EVF_ERR, "JSON tokener %s at %i",
			json_tokener_error_desc(jerr), tok->char_offset);
		goto err2;
	}

	ret = filters_from_json(json);
err2:
	if (json)
		json_object_put(json);
	json_tokener_free(tok);
err1:
	close(fd);
	return ret;
}

/*
 * Just wrapper for evf_load_filters.
 */
struct evf_filter *evf_load_filters_compose(const char *path, const char *file,
                                            union evf_err *err)
{
	char *str_buf = malloc(strlen(path) + strlen(file) + 1);
	struct evf_filter *tmp;

	if (str_buf == NULL) {
		err->type = evf_errno;
		err->err_no.err_no = errno;
		return NULL;
	}

	strcpy(str_buf, path);
	strcpy(str_buf + strlen(path), file);

	tmp = evf_load_filters(str_buf, err);
	free(str_buf);

	return tmp;
}
