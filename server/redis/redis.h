#ifndef REDIS_H
#define REDIS_H

#include "connect.h"

#define USER_DEVICE_DB	1
#define USER_KEY_DB		2
#define	DEVICE_STATE_DB	3

#define KEY_LEN			80
#define VALUE_LEN		80

#define GET			1
#define SET			2


clusterInfo* connectRedis(char *database_ip, int database_port);
void disconnectDatabase(clusterInfo* cluster);
int get(clusterInfo* cluster, const char *key, char *get_in_value, const int dbnum);
int set(clusterInfo* cluster, const char *key, const char *set_in_value, const int dbnum);
void flushDatabase(clusterInfo * cluster);

#endif
