#include "statistics.h"


void resetSecStats()
{
    stats.send_counter_sec = 0;
    stats.request_counter_sec = 0;
    stats.receive_counter_sec = 0;
    stats.RTT_sum = 0;
    stats.throughput_sec = 0;
	stats.RTT_max = 0;
}

void resetMinStats()
{
    stats.send_counter_min = 0;
    stats.request_counter_min = 0;
    stats.receive_counter_min = 0;
}


void *throughput_sec(void *context)
{
	while (1) {
		sleep(1);
		printf("packets: %u, requests: %d, receives: %d, rev tput: %d Mbps", 
                stats.send_counter_sec, 
                stats.request_counter_sec, 
                stats.receive_counter_sec, 
                stats.throughput_sec >> 17);
		if (stats.receive_counter_sec == 0) {
			printf("\n");
		} else {
			printf(", RTT: %d, max RTT: %d\n", 
					stats.RTT_sum/stats.receive_counter_sec,
					stats.RTT_max);
		}
        resetSecStats();
	}
}


void *throughput_min(void *context)
{
	while (1) {
		sleep(60);
		printf("**************************\n");
		printf("send pkts: %d/min, request pkts: %d/min, recv pkts: %d/min\n", 
                stats.send_counter_min, 
                stats.request_counter_min,
                stats.receive_counter_min);
		printf("**************************\n");
        resetMinStats();
	}
}


void increSendPkts()
{
    stats.send_counter_sec += 1;
    stats.send_counter_min += 1;
    stats.throughput_sec += MTU;
}


void increRequestPkts()
{
    stats.request_counter_sec += 1;
    stats.request_counter_min += 1;
}


void increReceivePkts(const int RTT)
{
	if (RTT < RTT_MAX) {
    	stats.receive_counter_sec += 1;
    	stats.receive_counter_min += 1;
    	stats.RTT_sum += RTT;
		if (RTT > stats.RTT_max) {
			stats.RTT_max = RTT;
		}
	}
}
