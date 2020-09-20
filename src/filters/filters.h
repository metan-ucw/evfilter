// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef FILTERS_H__
#define FILTERS_H__

#include <stdio.h>

#define MAX(a, b) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	_a > _b ? _a : _b; \
})

#define MIN(a, b) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	_a < _b ? _a : _b; \
})

struct input_event;
union evf_err;

extern struct evf_filter_ops evf_key_lock_ops;
struct evf_filter *evf_key_lock_alloc(int key);

extern struct evf_filter_ops evf_kalman_ops;
struct evf_filter *evf_kalman_alloc(float gain_x, float gain_y);

extern struct evf_filter_ops evf_barrier_ops;
struct evf_filter *evf_barrier_alloc(unsigned int buffer_size);

extern struct evf_filter_ops evf_no_repeat_ops;
struct evf_filter *evf_no_repeat_alloc(void);

extern struct evf_filter_ops evf_abs2rel_ops;
struct evf_filter *evf_abs2rel_alloc(void);

extern struct evf_filter_ops evf_dump_ops;
struct evf_filter *evf_dump_alloc(const char *prefix, FILE *f);

extern struct evf_filter_ops evf_btn2rel_ops;
struct evf_filter *evf_btn2rel_alloc(int key_down, int key_up,
                                     int key_left, int key_right,
                                     int left_btn, int middle_btn, int right_btn);

extern struct evf_filter_ops evf_to_pipe_ops;
struct evf_filter *evf_to_pipe_alloc(const char *name);

extern struct evf_filter_ops evf_filter_to_pipe_ops;
struct evf_filter *evf_filter_to_pipe_alloc(const char *name, int type, int code);

extern struct evf_filter_ops evf_from_pipe_ops;
struct evf_filter *evf_from_pipe_alloc(const char *name);

extern struct evf_filter_ops evf_key_map_ops;
struct evf_filter *evf_key_map_alloc(int key_from[], int key_to[], unsigned int key_cnt);

extern struct evf_filter_ops evf_rel2scroll_ops;
struct evf_filter *evf_rel2scroll_alloc(int trigger_btn, int xmod, int ymod);

extern struct evf_filter_ops evf_rotate_ops;
struct evf_filter *evf_rotate_alloc(int rotate_abs, int rotate_rel);

extern struct evf_filter_ops evf_mirror_ops;
struct evf_filter *evf_mirror_alloc(int mirror_x, int mirror_y);

extern struct evf_filter_ops evf_pressure_to_key_ops;
struct evf_filter *evf_pressure_to_key_alloc(int treshold, int key);


struct evf_filter *evf_commit_alloc(void (*commit)(struct input_event *ev,
                                    void *data), void *data);

extern struct evf_filter_ops evf_half_qwerty_ops;
struct evf_filter *evf_half_qwerty_alloc(void);

#endif /* FILTERS_H__ */
