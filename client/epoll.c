#include "epoll.h"


int createEpoll()
{
	int efd;
	efd = epoll_create1(0);
	if (efd == -1) {
		perror("epoll_create");
		exit(1);
	}
	return efd;
}


void addEpollEvent(const int efd, const int fd, const int events)
{
	struct epoll_event event;

	packets_time[fd].efd = efd;
	packets_time[fd].connectionClosed = 0;
	packets_time[fd].sendingFlag = 0;
	epoll_conns_stats.connections += 1;
	epoll_conns_stats.active_connections += 1;
	event.data.fd = fd;
	assert (events == EPOLLIN || events == EPOLLOUT);
	event.events = events | EPOLLET;
	if ((epoll_ctl (efd, EPOLL_CTL_ADD, fd, &event)) == -1) {
		perror ("epoll_ctl");
		exit (1);
	} 
}


void modifyEpollEvent(const int efd, const int fd, const int events)
{
	struct epoll_event event;

	event.data.fd = fd;
	assert (events == EPOLLIN || events == EPOLLOUT);
	event.events = events | EPOLLET;
	if ((epoll_ctl (efd, EPOLL_CTL_MOD, fd, &event)) == -1) {
		addEpollEvent(efd, fd, events);	
	}
}


void deleteEpollEvent(const int efd, const int fd)
{
	struct epoll_event event;
	packets_time[fd].efd = 0;
	packets_time[fd].connectionClosed = 1;
	epoll_conns_stats.connections -= 1;
	epoll_conns_stats.active_connections -= 1;
	close (fd);
	epoll_ctl(efd, EPOLL_CTL_DEL, fd, &event);
}


void initEpollConnectionStats()
{
	epoll_conns_stats.connections = 0;
	epoll_conns_stats.active_connections = 0;
	epoll_conns_stats.dead_connections = 0;
	int i;
	for (i=0; i<MAX_EVENTS; i++) {
		epoll_conns_stats.connsMap[i] = i;
	}
	shuffer(epoll_conns_stats.connsMap, MAX_EVENTS);	
}

void initEventTime()
{
	int i;

	initEpollConnectionStats();

	struct timeval now;
	gettimeofday(&now, NULL);
	for (i=0; i<MAX_EVENTS; i++) {
		packets_time[i].efd = 0;
		packets_time[i].virtual_time.tv_sec = now.tv_sec;
		packets_time[i].virtual_time.tv_usec = now.tv_sec;
		packets_time[i].sendingFlag = 0;
		packets_time[i].connectionClosed = 1;
	}
}


void setEventSendTime(const int fd, const struct timeval *time, const int interval)
{
	assert(fd < MAX_EVENTS);
	struct timeval now;

	if (time != NULL) {
		packets_time[fd].virtual_time.tv_sec = time->tv_sec;
		packets_time[fd].virtual_time.tv_usec = time->tv_usec;
		packets_time[fd].last_send.tv_sec = time->tv_sec;
		packets_time[fd].last_send.tv_usec = time->tv_usec;
	} else {
		gettimeofday(&now, NULL);
		packets_time[fd].virtual_time.tv_sec = now.tv_sec;
		packets_time[fd].virtual_time.tv_usec = now.tv_usec / interval * interval;
		gettimeofday(&(packets_time[fd].last_send), NULL);
	}
}


void setEventRecvTime(const int fd, const struct timeval *time)
{
	assert(fd < MAX_EVENTS);
	if (time != NULL) {
		packets_time[fd].last_recv.tv_sec = time->tv_sec;
		packets_time[fd].last_recv.tv_usec = time->tv_usec;
	} else {
		gettimeofday(&(packets_time[fd].last_recv), NULL);
	}
}


int getTimeDiff(const struct timeval *start, const struct timeval *end)
{
	return (end->tv_sec - start->tv_sec) * S_TO_US + (end->tv_usec - start->tv_usec);
}


void setVirtualEventTime(const int fd, const int total, const struct timeval *time, const int epoch, const int interval, const float unit_delay)
{
	int sec_offset, usec_offset, virtual_delay;
	int average;
	struct timeval virtual_time;

	average = (epoch * S_TO_US) / total;
	usec_offset = (fd * average) / interval * interval;
	virtual_delay = epoch - (total / GROUP_SIZE) * unit_delay;
	virtual_time.tv_sec = time->tv_sec + (usec_offset / S_TO_US) - virtual_delay;
	virtual_time.tv_usec = usec_offset % S_TO_US;

	setEventSendTime(fd, &virtual_time, interval);
	setEventRecvTime(fd, &virtual_time);
}


int getRTT(const int fd)
{
	assert(fd < MAX_EVENTS);
	return getTimeDiff(&(packets_time[fd].last_send), &(packets_time[fd].last_recv));
}


int canSendPkt(const int fd, const int epoch)
{
	if (packets_time[fd].sendingFlag == 1 || packets_time[fd].connectionClosed == 1) {
		return FALSE;
	}
	struct timeval now;
	gettimeofday(&now, NULL);
	if (getTimeDiff(&(packets_time[fd].virtual_time), &now) / S_TO_US >= epoch) {
		packets_time[fd].sendingFlag = 1;
		return TRUE;
	} else {
		return FALSE;
	}
}


void resetSendPkt(const int fd)
{
	packets_time[fd].sendingFlag = 0;
}


void setEventTimeEfd(const int efd, const int fd)
{
	packets_time[fd].efd = efd;
	packets_time[fd].connectionClosed = 0;
	packets_time[fd].sendingFlag = 0;
}


int closeConnection(const int efd, const int fd)
{
	if (packets_time[fd].efd != efd) {
		return 0;
	}
	if (packets_time[fd].connectionClosed == 1) {
		return 0;
	} else {
		packets_time[fd].connectionClosed = 1;
		epoll_conns_stats.active_connections -= 1;
		epoll_conns_stats.dead_connections += 1;
		return 1;
	}
}


int restoreConnection(const int efd, const int fd)
{
	if (packets_time[fd].efd != efd) {
		return 0;
	}
	if (packets_time[fd].connectionClosed == 1) {
		packets_time[fd].connectionClosed = 0;
		epoll_conns_stats.active_connections += 1;
		epoll_conns_stats.dead_connections -= 1;
		return 1;
	}
}


void closeConnections(const int efd, const int num) 
{
	int closed_total = num;
	int closed_failure = 0;
	int index = 1;
	int fd;
	
	while (closed_total > 0) {
		fd = epoll_conns_stats.connsMap[index];
		if (closeConnection(efd, fd) == 1) {
			closed_total -= 1;
		}
		index += 1;
		if (index >= MAX_EVENTS) {
			break;
		}
	}
}


int restoreConnections(const int efd, const int num, const int epoch, const int interval) 
{
	int restore_total = num;
	int index = 0;
	int fd;
	struct timeval now;
	gettimeofday(&now, NULL);
	while (restore_total > 0) {
		if (epoll_conns_stats.dead_connections == 0 || index >= MAX_EVENTS) {
			epoll_conns_stats.dead_connections = 0;
			return restore_total;
		}
		fd = epoll_conns_stats.connsMap[index];
		if (restoreConnection(efd, fd) == 1) {
			setVirtualEventTime(fd, epoll_conns_stats.connections, &now, epoch, interval, 0.1);
			restore_total -= 1;
		}
		index += 1;
	}
	return restore_total;
}


void closeAllConnections()
{
	int efd, fd;
	for (fd=0; fd<MAX_EVENTS; fd++) {
		efd = packets_time[fd].efd;
		if (efd != 0) {
			deleteEpollEvent(efd, fd);
		}
	}
}
