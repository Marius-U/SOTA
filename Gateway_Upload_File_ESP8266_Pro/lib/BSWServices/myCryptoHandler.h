#ifndef CRYPTOHANDLER_HEADER
#define CRYPTOHANDLER_HEADER

#include <Arduino.h>
#include <Crypto.h>
#include <AES.h>
#include <string.h>
#include <LittleFS.h>
#include <fstream>

#define SECURITY_ACCESS_SEED_SIZE 0x02u
#define SECURITY_ACCESS_KEY_SIZE   0x04u

void encrypt(BlockCipher* cipher, uint8_t* plainData);
void decrypt(BlockCipher* cipher, uint8_t* encryptedData);
void decryptFromFS(void);
void setSeed(uint8_t* seed);
uint8_t getKey(uint8_t* key);


#endif