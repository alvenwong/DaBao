#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <string.h>
#include <libconfig.h>
#include <stdlib.h>

#define MAXPORTS		1
#define PORT_LEN		6
#define BASIS_PORT		8880
#define MIN_THREADS		1
#define MAX_THREADS		20
#define IP_LEN			20

struct configs {
	int		ports[MAXPORTS];
	int		ports_count;
	double	delay_in_ms;
	int		threads_num;
	int		redis_concurrency;
	int 	packets_group;
	char	dev[20];
	char	database_ip[IP_LEN];
	int 	database_port;
};

void initConfigs(struct configs *conf);
int getConfigs(struct configs *conf);
int updateEpoch(struct configs *conf);

static char *cfg_file = "settings.cfg";

#endif
