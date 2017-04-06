#include <stdio.h>
#include <stdlib.h>

#include "payload.h"

void initPayload(struct packet_format * payload)
{
    memset(payload, 0, sizeof(struct packet_format));
    memset(payload->state, 'a', STATE_SIZE*sizeof(char));
}


void freePayload(struct packet_format *payload)
{
	free(payload);
}


void setPayload(struct packet_format *payload, const unsigned char pkt_type, const unsigned char direction, const unsigned int ID)
{
	payload->direction	= direction;
	payload->type		= pkt_type;
    payload->type_affirm= pkt_type;
	payload->userID[0]	= ID;
	payload->srcDeviceID[0]= ID;
	payload->dstDeviceID[0]= ID;
}
