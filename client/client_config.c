#include "client_config.h"

char *cfg_file = "settings.cfg";

double validRequestRate(const double rate)
{
    if (rate > 1) {
        return 1.0;
    } else if (rate < 0) {
        return  0.0;
    } else {
        return rate;
    }
}


void initConfigs(struct epoll_configs *conf)
{
	conf = malloc(sizeof(struct epoll_configs));
	memset(conf, 0, sizeof(struct epoll_configs));
	conf->epoch	      = 60;
	conf->connections = 320;
	conf->interval	  = 10000;
	conf->kill		  = 0;
	conf->unit_delay  = 0.0;
}


int getConfigs(struct epoll_configs *conf) 
{
	config_t cfg;
	config_setting_t *ports;
	const char *ip;
	const char *device;
	int ports_count, n;
	double requestRate;

	config_init(&cfg);
	if (! config_read_file(&cfg, cfg_file)) {
		perror("config_read_file");
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	config_lookup_int(&cfg, "epoch", &(conf->epoch));
	config_lookup_int(&cfg, "connections", &(conf->connections));
	config_lookup_int(&cfg, "burst", &(conf->burst));
	conf->burst = (conf->connections * 1.0) / (BASE_CONNS * 1.0) * conf->burst;
	if (conf->connections != 0) { 
		conf->interval = (S_TO_US * conf->epoch) / conf->connections * conf->burst;
	} else {
		conf->interval = 10000;
	}
	printf("burst: %d\n", conf->burst);
	ports = config_lookup(&cfg, "ports");
	ports_count = config_setting_length(ports);
	ports_count = ports_count >= MAXPORTS ? MAXPORTS : ports_count;
	for (n=0; n<ports_count; n++) {
		conf->ports[n] = config_setting_get_int_elem(ports, n);
		conf->ports_count += 1;
	}
	config_lookup_float(&cfg, "requestRate", &requestRate);
	config_lookup_string(&cfg, "dest_ip", &ip);
	config_lookup_string(&cfg, "dev", &device);
	strcpy(conf->dest_ip, ip);
	strcpy(conf->dev, device);
    conf->requestRate = validRequestRate(requestRate);
	config_lookup_int(&cfg, "SID_start", &(conf->SID_start));
	config_lookup_int(&cfg, "SID_end", &(conf->SID_end));
	config_lookup_float(&cfg, "unit_delay", &(conf->unit_delay));
	config_destroy(&cfg);
	return (EXIT_SUCCESS);
}


int updateEpoch(struct epoll_configs *conf) 
{
	config_t cfg;
	unsigned int epoch, connections, prior_conns, burst, kill;
	double rate;
	int flag = 0;

	config_init(&cfg);
	if (! config_read_file(&cfg, cfg_file)) {
		perror("config_read_file");
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	if (config_lookup_int(&cfg, "epoch", &epoch)) {
		if (epoch != conf->epoch) {
			printf("frequency updated from %d to %d\n", conf->epoch, epoch);
			conf->epoch = epoch;
		}
	} else {
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}
	if (config_lookup_int(&cfg, "burst", &burst)) {
		if (burst != conf->burst) {
			conf->burst = burst;
		}
	} else {
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}
	if (config_lookup_int(&cfg, "connections", &connections)) {
		prior_conns = conf->connections;
		if (connections != prior_conns) {
            if (connections > prior_conns) {
                flag = INCRE;
            } else {
                flag = DECRE;
            }
			printf("connections updated from %d to %d\n", prior_conns, connections);
			conf->connections = connections;
			conf->burst = (conf->connections * 1.0) / (BASE_CONNS * 1.0) * conf->burst;
			if (conf->connections != 0) {
				conf->interval = (S_TO_US * conf->epoch) / conf->connections * conf->burst;
			} else {
				conf->interval = 10000;
			}
		}
	} else {
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}
	if (config_lookup_float(&cfg, "requestRate", &rate)) {
		rate = validRequestRate(rate);
		if (rate != conf->requestRate) {
			printf("frequency updated from %.2f to %.2f\n", conf->requestRate, rate);
			conf->requestRate = rate;
		}
	} else {
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}
	if (config_lookup_int(&cfg, "kill", &kill)) {
		if (kill != conf->kill) {
			conf->kill = kill;
		}
	} else {
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}
	config_destroy(&cfg);
	return flag;
}
