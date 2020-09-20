// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <evfilter.h>
#include <evf_struct.h>
#include <filters/evf_msg.h>
#include <json-c/json.h>

static void feed_sync(struct evf_filter *filters)
{
	struct input_event ev = {};

	evf_filter_process(filters, &ev);
}

static void feed_key(struct evf_filter *filters, json_object *json)
{
	int code = 1, value;
	const char *str_code = NULL;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "value")) {
			value = json_object_get_int(val);
		} else if (!strcmp(key, "code")) {
			str_code = json_object_get_string(val);
		} else {
			fprintf(stderr, "Invalid Key event JSON key '%s'", key);
			exit(1);
		}
	}

	if (!str_code) {
		fprintf(stderr, "Missing Key value\n");
		exit(1);
	}

	code = keyparser_getkey(str_code);
	if (code < 0) {
		fprintf(stderr, "Invalid Key value '%s'\n", str_code);
		exit(1);
	}

	struct input_event ev = {
		.type = EV_KEY,
		.code = code,
		.value = value,
	};

	evf_filter_process(filters, &ev);
}

static void feed_event_json(json_object *json, struct evf_filter *filters)
{
	json_object *event_type;

	if (!json_object_object_get_ex(json, "type", &event_type)) {
		fprintf(stderr, "Missing event type!\n");
		exit(1);
	}

	const char *type = json_object_get_string(event_type);

	json_object_object_del(json, "type");

	if (!strcmp(type, "Sync")) {
		feed_sync(filters);
	} else if (!strcmp(type, "Key")) {
		feed_key(filters, json);
	} else {
		fprintf(stderr, "Invalid event type '%s'\n", type);
		exit(1);
	}

}

static void feed_events_json(json_object *json, struct evf_filter *filters)
{
	size_t i;

	if (!json_object_is_type(json, json_type_array)) {
		feed_event_json(json, filters);
		return;
	}

	for (i = 0; i < json_object_array_length(json); i++) {
		json_object *json_data = json_object_array_get_idx(json, i);
		feed_event_json(json_data, filters);
	}
}

static void feed_events(const char *fname, struct evf_filter *filters)
{
	int fd;
	char buf[2048];
	ssize_t size;
	json_object *json;
	json_tokener *tok;

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open '%s'\n", fname);
		exit(1);
	}

	tok = json_tokener_new();
	if (!tok) {
		fprintf(stderr, "Failed to create JSON tokener\n");
		exit(1);
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
		fprintf(stderr, "JSON tokener %s at %i\n",
			json_tokener_error_desc(jerr), tok->char_offset);
		exit(1);
	}

	feed_events_json(json, filters);

	json_object_put(json);
	json_tokener_free(tok);
	close(fd);
}

static FILE *logfile;

static void log_event(struct evf_filter *self, struct input_event *ev)
{
	(void)self;

	evf_input_print(logfile, "", ev);
	fprintf(logfile, "\n");
}

static struct evf_filter_ops logger_ops = {
	.process = log_event,
};

static struct evf_filter logger = {
	.ops = &logger_ops
};

int main(int argc, char *argv[])
{
	struct evf_filter *filters;
	union evf_err err;

	if (argc != 2 && argc != 3) {
		printf("usage: config_file.json [data_file.json.in]\n");
		return 1;
	}

	evf_msg_output(EVF_STDERR, 0);

	filters = evf_load_filters(argv[1], &err);

	if (!filters) {
		logfile = fopen("out", "w");
		fprintf(logfile, "Failed to load filters\n");
		fclose(logfile);
		return 0;
	}

	if (argc == 3) {
		struct evf_filter *last = evf_filters_last(filters);

		last->next = &logger;

		logfile = fopen("out", "w");

		feed_events(argv[2], filters);

		fclose(logfile);

		last->next = NULL;
	}

	evf_filters_free(filters);

	return 0;
}
