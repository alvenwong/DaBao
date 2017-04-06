#ifndef RING_H
#define RING_H

#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define RING_SIZE 4096
#define BUFSIZE	 200

struct epoll_packet {
	char buf[BUFSIZE];
	unsigned int fd;
	unsigned int efd;
	struct timeval recv;
	struct timeval done;
};

struct ring_buffer {
	pthread_cond_t buffer_not_full;
	pthread_cond_t buffer_not_empty;
	pthread_mutex_t mlock;
	unsigned int head, tail;
	// this buffer is a circular array
	struct epoll_packet *buf;
};

inline struct epoll_packet* ring_buffer_tail(const struct ring_buffer *bufmgr) 
{
	return bufmgr->buf + bufmgr->tail;
}

inline struct epoll_packet* ring_buffer_head(const struct ring_buffer *bufmgr)
{
	return bufmgr->buf + bufmgr->head;
}

inline void ring_buffer_push(struct ring_buffer *bufmgr)
{
	bufmgr->head = (bufmgr->head + 1) & (RING_SIZE - 1);
}

inline void ring_buffer_pop(struct ring_buffer *bufmgr)
{
	bufmgr->tail = (bufmgr->tail + 1) & (RING_SIZE - 1);
}

inline int ring_buffer_used(const struct ring_buffer *bufmgr)
{
	return (bufmgr->head - bufmgr->tail) & (RING_SIZE - 1);
}

inline int ring_buffer_avail(const struct ring_buffer *bufmgr)
{
	return RING_SIZE - ring_buffer_used(bufmgr);
}

inline void ring_buffer_init(struct ring_buffer *bufmgr) 
{
	pthread_cond_init(&bufmgr->buffer_not_full, NULL);
	pthread_cond_init(&bufmgr->buffer_not_empty, NULL);
	pthread_mutex_init(&bufmgr->mlock, NULL);
	bufmgr->head = bufmgr->tail = 0;
	bufmgr->buf = calloc(RING_SIZE, sizeof(struct epoll_packet));	
}

inline void reset_packet(struct epoll_packet *buf)
{
	memset(buf->buf, 0, sizeof(buf->buf));
	buf->fd = 0;
}

#endif
