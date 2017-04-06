#ifndef CIPHER_H
#define CIPHER_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define INITKEY		0x1234
#define SIZE		8
#define BUFSIZE		9
#define GROUPS		4
#define GROUP_SIZE	2

#define ROTATE_LEFT(x, s, n) ((x) << (n)) | ((x) >> ((s) - (n)))
#define ROTATE_RIGHT(x, s, n) ((x) >> (n)) | ((x) << ((s) - (n)))
#define PRINT(n, format, args...) (n) == (1) ? (printf(format, ##args)) : (printf(""))


void encipher(uint8_t *plaintext, uint8_t *key, uint8_t *ciphertext);
void decipher(uint8_t *plaintext, uint8_t *key, uint8_t *ciphertext);

#endif
