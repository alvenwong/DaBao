#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdio.h>
#include <stdint.h>


#define PAYLOAD_SIZE 146
#define MTU			 200
#define RTT_MAX		 1000000

struct statistics {
    uint32_t send_counter_sec;
    uint32_t send_counter_min;
    uint32_t request_counter_sec;
    uint32_t request_counter_min;
    uint32_t receive_counter_sec;
    uint32_t receive_counter_min;
    uint64_t throughput_sec;
    uint64_t RTT_sum;
	uint32_t RTT_max;
};

void *throughput_sec(void *context);
void *throughput_min(void *context);
void increSendPkts();
void increRequestPkts();
void increReceivePkts(const int RTT);

struct statistics stats;

#endif 
