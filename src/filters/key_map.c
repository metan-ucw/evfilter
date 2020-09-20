// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>
#include <stdint.h>

#include "evf_struct.h"
#include "evf_msg.h"
#include "key_parser.h"
#include "filters.h"

struct bit_map {
	unsigned int map_size;
	uint8_t map[];
};

#define MAP_SIZE(key) ((key)/8 + !!((key)%8))

static inline int bit_map_get(struct bit_map *map, int key)
{
	unsigned int map_byte = key/8;

	if (map_byte >= map->map_size)
		return 0;

	return map->map[map_byte] & (key%8);
}

static inline void bit_map_set(struct bit_map *map, int key)
{
	unsigned int map_byte = key/8;

	if (map_byte >= map->map_size)
		return;

	map->map[map_byte] |= (key%8);
}

static struct bit_map *map_alloc(int keys[], unsigned int key_cnt)
{
	unsigned int i;
	struct bit_map *map;
	size_t size = 0;

	for (i = 0; i < key_cnt; i++)
		size = MAX(MAP_SIZE((size_t)keys[i]), size);

	map = malloc(sizeof(struct bit_map) + size);

	map->map_size = size;
	memset(map->map, 0, size);

	for (i = 0; i < key_cnt; i++)
		bit_map_set(map, keys[i]);

	return map;
}

static void bit_map_free(struct bit_map *map)
{
	free(map);
}

struct key {
	int key_from;
	int key_to;
};

struct priv {
	struct bit_map *map;
	unsigned int key_cnt;
	struct key keys[];
};

static int map_key(struct priv *self, int key)
{
	unsigned int i;

	for (i = 0; i < self->key_cnt; i++) {
		if (self->keys[i].key_from == key)
			return self->keys[i].key_to;
	}

	return 0;
}

static void key_map_process(struct evf_filter *self, struct input_event *ev)
{
	struct priv *priv = (struct priv*)self->data;

	if (ev->type == EV_KEY && bit_map_get(priv->map, ev->code))
		ev->code = map_key(priv, ev->code);

	evf_filter_process(self->next, ev);
}

static struct evf_filter *key_map_from_json(json_object *json_data)
{
	struct evf_filter *ret;
	int key_cnt = json_object_object_length(json_data);
	int *key_from = malloc(2 * key_cnt * sizeof(int));
	int *key_to;
	int cnt = 0;

	if (!key_from) {
		evf_msg(EVF_DEBUG, "Malloc failed");
		return NULL;
	}

	key_to = &key_from[key_cnt];

	json_object_object_foreach(json_data, key, val) {
		const char *val_str = json_object_get_string(val);

		key_from[cnt] = keyparser_getkey(key);
		key_to[cnt] = keyparser_getkey(val_str);

		if (key_from[cnt] == -1) {
			evf_msg(EVF_DEBUG, "Invalid key name %s", key);
			free(key_from);
			return NULL;
		}

		if (key_to[cnt] == -1) {
			evf_msg(EVF_DEBUG, "Invalid key name %s", val_str);
			free(key_from);
			return NULL;
		}

		cnt++;
	}

	ret = evf_key_map_alloc(key_from, key_to, key_cnt);

	free(key_from);

	return ret;
}

static void key_map_free(struct evf_filter *self)
{
	struct priv *priv = (struct priv*)self->data;

	bit_map_free(priv->map);
}

struct evf_filter_ops evf_key_map_ops = {
	.json_id = "key_map",
	.from_json = key_map_from_json,
	.process = key_map_process,
	.free = key_map_free,
	.desc = "Remaps keys",
};

struct evf_filter *evf_key_map_alloc(int key_from[], int key_to[], unsigned int key_cnt)
{
	struct evf_filter *filter = malloc(sizeof(struct evf_filter) +
	                                   sizeof(struct priv) +
	                                   key_cnt * sizeof(struct key));
	struct priv *priv;
	unsigned int i;

	if (!filter)
		return NULL;

	filter->ops = &evf_key_map_ops;

	priv = (struct priv*)filter->data;

	priv->map = map_alloc(key_from, key_cnt);

	if (!priv->map) {
		free(filter);
		return NULL;
	}

	priv->key_cnt = key_cnt;

	for (i = 0; i < key_cnt; i++) {
		priv->keys[i].key_from = key_from[i];
		priv->keys[i].key_to = key_to[i];
	}

	return filter;
}
