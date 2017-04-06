#include "payload.h"

unsigned int *map;

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
	payload->type = pkt_type;
	payload->direction	= direction;
	payload->userID[0]	= ID;
	payload->userID[1]	= map[ID];
	payload->userID[2]	= ID;
	payload->type_affirm = pkt_type;
	payload->srcDeviceID[0]= map[ID];
	payload->dstDeviceID[0]= ID;
}


void initIDMap(const int index_start, const int index_end)
{
	int id, map_size;

	map_size = index_end - index_start;
	assert (map_size > 0);
	map = calloc(map_size, sizeof(unsigned int));
	
	for (id=0; id<map_size; id++) {
		map[id] = id + index_start;
	}

	shuffer(map, map_size);
}


void freeIDMap()
{
	free(map);
}
