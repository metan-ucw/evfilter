// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "evf_io_queue.h"

/*
#include <stdio.h>
#define DEBUG_PRINT(...) { printf("%s: %i: ", __FILE__, __LINE__); printf(__VA_ARGS__); }
*/

#define DEBUG_PRINT(...)

/*
 * Insert into sorted linked list
 */
static struct evf_io_queue_memb *list_insert(struct evf_io_queue_memb *root,
                                             struct evf_io_queue_memb *memb)
{
	struct evf_io_queue_memb *prev = NULL, *here = root;

	DEBUG_PRINT("Inserting into list\n");

	while (here != NULL && here->fd > memb->fd) {
		prev = here;
		here = here->next;
	}

	/* we are changing root */
	if (prev == NULL) {
		memb->next = root;
		return memb;
	}

	prev->next = memb;
	memb->next = here;

	return root;
}

/*
 * Remove from sorted linked list
 */
static struct evf_io_queue_memb *list_delete(struct evf_io_queue_memb *root,
                                             int fd)
{
	struct evf_io_queue_memb *prev = NULL, *here = root;

	while (here != NULL && here->fd != fd) {
		prev = here;
		here = here->next;
	}

	/* wasn't found */
	if (here == NULL)
		return root;

	if (prev == NULL)
		root = root->next;
	else
		prev->next = here->next;

	free(here);

	return root;
}

struct evf_io_queue *evf_io_queue_new(void)
{
	struct evf_io_queue *queue = malloc(sizeof(struct evf_io_queue));

	if (queue == NULL)
		return NULL;

	FD_ZERO(&queue->rfds);
	queue->root = NULL;
	queue->cnt  = 0;

	return queue;
}

void evf_io_queue_destroy(struct evf_io_queue *queue, int flags)
{
	struct evf_io_queue_memb *here, *prev = NULL;

	for (here = queue->root; here != NULL; prev = here, here = here->next) {
		if (flags & EVF_IO_QUEUE_CLOSE)
			close(here->fd);

		if (flags & EVF_IO_QUEUE_DFREE)
			free(here->priv);

		free(prev);
	}

	free(queue);
}

int evf_io_queue_wait(struct evf_io_queue *queue, struct timeval *timeout)
{
	int ret, ret_read;
	struct evf_io_queue_memb *here = queue->root;

	/* empty queue */
	if (queue->root == NULL) {
		usleep(500);
		return 0;
	}

	if ((ret = select(queue->root->fd + 1, &queue->rfds, NULL, NULL,
	                  timeout)) < 0)
		return ret;

	for (here = queue->root; here != NULL; here = here->next) {
		DEBUG_PRINT("Testing for data on fd %i.\n", here->fd);

		if (FD_ISSET(here->fd, &queue->rfds)) {

			ret_read = here->read(here);

			if (ret_read == EVF_IO_QUEUE_OK)
				continue;

			if (ret_read & EVF_IO_QUEUE_CLOSE)
				close(here->fd);

			if (ret_read & EVF_IO_QUEUE_DFREE)
				free(here->priv);

			FD_CLR(here->fd, &queue->rfds);

			if (ret_read & EVF_IO_QUEUE_REM)
				queue->root = list_delete(queue->root,
				                          here->fd);

		} else
			FD_SET(here->fd, &queue->rfds);
	}

	return ret;
}

int evf_io_queue_add(struct evf_io_queue *queue, int fd,
                     int (*read)(struct evf_io_queue_memb *self), void *priv)
{
	struct evf_io_queue_memb *memb;

	memb = malloc(sizeof (struct evf_io_queue_memb));

	DEBUG_PRINT("Inserting fd %i into queue.\n", fd);

	if (memb == NULL)
		return 0;

	memb->fd   = fd;
	memb->read = read;
	memb->priv = priv;

	FD_SET(fd, &queue->rfds);

	queue->root = list_insert(queue->root, memb);
	queue->cnt++;

	return 1;
}

void evf_io_queue_rem(struct evf_io_queue *queue, int fd)
{
	if (FD_ISSET(fd, &queue->rfds)) {
		FD_CLR(fd, &queue->rfds);
		queue->root = list_delete(queue->root, fd);
		queue->cnt--;
	}
}

unsigned int evf_io_queue_get_count(struct evf_io_queue *queue)
{
	return queue->cnt;
}
