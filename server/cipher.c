#include "cipher.h"


int __binary(uint16_t value, const int print)
{
	uint16_t tmp = value;
	uint16_t cmp = 0x8000;
	int i, counter;
	PRINT(print, "%d:", value);
	counter = 0;
	for (i=0; i<16; i++) {
		if (i % 4 == 0) {
			PRINT(print, " ");
		}
		if ((uint16_t)(tmp & cmp) == 0) {
			PRINT(print, "0");
		} else {
			counter += 1;
			PRINT(print, "1");
		}
		cmp = cmp >> 1;
	}
	if (print == 1) {
		PRINT(print, "\n");
	}
	return counter;
}


int getCounter(uint16_t value) 
{
	return __binary(value, 0);
}


int showBinary(uint16_t value)
{
	return __binary(value, 1);
}


void uint16ToChar(const uint16_t value, uint8_t *ch)
{
	*ch = value;
	*(ch + 1) = value >> 8;
}


uint16_t charToUint16(uint8_t *ch)
{
	uint16_t value = 0x0000;
	value = (value | *(ch + 1) << 8) | *(ch);
	return value;
}


uint16_t __encipher(uint16_t plain, uint16_t key, uint8_t *cipher)
{
	int cnt;
	uint16_t tmp;

	cnt = getCounter(key);
	tmp = ROTATE_LEFT(plain, 16, cnt);
	tmp ^= key;
	uint16ToChar(tmp, cipher);
	return tmp;
}


void encipher(uint8_t *plaintext, uint8_t *key, uint8_t *ciphertext)
{
	assert(strlen(plaintext) <= SIZE);
	assert(strlen(key) <= SIZE);
	assert(strlen(ciphertext) <= SIZE);
	uint16_t cipherKey = INITKEY;
	uint16_t plainPart, keyPart;
	int i;
	uint16_t tmp;
	for (i=0; i<GROUPS; i++) {
		plainPart = charToUint16(plaintext + i*GROUP_SIZE);
		keyPart = charToUint16(key + i*GROUP_SIZE);
		tmp = plainPart ^ cipherKey;
		cipherKey = __encipher(tmp, keyPart, ciphertext + i*GROUP_SIZE);
	}
}


void decipher(uint8_t *plaintext, uint8_t *key, uint8_t *ciphertext)
{
	assert(strlen(plaintext) <= SIZE);
	assert(strlen(key) <= SIZE);
	assert(strlen(ciphertext) <= SIZE);
	uint16_t cipherKey = INITKEY;
	uint16_t cipherPart, keyPart, tmpCipherPart;
	int i, cnt;
	uint16_t tmp;
	for (i=0; i<GROUPS; i++) {
		cipherPart = charToUint16(ciphertext + i*GROUP_SIZE);
		tmpCipherPart = cipherPart;
		keyPart = charToUint16(key + i*GROUP_SIZE);
		tmp = cipherPart ^ keyPart;
		cnt = getCounter(keyPart);
		tmp = ROTATE_RIGHT(tmp, 16, cnt);
		tmp ^= cipherKey;
		uint16ToChar(tmp, plaintext + i*GROUP_SIZE);
		cipherKey = cipherPart; 
	}
	plaintext[SIZE] = '\0';
}


int main(int args, char *argv[])
{
	if (args != 3) {
		printf("%s <plain> <key>\n", argv[0]);
		return 0;
	}
	uint8_t plaintext[BUFSIZE];
	uint8_t ciphertext[BUFSIZE];
	uint8_t solution[BUFSIZE];
	uint8_t key[BUFSIZE]; 

	memset(plaintext, 0, sizeof(plaintext));
	strcpy(plaintext, argv[1]);
	assert(strlen(plaintext) <= SIZE);
	memset(key, 0, sizeof(key));
	strcpy(key, argv[2]);
	assert(strlen(key) <= SIZE);
	encipher(plaintext, key, ciphertext);
	decipher(solution, key, ciphertext);

	printf("plaintext: %s\n", plaintext);
	printf("solution: %s\n", solution);
	assert(strcmp(plaintext, solution) == 0);

	return 0;
}
