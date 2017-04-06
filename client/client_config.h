#ifndef CLIENT_CONFIG_H
#define CLIENT_CONFIG_H

#include <stdlib.h>
#include <libconfig.h>
#include <errno.h>
#include <string.h>

#define MAXPORTS	1

#define INCRE       1
#define DECRE       2
#define S_TO_US		1000000
#define BASE_CONNS	100000

struct epoll_configs {
	char	dest_ip[20];
	int		ports[MAXPORTS];
	int		ports_count;
	char	dev[10];
	double	requestRate;
	int		epoch;
	int		connections;
	int		burst;
	int		SID_start;
	int		SID_end;
	int		kill;
	double  unit_delay;
	int		interval;
};


void initConfigs(struct epoll_configs *conf);
int getConfigs(struct epoll_configs *conf); 
int updateEpoch(struct epoll_configs *conf); 

struct epoll_configs *conf;

#endif
