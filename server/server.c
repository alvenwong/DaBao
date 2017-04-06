#define _GNU_SOURCE
#include "server.h"


pthread_mutex_t lock;
static unsigned counter;


void *update_cfg(void *context)
{
	while (1) {
		sleep(10);
		updateEpoch(conf);
	}
}

//create and bind a tcp socket
//parameter : port
//return : sockfd
static int create_and_bind (const char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset (&hints, 0, sizeof (struct addrinfo));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //All interface
		
	s = getaddrinfo (NULL, port, &hints, &result);
	if (s != 0) {
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;
		s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0) {
			break;
		}
		close (sfd);
	}
	if (rp == NULL) {
		fprintf (stderr, "could not bind \n");
		return -1;
	}
	freeaddrinfo (result);

	return sfd;
}

static void createServerFDs(const char ports[MAXPORTS][PORT_LEN], int server_fds[], const int num)
{
	int fd, n;
	char port[10];
	for (n=0; n<num; n++) {
		fd = create_and_bind(ports[n]);
		if (fd == -1) {
			perror ("create and bind error.\n");
			exit(1);
		}
		server_fds[n] = fd;
	}
}

//set socket -> non-blocking
static int make_socket_non_blocking (int sfd)
{
	int flags, s;

	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1) {
		perror ("fcntl");
		return -1;
	}
	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1) {
		perror ("fcntl");
		return -1;
	}

	return 0;
}


static void decr_usr_num()
{
	pthread_mutex_lock(&lock);
	usr_num --;
	pthread_mutex_unlock(&lock);
#ifdef DEBUG
	printf ("online usr %d\n",usr_num);
#endif
}

static void incr_usr_num()
{
	pthread_mutex_lock(&lock);
	usr_num ++;
	pthread_mutex_unlock(&lock);
#ifdef DEBUG
	printf ("online usr %d\n",usr_num);
#endif
}


int getLocalIP(char *device, char *IP)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;  
	}

        s=getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if((strcmp(ifa->ifa_name, device) == 0) && (ifa->ifa_addr->sa_family == AF_INET)) {
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
			strcpy(IP, host);
			return SUCCESS;
        }
    }
    freeifaddrs(ifaddr);
	return FAILURE;
}


static void setDirection(struct packet_format *payload, const unsigned char direction)
{
	payload->direction = direction;
}

static unsigned char getPktType(struct packet_format *payload)
{
	return payload->type;
}


static int getTimeDiff(struct timeval start, struct timeval end) 
{
	return (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
}


static void nop(const unsigned int delay_in_ms)
{
	if (delay_in_ms == 0) {
		return;
	}
	struct timeval start, end;
	int sum = 0;
	int i;
	gettimeofday(&start, NULL);
	while (1) {
		gettimeofday(&end, NULL);
		// waste time
		for (i=0; i<100; i++) {
			sum += i;
		}
		if (getTimeDiff(start, end) >= delay_in_ms) {
			break;
		}
	}
}


static int queryDatabase(const char *key, const char *cmpValue, clusterInfo *cluster, const int database, const int op)
{
	char value[VALUE_LEN];
	int error;

	if (op == GET) {
		error = get(cluster, key, value, database);
		if (error == 0) {
			if (strcmp(value, "nil") == 0) {
				set(cluster, key, cmpValue, database);
			}
			return SUCCESS;
		} else {
			return FAILURE;
		}
	} else if (op == SET) {
		set(cluster, key, cmpValue, database);
		return SUCCESS;
	} else {
		return SUCCESS;
	}
}


static void processDatabase(struct packet_format *payload, clusterInfo *cluster)
{
	char userID[KEY_LEN], deviceID[KEY_LEN], state[KEY_LEN], SID[KEY_LEN];
	sprintf(userID, "%d", payload->userID[0]);
	sprintf(deviceID, "%d", payload->dstDeviceID[0]);
	sprintf(state, "%d", payload->state[0]);
	sprintf(SID, "%d", payload->userID[1]);
	queryDatabase(userID, deviceID, cluster, USER_DEVICE_DB, GET);
	queryDatabase(userID, state, cluster, DEVICE_STATE_DB, SET);
	queryDatabase(SID, deviceID, cluster, USER_KEY_DB, GET);
}


static void setServerSocket(const int sfd)
{
	int error;
	int keepalive = 1, reuseaddr = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int))) {
		perror("setsockopt failed.\n");
		exit(1);
	}
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int))) {
		perror("setsockopt failed.\n");
		exit(1);
	}
	if ((make_socket_non_blocking (sfd)) == -1 ) {
		exit(1);
	}
}

static void setClientSocket(const int fd)
{
	int error;
	int keepalive = 1, reuseaddr = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int))) {
		perror("setsockopt failed.\n");
		exit(1);
	}
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int))) {
		perror("setsockopt failed.\n");
		exit(1);
	}
	if ((make_socket_non_blocking (fd)) == -1 ) {
		exit(1);
	}
}

static void addEpollEvent(const int efd, const int fd, const int events)
{
	struct epoll_event event;

	event.data.fd = fd;
	assert (events == EPOLLIN || events == EPOLLOUT);
	event.events = events | EPOLLET;
	if ((epoll_ctl (efd, EPOLL_CTL_ADD, fd, &event)) == -1) {
		perror ("epoll_ctl");
		exit (1);
	}
	incr_usr_num();
}


static void modifyEpollEvent(const int efd, const int fd, const int events)
{
	struct epoll_event event;

	event.data.fd = fd;
	assert (events == EPOLLIN || events == EPOLLOUT);
	event.events = events | EPOLLET;
	if ((epoll_ctl (efd, EPOLL_CTL_MOD, fd, &event)) == -1) {
		perror ("epoll_ctl");
		exit (1);
	}
}


static void acceptEpollEvents(const int efd, const int sfd)
{
	struct sockaddr in_addr;
	struct epoll_event event;
	socklen_t in_len;
	int infd;
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	int s;

	while(1) {
		in_len = sizeof (in_addr);
		infd = accept (sfd, &in_addr, &in_len);
		if (infd == -1) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				//we have processed all incomming connectings
				break;
			} else {
				perror ("accept");
				break;
			}
		}
		s = getnameinfo (&in_addr, in_len, hbuf, sizeof hbuf, 
						sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV);
		if (s == 0)	{
#ifdef DEBUG
			printf("Accept connection on description %d, host=%s, port=%s\n",
					infd, hbuf, sbuf);
#endif
		}
		//make the incoming socket non-blocking 
		//and add it to the list of fds to monitor
		setClientSocket(infd);
		addEpollEvent(efd, infd, EPOLLIN);
	}
}


static void deleteEpollEvent(const int efd, const int fd)
{
	struct epoll_event event;
	close (fd);
	epoll_ctl(efd, EPOLL_CTL_DEL, fd, &event);
	decr_usr_num();
}


static void processRequestPkt(const int efd, const int fd, struct packet_format *payload)
{
	unsigned char payload_buf[PAYLOAD_SIZE], *pbuf;
	unsigned int count = 0;
	pbuf = serialize_payload(payload_buf, payload);
	count = write(fd, payload_buf, pbuf - payload_buf);
	if (count == -1) {
		deleteEpollEvent(efd, fd);
	} else {
		increSendPkts();
	}
}


static int getThreadIndex()
{
	return ((stats.receive_counter_min / conf->packets_group) % conf->threads_num);
}


static void processEPOLLIN(const int efd, const int fd)
{
	int socketfd = fd;
	char buf[BUFF_SIZE];
	int count, type;
	int pkt_size = 0;

	while(1) {
		count = read (socketfd, buf, PAYLOAD_SIZE);
		if (count == -1) {
			//if errno == EAGAIN, that means we have read all data.
			//So go back to the main loop
			if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
				perror ("read");
				deleteEpollEvent(efd, socketfd);
				return;
			} else {
				break;
			}
		} else if (count == 0) {
			//end of file. The remote has closed the connected.
			deleteEpollEvent(efd, socketfd);
			return;
		}
	}
	increReceivePkts();
	int thread = getThreadIndex();
	struct ring_buffer *ring = packets_for_thread[thread];
	pthread_mutex_lock(&ring->mlock);
	if (ring_buffer_avail(ring) <= 1) {
		pthread_cond_wait(&ring->buffer_not_full, &ring->mlock);
	}
	struct epoll_packet *packet = ring_buffer_head(ring);
	memcpy(packet->buf, buf, PAYLOAD_SIZE);
	packet->fd = socketfd;
	packet->efd = efd;
	gettimeofday(&packet->recv, NULL);
	ring_buffer_push(ring);
	pthread_mutex_unlock(&ring->mlock);
	pthread_cond_signal(&ring->buffer_not_empty);
}


static void processEPOLLOUT(const int efd, const int fd)
{
	return;
}


void *processPackets (void *context)
{
	struct thread_info *info = (struct thread_info *)context;
	cpu_set_t mask;
	int fd, efd, type;
	int thread = info->index;

	CPU_ZERO(&mask);
	CPU_SET(thread+BASIS_CPU, &mask);
	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
		perror("pthread_setaffinity_np");
	}

#ifdef DATABASE
	clusterInfo *cluster = connectRedis(conf->database_ip, conf->database_port);
#endif
	struct timeval db_start, db_end;
	struct timeval cipher_start, cipher_end;
	unsigned int process_delay;
	struct ring_buffer *ring = packets_for_thread[thread];
	while (1) {
		if (ring_buffer_used(ring) <= 0) {
			pthread_cond_wait(&ring->buffer_not_empty, &ring->mlock);
		}
		struct epoll_packet *packet = ring_buffer_tail(ring);
		struct packet_format payload;
#ifdef CIPHER
		unsigned char buf_plain[PAYLOAD_SIZE];
		memcpy(buf_plain, packet->buf, PAYLOAD_OFFSET);
		gettimeofday(&cipher_start, NULL);
		decrypt_payload(buf_plain+PAYLOAD_OFFSET, buf_plain + ZERO_FILL_OFFSET, packet->buf+PAYLOAD_OFFSET);
		deserialize_payload(buf_plain, &payload);
		nop(conf->delay_in_ms * MS_TO_US);
		gettimeofday(&cipher_end, NULL);
		updateCipherDelay(getTimeDiff(cipher_start, cipher_end));
#else
		deserialize_payload(packet->buf, &payload);
#endif
		fd = packet->fd;
		efd = packet->efd;
		ring_buffer_pop(ring);
		pthread_mutex_unlock(&ring->mlock);	
		pthread_cond_signal(&ring->buffer_not_full);
		type = getPktType(&payload);
#ifdef DATABASE
		gettimeofday(&db_start, NULL);
		processDatabase(&payload, cluster);
		gettimeofday(&db_end, NULL);
		updateDBDelay(getTimeDiff(db_start, db_end));
#endif
		if (type ==  REQUEST_PKT) {
			increRequestPkts();
			setDirection(&payload, SERVER_TO_APP);
			processRequestPkt(efd, fd, &payload);
		}
		gettimeofday(&packet->done, NULL);
		process_delay = getTimeDiff(packet->recv, packet->done);
	}
#ifdef DATABASE
	disconnectDatabase(cluster);
#endif
}


void *server (void *context)
{
	int sfd, efd;
	int port;
	char port_string[10];
	struct epoll_event *events;
	cpu_set_t mask;
	struct thread_epoll_info *info = (struct thread_epoll_info *)context;
	sprintf(port_string, "%d", info->port);

	CPU_ZERO(&mask);
	CPU_SET(info->cpu, &mask);
	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
		perror("pthread_setaffinity_np");
	}

	sfd = create_and_bind(port_string);
	if (sfd == -1) {
		exit(1);
	}
	setServerSocket(sfd);				
	if ((listen (sfd, SOMAXCONN)) == -1 ) {
		exit(1);
	}
	efd = epoll_create1 (0);
	if (efd == -1) {
		perror ("epoll_create");
		exit(1);
	}
	addEpollEvent(efd, sfd, EPOLLIN);
	events = calloc (MAXEVENTS, sizeof(struct epoll_event));
	//signal(SIGINT, processSignal);
	while (1) {
		int i, n;
		n = epoll_wait (efd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i++) {
			if ((events[i].events & EPOLLERR) ||
				(events[i].events & EPOLLHUP) || 
				(!(events[i].events & EPOLLIN))) {
					deleteEpollEvent(efd, events[i].data.fd);
					continue;
			} else if (sfd == events[i].data.fd) {
					acceptEpollEvents(efd, sfd);
			} else if ( events[i].events & EPOLLIN ) {
					processEPOLLIN(efd, events[i].data.fd);
			} else if ( events[i].events & EPOLLOUT) {
					processEPOLLOUT(efd, events[i].data.fd);
			}
		}
	}
	free (events);
}

static void initThreads(pthread_t threads[], const int threads_num)
{
	assert (threads_num >= MIN_THREADS);
	int thread, rc;
	pthread_attr_t attr;
	struct thread_info info[MAX_THREADS];
	pthread_attr_init(&attr);

	for (thread=0; thread<threads_num; thread++) {
		packets_for_thread[thread] = malloc(sizeof(struct ring_buffer));	
		ring_buffer_init(packets_for_thread[thread]);
		info[thread].index = thread;
		rc = pthread_create(&threads[thread], &attr, processPackets, &info[thread]); 
		if (rc) {
			printf("ERROR.\n");
			exit(-1);
		}
	}
	sleep(1);
}


void printServerStarted()
{
    char IP[NI_MAXHOST];
	memset(IP, 0, sizeof(IP));
	if (getLocalIP(conf->dev, IP) == SUCCESS) {
		fprintf (stderr, "Server %s has been started.\n", IP);
	} else {
		fprintf (stderr, "Fail to parse IP for this server.\n");
	}
}


int main (int argc, char *argv[])
{
	struct thread_epoll_info info[MAXPORTS];
	pthread_t server_threads[MAXPORTS], packets_threads[MAX_THREADS];
	pthread_t throughput_min_thread, throughput_sec_thread, update_thread, admin_thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	
	conf = malloc(sizeof(struct configs));
	int rc;
#ifdef CIPHER
	initKeyTable();
#endif
	initConfigs(conf);
	getConfigs(conf);
	initThreads(packets_threads, conf->threads_num);

	int ports_num, port;
	ports_num = conf->ports_count;
	// The configuration that servers group just has single IP and single port is the best reasonable setting.
	for (port=0; port<ports_num; port++) {
		info[port].port = conf->ports[port];
		info[port].cpu  = port + EPOLL_CPU; 
		rc = pthread_create(&server_threads[port], &attr, server, &info[port]);
		if (rc) {
			printf("ERROR.\n");
			exit(-1);
		}
	}
#ifdef TPUT	
	rc = pthread_create(&throughput_sec_thread, &attr, throughput_sec, NULL);
    if (rc) {
		printf("ERROR.\n");
		exit(-1);
	}
	rc = pthread_create(&throughput_min_thread, &attr, throughput_min, NULL);
    if (rc) {
		printf("ERROR.\n");
		exit(-1);
	}
#endif
	rc = pthread_create(&update_thread, &attr, update_cfg, NULL);
    if (rc) {
		printf("ERROR.\n");
		exit(-1);
	}

	printServerStarted();

	pthread_exit(NULL);
	free(conf);
	closeRedis();
}
