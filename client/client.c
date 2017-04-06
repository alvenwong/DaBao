#define _GNU_SOURCE
#include "client.h"


static int getPktTypeNum(const int upper_bound)
{
	pktTypeCounter += 1;
	if (pktTypeCounter <= upper_bound) {
		return REQUEST_PKT;
	} else if (pktTypeCounter >= upper_bound && pktTypeCounter < RATE_ACCURACY) {
		return HEARTBEAT_PKT;
	} else {
		pktTypeCounter = 0;
		return HEARTBEAT_PKT;
	}
}


static int getPktType(const int upper_bound) 
{
	return getPktTypeNum(upper_bound);
}


static int addConnections(const int conns) 
{
	int conns_per_port, port, restore_left;
	int ports = conf->ports_count;
	conns_per_port = conns / ports;

	for (port=0; port<ports; port++) {
		restore_left = restoreConnections(params->efds[port], conns_per_port, conf->epoch, conf->interval);
#ifdef DEBUG
		printf("add %d connections, left %d connections\n", conns_per_port, restore_left);
#endif
		createEvents(restore_left, params->server[port], params->efds[port]);
		params->connections[port] += conns_per_port;
	}
	return 0;
}


static int deleteConnections(const int conns)
{
	int conns_per_port, port;
	int ports = conf->ports_count;
	conns_per_port = conns / ports;

	for (port=0; port<ports; port++) {
		params->close_conns[port] += conns_per_port;
	}
	return 0;
}


static int updateConnections(const int prior_conns, const int conns)
{
	int diff;
	if (prior_conns < conns) {
		diff = conns - prior_conns;
		return addConnections(diff);
	} else if (prior_conns > conns) {
		diff = prior_conns - conns;
		return deleteConnections(diff);
	} else {
		return 0;
	}
}


void *update_cfg(void *context)
{
    int flag = 0;
    int prior_conns;
	while (1) {
		sleep(5);
        prior_conns = conf->connections;
		flag = updateEpoch(conf);
		if (conf->kill == 1) {
#ifdef DEBUG
			printf("kill the program. close all the connections...\n");
#endif
			closeAllConnections();	
#ifdef DEBUG
			printf("closed successfully. exit!\n");
#endif
			exit(1);
		}
        if (flag == INCRE || flag == DECRE) {
            updateConnections(prior_conns, conf->connections);  
        }
	}
}


static void getAliasIPs()
{
	assert(params->client_ips == 0);

	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char host[NI_MAXHOST];
	
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddr");
		exit(EXIT_FAILURE);
	}
	for (ifa=ifaddr; ifa!=NULL; ifa=ifa->ifa_next) {
		if (ifa->ifa_addr == NULL) {
			continue;
		}
		family = ifa->ifa_addr->sa_family;
		if (family != AF_INET || strstr(ifa->ifa_name, conf->dev) == NULL) {
			continue;
		}

		s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), 
						host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (s != 0) {
			perror("getnameinfo");
			exit(EXIT_FAILURE);
		}
		strcpy(params->aliasIPs[params->client_ips], host);
#ifdef DEBUG
		printf("ip_index: %d, ip: %s\n", params->client_ips, params->aliasIPs[params->client_ips]);
#endif
		if (params->client_ips >= MAXIPS) {
			break;
		}
		params->client_ips += 1;
	}
	freeifaddrs(ifaddr);
}

static void createClients()
{
	int ips = params->client_ips;
	assert(ips < MAXIPS);
	int ip;
	for (ip=0; ip<ips; ip++) {
		bzero (&(params->clients[ip]), sizeof(struct sockaddr_in));
		params->clients[ip].sin_family = AF_INET;
		params->clients[ip].sin_port = htons(0);
		params->clients[ip].sin_addr.s_addr = inet_addr(params->aliasIPs[ip]);
	}
}


static void createServer()
{
	char dest_ip[NI_MAXHOST];
	int n, count;
	strcpy(dest_ip, conf->dest_ip);
	count = conf->ports_count;

    if ((gethostbyname(dest_ip)) == NULL) {
		perror ("gethostbyname error. \n");
		exit (1);
	}
	for (n=0; n<count; n++) {
		bzero (&(params->server[n]), sizeof(struct sockaddr_in));
		params->server[n].sin_family = AF_INET;
		params->server[n].sin_port = htons(conf->ports[n]);
		params->server[n].sin_addr.s_addr = inet_addr(dest_ip);
	}
}


static void pauseSignal(int *counter, const int thres)
{
	*counter += 1;
	if (*counter >= thres) {
		sleep(PAUSETIME);
		*counter = 0;
	}
}


static void createEvents(const int connections, struct sockaddr_in server, const int efd)
{
	int sockfd;
	struct epoll_event event;
	int average, remainder;
	int ip, ips, connection;
	int error;
	int total = connections;
	int connections_counter = 0;
	struct sockaddr_in *client;
	struct timeval time, start, end;

	ips = params->client_ips;

		average = total / ips;
		if (average > MAX_CONN_PER_PORT) {
			average = MAX_CONN_PER_PORT;
			total = ips * average;
#ifdef DEBUG
			printf("Run out of ports, reduce the number of connections to %d.\n", total);
#endif
		}
		remainder = total % ips;

		gettimeofday(&time, NULL);
		gettimeofday(&start, NULL);
		for (ip=0; ip<ips; ip++) {
			client = &(params->clients[ip]);
			for (connection=0; connection<average; connection++) {
				if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
					perror ("socket error. \n");
					exit(EXIT_FAILURE);
				}
				if ((bind(sockfd, (struct sockaddr*)client, sizeof(struct sockaddr_in))) < 0) {
					perror ("bind error. \n");
					continue;
				}
				make_socket_nonblocking(sockfd);
#ifdef DEBUG
				if (connection % DEBUG_GROUP == 0 ) {
					printf("fd: %d, connection: %d, ip: %d, total: %d, avg: %d\n", sockfd, connection, ip, total, average);
				}
#endif
				if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
					if (errno != EINPROGRESS) {
						close(sockfd);
						perror ("connect error.\n");
						continue;
					}
				}
				setEventTimeEfd(efd, sockfd);
				pauseSignal(&connections_counter, PAUSETHRES);
				// set virtual event send time and recv time
				setVirtualEventTime(sockfd, total, &time, conf->epoch, conf->interval, conf->unit_delay);
			}
		}
	gettimeofday(&end, NULL);
//#ifdef DEBUG
	fprintf(stderr, "time: %d\n", getTimeDiff(&start, &end));
//#endif
}


static void epollLoop(const int efd, const int port)
{
	int nfds, fd;
	int error;
	int type;
	int upper_bound;
	int i, count, out_counter;
	char buf[BUFF_SIZE];
	struct epoll_event * events;
	struct epoll_event event;
	int delay_in_us, packets_per_50us;
	double requestRate;
	int epoch, total, close_conns;

	events = calloc(MAXCONNECTIONS, sizeof(struct epoll_event));
	while (1) {
		upper_bound = conf->requestRate * RATE_ACCURACY;
		nfds = epoll_wait(efd, events, MAXCONNECTIONS, -1);

		close_conns = params->close_conns[port];
		if (close_conns > 0) {
			closeConnections(efd, close_conns);	
	        params->connections[port] -= close_conns;
			params->close_conns[port] -= close_conns;
		}

		for (i=0; i<nfds; i++) {
			fd = events[i].data.fd;
			if ((events[i].events & EPOLLERR) ||
				(events[i].events & EPOLLHUP)) {
					deleteEpollEvent(efd, fd);
	                params->connections[port] -= 1;
					continue;
			} else if (events[i].events & EPOLLOUT) {
				struct packet_format payload;
				unsigned char payload_buf[PAYLOAD_SIZE], *pbuf;
				type = getPktType(upper_bound);
				initPayload(&payload);
				setPayload(&payload, type, USER_TO_SERVER, fd);
				pbuf = serialize_payload(payload_buf, &payload);
#ifdef CIPHER
                unsigned char payload_cipher[PAYLOAD_SIZE];
                memcpy(payload_cipher, payload_buf, PAYLOAD_OFFSET);
                encrypt_payload(payload_buf+PAYLOAD_OFFSET, payload_buf + ZERO_FILL_OFFSET, payload_cipher+PAYLOAD_OFFSET);
				count = write(fd, payload_cipher, pbuf - payload_buf);
#else
				count = write(fd, payload_buf, pbuf - payload_buf);
#endif
				if (count == -1) {
					if (errno != EINTR) {
						perror("write error.");
						deleteEpollEvent(efd, fd);
	                    params->connections[port] -= 1;
					}
				} else if (count == 0) {
					// the connection has been closed
					deleteEpollEvent(efd, fd);
	                params->connections[port] -= 1;
				} else {
                    increSendPkts();
					if (type == REQUEST_PKT) {
                        increRequestPkts();
						modifyEpollEvent(efd, fd, EPOLLIN);
					} else {
						resetSendPkt(fd);
					}
					setEventSendTime(fd, NULL, conf->interval);
				}
			} else if (events[i].events & EPOLLIN) {
				fd = events[i].data.fd;
				count = read(fd, buf, BUFF_SIZE);
				if (count == -1) {
					if (error != EAGAIN) {
						perror("read error");
						deleteEpollEvent(efd, fd);
	                    params->connections[port] -= 1;
					}
					continue;
				} else if (count == 0) {
					// the connection has been closed
					perror("the connection has been closed by server.\n");
					deleteEpollEvent(efd, fd);
	                params->connections[port] -= 1;
					continue;
				}
				setEventRecvTime(fd, NULL);
				resetSendPkt(fd);
                increReceivePkts(getRTT(fd));
			} else {
				continue;
			}
		}
	}
	free(events);
}

void *epoll_client(void *context)
{
	struct epoll_thread *param;
	param = (struct epoll_thread *)context;
	int total, port;
	struct epoll_event event;
	int efd, error;
	int sockfd = -1;
	int conn_id;
	
	total = param->total;
	port = param->port;
	
	efd = createEpoll();
	params->efds[port] = efd;
	params->connections[port] = total;
	// create connections
	createEvents(total, params->server[port], efd);
	epollLoop(efd, port);

	return 0;
}

static void initGlobal()
{
	int i;
	params = malloc(sizeof(struct global_params));
	params->client_ips = 0;
	for (i=0; i<MAXPORTS; i++) {
		params->efds[i] = 0;
		params->connections[i] = 0;
		params->close_conns[i] = 0;
		params->failed_conns[i] = 0;
	}
}


static void initParams()
{
    conf = malloc(sizeof(struct epoll_configs));
	initConfigs(conf);
	initGlobal();
}


static void init()
{
	initParams();
	getConfigs(conf);
	getAliasIPs();
	initEventTime();
	initIDMap(conf->SID_start, conf->SID_end);
	initKeyTable();
	createClients();
	createServer();
}


void *updateEvents(void *context)
{
	int fd;
	struct timeval now;
	
	while (1) {
		for (fd=0; fd<MAX_EVENTS; fd++) {
			if (packets_time[fd].efd == 0) {
				continue;
			}
			if (canSendPkt(fd, conf->epoch) == TRUE) {
				modifyEpollEvent(packets_time[fd].efd, fd, EPOLLOUT);
			}
		}
	}
}


int main(int argc, char *argv[])
{
	int rc, n, total, ports;
	struct epoll_thread param[THREADNUM];

	pthread_t client_thread[THREADNUM], throughput_sec_thread, throughput_min_thread, update_thread, admin_thread;
	pthread_t update_events_thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	cpu_set_t cpus;

	init();
	ports = conf->ports_count;
	total = conf->connections;
#ifdef DEBUG
	printf ("the total connections is %d\n", total);
#endif
	if (total == 0) {
		printf("the number of connnections is zero!\n");
		return 0;
	}

	for (n=0; n<ports; n++) {
		param[n].total = total/ports;
		param[n].mask = n+1;
		param[n].port = n;
		CPU_ZERO(&cpus);
		CPU_SET(n+1, &cpus);
		pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
		rc = pthread_create(&client_thread[n], &attr, epoll_client, &param[n]);
		if (rc) {
			printf("ERROR.\n");
			exit(-1);
		}
	}
#ifdef TPUT
	CPU_ZERO(&cpus);
	CPU_SET(ports+1, &cpus);
	pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
	rc = pthread_create(&throughput_sec_thread, &attr, throughput_sec, NULL);
    if (rc) {
		printf("ERROR.\n");
		exit(-1);
	}
	CPU_ZERO(&cpus);
	CPU_SET(ports+1, &cpus);
	pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
	rc = pthread_create(&throughput_min_thread, &attr, throughput_min, NULL);
    if (rc) {
		printf("ERROR.\n");
		exit(-1);
	}
#endif
	CPU_ZERO(&cpus);
	CPU_SET(ports+2, &cpus);
	pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
	rc = pthread_create(&update_thread, &attr, update_cfg, 0);
    if (rc) {
		printf("ERROR.\n");
		exit(-1);
	}
	CPU_ZERO(&cpus);
	CPU_SET(ports+3, &cpus);
	pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
	rc = pthread_create(&update_events_thread, &attr, updateEvents, 0);
    if (rc) {
		printf("ERROR.\n");
		exit(-1);
	}
	pthread_exit(NULL);
}
