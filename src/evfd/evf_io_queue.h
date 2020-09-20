// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2020 Cyril Hrubis <metan@ucw.cz>
 */
/*

   IO queue for traditional unix select() call (tested only on linux).

   Member in io queue consists of file descriptor and callback, that is called,
   when data on fd are ready.

 */

#ifndef EVF_IO_QUEUE_H__
#define EVF_IO_QUEUE_H__

#include <sys/select.h>

/*
 * Returns values for queue member callback.
 *
 * Use as bitfields, to automatically remove member from queue and close fd:
 *
 * return EVF_IO_QUEUE_REM | EVF_IO_QUEUE_CLOSE;
 */
#define EVF_IO_QUEUE_OK    0x00 /* read was succesfull, continue */
#define EVF_IO_QUEUE_REM   0x01 /* remove memb from select queue */
#define EVF_IO_QUEUE_CLOSE 0x02 /* close fd                      */
#define EVF_IO_QUEUE_DFREE 0x04 /* call free on void *priv       */

/*
 * Select queue member.
 *
 * They are saved in sorted linked list, so that
 * maximum is O(1) and other functions O(N), but
 * as we are checking select flags in O(N) anyway
 * there is no reason to make it faster.
 */
struct evf_io_queue_memb {
	int fd;
	int (*read)(struct evf_io_queue_memb *self);
	void *priv;

	struct evf_io_queue_memb *next;
};

struct evf_io_queue {
	unsigned int cnt;
	fd_set rfds;
	struct evf_io_queue_memb *root;
};

/*
 * Create and initalize new queue. Uses malloc, may
 * fail and return NULL.
 */
struct evf_io_queue *evf_io_queue_new(void);

/*
 * Destroy, you can pass EVF_IO_QUEUE_CLOSE and EVF_IO_QUEUE_DFREE as a flag.
 */
void evf_io_queue_destroy(struct evf_io_queue *queue, int flags);

/*
 * Wait for data on any file descriptor or timeout.
 */
int evf_io_queue_wait(struct evf_io_queue *queue, struct timeval *timeout);

/*
 * Insert fd, its read function and priv pointer into queue.
 *
 */
int evf_io_queue_add(struct evf_io_queue *queue, int fd,
                     int (*read)(struct evf_io_queue_memb *self), void *priv);

/*
 * Remove fd from queue, filedescriptor is not closed here!.
 */
void evf_io_queue_rem(struct evf_io_queue *queue, int fd);

/*
 * Returns number of members in the queue.
 */
unsigned int evf_io_queue_get_count(struct evf_io_queue *queue);

/*
 * Loop trough all queue memebers.
 */
#define EVF_IO_QUEUE_MEMB_LOOP(queue, i) \
	for ((i) = (queue)->root; (i) != NULL; (i) = (i)->next)

#endif /* EVF_IO_QUEUE_H__ */
