#include "server_config.h"

int isValidThreadNum(int threads_num) 
{
	if (threads_num < MIN_THREADS) {
		return 0;
	} else if (threads_num > MAX_THREADS) {
		return 0;
	} else {
		return 1;
	}
}

void initConfigs(struct configs *conf)
{
	memset(conf->ports, 0, MAXPORTS*sizeof(int));
	conf->ports_count = 0;
	conf->delay_in_ms = 0;
	conf->threads_num  = 1;
	conf->packets_group = 1;
}


int getConfigs(struct configs *conf)
{
	config_t cfg;
	config_setting_t *ports;
	int ports_count, n;
	double delay_in_ms;
	int threads_num, packets_group;
	const char *device, *db_ip;

	config_init(&cfg);
	if (!config_read_file(&cfg, cfg_file)) {
		perror("config_read_file");
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	ports = config_lookup(&cfg, "ports");
	ports_count = config_setting_length(ports);
	ports_count = ports_count >= MAXPORTS ? MAXPORTS : ports_count;
	for (n=0; n<ports_count; n++) {
		conf->ports[n] = config_setting_get_int_elem(ports, n);
		conf->ports_count += 1;
	}

	config_lookup_float(&cfg, "delay_in_ms", &delay_in_ms);
	conf->delay_in_ms = delay_in_ms;
	config_lookup_int(&cfg, "threads_num", &threads_num);
	if (!isValidThreadNum(threads_num)) {
		conf->threads_num = MIN_THREADS;
	} else {
		conf->threads_num = threads_num;
	}
	config_lookup_int(&cfg, "packets_group", &packets_group);
	if (packets_group <= 0) {
		conf->packets_group = 1;
	} else {
		conf->packets_group = packets_group;
	}
	config_lookup_string(&cfg, "device", &device);
	strcpy(conf->dev, device);
	config_lookup_string(&cfg, "database_ip", &db_ip);
	strcpy(conf->database_ip, db_ip);
	config_lookup_int(&cfg, "database_port", &(conf->database_port));
	config_destroy(&cfg);
	return (EXIT_SUCCESS);
}


int updateEpoch(struct configs *conf)
{
	config_t cfg;
	double delay_in_ms;
	int packets_group;

	config_init(&cfg);
	if (! config_read_file(&cfg, cfg_file)) {
		perror("config_read_file");
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}
	if (config_lookup_float(&cfg, "delay_in_ms", &delay_in_ms)) {
		if (delay_in_ms != conf->delay_in_ms) {
			printf("delay_in_ms updated from %f to %f\n", conf->delay_in_ms, delay_in_ms);
			conf->delay_in_ms = delay_in_ms;
		}
	} else {
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}
	if (config_lookup_int(&cfg, "packets_group", &packets_group)) {
		if (packets_group != conf->packets_group) {
			printf("packets_group updated from %d to %d\n", conf->packets_group, packets_group);
			conf->packets_group = packets_group;
		}
	} else {
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}
	config_destroy(&cfg);
	return (EXIT_SUCCESS);
}
