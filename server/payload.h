#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <stdint.h>
#include <string.h>

#define MTU				200
#define PAYLOAD_SIZE	146
#define STATE_SIZE      67
#define PAYLOAD_OFFSET	22
#define ZERO_FILL_OFFSET 6


struct packet_format {
    uint8_t     zero[ZERO_FILL_OFFSET];
	uint8_t		direction;
	uint8_t		type;
	uint32_t	userID[3];
    uint16_t    reserve;
    uint8_t     type_affirm;
	uint64_t	family;
    uint32_t    srcDeviceID[4];
	uint32_t	dstDeviceID[4];
	uint32_t	operation[4];
	uint8_t		state[STATE_SIZE];
};


void initPayload(struct packet_format * payload);
void freePayload(struct packet_format * payload);
void setPayload(struct packet_format *payload, const unsigned char pkt_type, const unsigned char direction, const unsigned int ID);

#endif


