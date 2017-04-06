#include "redis.h"

//#define DEBUG

clusterInfo* connectRedis(char *database_ip, int database_port)
{
     __connect_cluster(database_ip, database_port);
}

void disconnectDatabase(clusterInfo* cluster)
{
    __global_disconnect(cluster);
    __remove_context_from_cluster(cluster);
	flushDb(cluster);
}

void flushDatabase(clusterInfo * cluster) 
{
	flushDb(cluster);
}

int get(clusterInfo* cluster, const char *key, char *get_in_value, const int dbnum)
{
      return  __get_withdb(cluster, key, get_in_value, dbnum);
}

int set(clusterInfo* cluster, const char *key, const char *set_in_value, const int dbnum)
{
	return __set_withdb(cluster, key, set_in_value, dbnum);
}
