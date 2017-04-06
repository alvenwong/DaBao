#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include "socket_tools.h"
#include "redis/redis.h"
#include "ring.h"
#include "server_config.h"
#include "serialize.h"
#include "statistics.h"

//#define TPUT
//#define TIME
#define DATABASE
#define CIPHER

#define MAXEVENTS		100000
#define BUFF_SIZE		200
#define TIMES			7000
#define MS_TO_US		1000

#define PKT_TYPES		3
#define STATE_PKT		0
#define REQUEST_PKT		1
#define HEARTBEAT_PKT	2

#define USER_TO_SERVER	0x00
#define SERVER_TO_USER	0x01
#define APP_TO_SERVER	0x02
#define SERVER_TO_APP	0x03

#define SUCCESS			0
#define FAILURE			1

#define EPOLL_CPU		1
#define BASIS_CPU		2


struct thread_info {
	int index;
};

struct thread_epoll_info {
	int port;
	int cpu;
};

static int usr_num;
struct ring_buffer *packets_for_thread[MAX_THREADS];
struct configs *conf;  

#endif
