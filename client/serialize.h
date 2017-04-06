#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "socket_tools.h"
#include "payload.h"
#include "protocol.h"

unsigned char * serialize_payload(unsigned char *buffer, struct packet_format *info);
void deserialize_payload(unsigned char *buffer, struct packet_format *info);
unsigned char * serialize_init_configs(unsigned char *buffer, struct client_init_configs *info);
unsigned char * serialize_update_configs(unsigned char *buffer, struct client_update_configs *info);
unsigned char * deserialize_init_configs(unsigned char *buffer, struct client_init_configs *info);
unsigned char * deserialize_update_configs(unsigned char *buffer, struct client_update_configs *info);

#endif
