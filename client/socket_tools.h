#ifndef SOCKET_TOOLS_H
#define SOCKET_TOOLS_H

#include <stdint.h>
#include <fcntl.h>
#include <error.h>


unsigned char * serialize_u64(unsigned char *buffer, uint64_t value);
unsigned char * serialize_u32(unsigned char *buffer, uint32_t value);
unsigned char * serialize_u16(unsigned char *buffer, uint16_t value);
unsigned char * serialize_u8(unsigned char *buffer, uint8_t value);

uint8_t deserialize_u8(unsigned char *buffer);
uint16_t deserialize_u16(unsigned char *buffer);
uint32_t deserialize_u32(unsigned char *buffer);
uint64_t deserialize_u64(unsigned char *buffer);

int make_socket_nonblocking(const int sfd);
#endif
