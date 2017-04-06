#ifndef PROTOCOL_H
#define PROTOCOL_H

#define IPLEN			16

struct client_init_configs {
	int		epoch;
	int		connections;
	int		requestRate;
	int		burst;
	int		vburst;
	char	server_ip[IPLEN];
	int		server_port;
};

struct client_update_configs {
	int		epoch;
	int		connections;
	int		requestRate;
	int		burst;
};

#endif
