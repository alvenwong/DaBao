#include "serialize.h"


unsigned char * serialize_payload(unsigned char *buffer, struct packet_format *info)
{
	int i;
	for (i=0; i<ZERO_FILL_OFFSET; i++) {
		buffer = serialize_u8(buffer, info->zero[i]);
	}
    buffer = serialize_u8(buffer, info->direction);
    buffer = serialize_u8(buffer, info->type);
	for (i=0; i<3; i++) {
		buffer = serialize_u32(buffer, info->userID[i]);
	}
    buffer = serialize_u16(buffer, info->reserve);
    buffer = serialize_u8(buffer, info->type_affirm);
    buffer = serialize_u64(buffer, info->family);
	for (i=0; i<4; i++) {
		buffer = serialize_u32(buffer, info->srcDeviceID[i]);
	}
	for (i=0; i<4; i++) {
		buffer = serialize_u32(buffer, info->dstDeviceID[i]);
	}
	for (i=0; i<4; i++) {
		buffer = serialize_u32(buffer, info->operation[i]);
	}
	for (i=0; i<STATE_SIZE; i++) {
		buffer = serialize_u8(buffer, info->state[i]);
	}
    return buffer;
}


void deserialize_payload(unsigned char *buffer, struct packet_format *info)
{
	int i;
	unsigned char * tmp_buf = buffer;
	for (i=0; i<ZERO_FILL_OFFSET; i++) {
		info->zero[i] = deserialize_u8(tmp_buf);
		tmp_buf += 1;
	}
    info->direction = deserialize_u8(tmp_buf);
	tmp_buf += 1;
    info->type		= deserialize_u8(tmp_buf);
	tmp_buf += 1;
	for (i=0; i<3; i++) {
		info->userID[i] = deserialize_u32(tmp_buf);
		tmp_buf += 4;
	}
    info->reserve = deserialize_u16(tmp_buf);
    tmp_buf += 2;
    info->type_affirm	= deserialize_u8(tmp_buf);
	tmp_buf += 1;
    info->family    = deserialize_u64(tmp_buf);
	tmp_buf += 8;
	for (i=0; i<4; i++) {
		info->srcDeviceID[i] = deserialize_u32(tmp_buf);
		tmp_buf += 4;
	}
	for (i=0; i<4; i++) {
		info->srcDeviceID[i] = deserialize_u32(tmp_buf);
		tmp_buf += 4;
	}
	for (i=0; i<4; i++) {
		info->operation[i] = deserialize_u32(tmp_buf);
		tmp_buf += 4;
	}
	for (i=0; i<STATE_SIZE; i++) {
		info->state[i] = deserialize_u8(tmp_buf);
		tmp_buf += 1;
	}
}

unsigned char * serialize_init_configs(unsigned char *buffer,  struct client_init_configs *info)
{
	int i;
	buffer = serialize_u32(buffer, info->epoch);
	buffer = serialize_u32(buffer, info->connections);
	buffer = serialize_u32(buffer, info->requestRate);
	buffer = serialize_u32(buffer, info->burst);
	buffer = serialize_u32(buffer, info->vburst);
	for (i=0; i<IPLEN; i++) {
		buffer = serialize_u8(buffer, info->server_ip[i]);
	}
	buffer = serialize_u32(buffer, info->server_port);

	return buffer;
}


unsigned char * serialize_update_configs(unsigned char *buffer, struct client_update_configs *info)
{
	buffer = serialize_u32(buffer, info->epoch);
	buffer = serialize_u32(buffer, info->connections);
	buffer = serialize_u32(buffer, info->requestRate);
	buffer = serialize_u32(buffer, info->burst);

    return buffer;
}


unsigned char * deserialize_init_configs(unsigned char *buffer, struct client_init_configs *info) 
{
	int i;
	unsigned char * tmp_buf = buffer;
	info->epoch = deserialize_u32(tmp_buf);
	tmp_buf += 4;
	info->connections = deserialize_u32(tmp_buf);
	tmp_buf += 4;
	info->requestRate = deserialize_u32(tmp_buf);
	tmp_buf += 4;
	info->burst = deserialize_u32(tmp_buf);
	tmp_buf += 4;
	info->vburst = deserialize_u32(tmp_buf);
	tmp_buf += 4;
	for (i=0; i<IPLEN; i++) {
		info->server_ip[i] = deserialize_u8(tmp_buf);
		tmp_buf += 1;
	}
	info->server_port = deserialize_u32(tmp_buf);
	tmp_buf += 4;
}


unsigned char * deserialize_update_configs(unsigned char *buffer, struct client_update_configs * info)
{
	unsigned char * tmp_buf = buffer;
	info->epoch = deserialize_u32(tmp_buf);
	tmp_buf += 4;
	info->connections = deserialize_u32(tmp_buf);
	tmp_buf += 4;
	info->requestRate = deserialize_u32(tmp_buf);
	tmp_buf += 4;
	info->burst = deserialize_u32(tmp_buf);
	tmp_buf += 4;
}
