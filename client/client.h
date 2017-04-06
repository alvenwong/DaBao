#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ifaddrs.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "payload.h"
#include "serialize.h"
#include "client_config.h"
#include "statistics.h"
#include "epoll.h"
#include "DES/payload_cipher.h"

//#define DEBUG
#define TPUT
//#define CIPHER

#define BUFF_SIZE		1500
#define MAXIPS			300
#define MAXCONNECTIONS	50000
#define MAX_CONN_PER_PORT 60000

#define STATE_PKT		0
#define REQUEST_PKT		1
#define HEARTBEAT_PKT	2

#define USER_TO_SERVER	0x00
#define SERVER_TO_USER	0x01
#define APP_TO_SERVER	0x02
#define SERVER_TO_APP	0x03
#define THREADNUM		20
#define MAX_BURST		1

#define PAUSETHRES		10000
#define PAUSETIME		0
#define RATE_ACCURACY	100
#define S_TO_MS			1000
#define DEBUG_GROUP		10000



struct epoll_thread {
	int		total;
	int		mask;
	int		port;
};

struct global_params {
	int		efds[MAXPORTS];
	int		connections[MAXPORTS];
	int		close_conns[MAXPORTS];
	int		failed_conns[MAXPORTS];
	int		client_ips;
	struct	sockaddr_in clients[MAXIPS];
	struct	sockaddr_in server[MAXPORTS];
	char	aliasIPs[MAXIPS][NI_MAXHOST];
};


static unsigned int connections_counter;
static unsigned int pktTypeCounter;
static struct global_params *params;

static void createEvents(const int connections, struct sockaddr_in server, const int efd);

#endif
