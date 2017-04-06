#ifndef EPOLL_H
#define EPOLL_H

#include <stdlib.h>
#include <time.h>
#include <sys/epoll.h>
#include <assert.h>
#include "utils.h"

#define MAX_EVENTS		4000000
#define S_TO_US			1000000
#define GROUP_SIZE      10000

#define TRUE			1
#define FALSE			0

struct packet_time {
	int efd;
	struct timeval	last_send;
	struct timeval	last_recv;
	struct timeval  virtual_time;
	int sendingFlag;
	int connectionClosed;
};

// this struct is for dynamic addition and deletion of connections
struct epoll_connections_stats {
	int connections;
	int active_connections;
	int dead_connections;
	int connsMap[MAX_EVENTS];
};

int createEpoll();
void addEpollEvent(const int efd, const int fd, const int events);
void modifyEpollEvent(const int efd, const int fd, const int events);
void deleteEpollEvent(const int efd, const int fd);
void setEventSendTime(const int fd, const struct timeval *time, const int burst);
void setEventRecvTime(const int fd, const struct timeval *time);
int getTimeDiff(const struct timeval *start, const struct timeval *end);
void setVirtualEventTime(const int fd, const int total, const struct timeval *time, const int epoch, const int burst, const float unit_delay);
int getRTT(const int fd);
int canSendPkt(const int fd, const int epoch);
void resetSendPkt(const int fd);
void setEventTimeEfd(const int efd, const int fd);
void closeConnections(const int efd, const int num);
int restoreConnection(const int efd, const int fd);
int restoreConnections(const int efd, const int num, const int epoch, const int burst); 
void closeAllConnections();

struct packet_time packets_time[MAX_EVENTS];
struct epoll_connections_stats epoll_conns_stats;

#endif
