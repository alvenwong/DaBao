#include "statistics.h"


void resetSecStats()
{
    stats.send_counter_sec = 0;
    stats.request_counter_sec = 0;
    stats.receive_counter_sec = 0;
    stats.throughput_sec = 0;
    stats.DB_delay = 0;
    stats.cipher_delay = 0;
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
		printf("packets: %u, requests: %u, send: %u, rev tput: %d Mbps", 
                stats.receive_counter_sec, 
                stats.request_counter_sec, 
                stats.send_counter_sec, 
                stats.throughput_sec >> 17);
		if (stats.receive_counter_sec == 0) {
			printf("\n");
		} else {
			printf(", DB delay: %d, cipher delay: %d\n", 
					stats.DB_delay/stats.receive_counter_sec,
					stats.cipher_delay/stats.receive_counter_sec);
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


void increReceivePkts()
{
    stats.receive_counter_sec += 1;
    stats.receive_counter_min += 1;
}


void updateDBDelay(const int delay)
{
    stats.DB_delay += delay;
}


void updateCipherDelay(const int delay)
{
    stats.cipher_delay += delay;
}
