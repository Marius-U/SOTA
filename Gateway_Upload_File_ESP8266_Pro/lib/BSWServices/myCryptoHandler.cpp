#include "myCryptoHandler.h"

void flushEncryptBuffer(void);
void flushDataBuffer(void);

const uint8_t key[16] = {(uint8_t)0x00, (uint8_t)0x01, (uint8_t)0x02, (uint8_t)0x03,
                         (uint8_t)0x04, (uint8_t)0x05, (uint8_t)0x06, (uint8_t)0x07,
                         (uint8_t)0x08, (uint8_t)0x09, (uint8_t)0x0A, (uint8_t)0x0B,
                         (uint8_t)0x0C, (uint8_t)0x0D, (uint8_t)0x0E, (uint8_t)0x0F };

uint8_t data[16] =      {0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
                         0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u};

uint8_t decryptBuffer[16];
uint8_t encryptBuffer[16];
uint8_t securityAccessSeed[SECURITY_ACCESS_SEED_SIZE];
uint8_t securityAccessKey[SECURITY_ACCESS_KEY_SIZE];
AES128 myAES128;

void encrypt(BlockCipher* cipher, uint8_t* plainData)
{
    crypto_feed_watchdog();

    cipher->setKey(key, cipher->keySize());

    cipher->encryptBlock(encryptBuffer, plainData);
}

void decrypt(BlockCipher* cipher, uint8_t* encryptedData)
{
    crypto_feed_watchdog();

    cipher->setKey(key, cipher->keySize());

    cipher->decryptBlock(decryptBuffer, encryptedData);
}

void decryptFromFS(void)
{
    
    File crytpedBin = LittleFS.open("/SavedCryptedFile.txt", "r");
    int fileSize = 0x00u;
    int count = 0x00u;

    Serial.println("Start decrypting the downloaded firmware!");
    if(crytpedBin)
    {
        fileSize = crytpedBin.size();

        if(LittleFS.exists("/SavedFile.txt"))
        {
            //Remove old file if exists.
            LittleFS.remove("/SavedFile.txt");
        }
        //Open the plain file file 
        File file = LittleFS.open("/SavedFile.txt", "w+");

        while(count < fileSize)
        {
            if((count%1600) == 0)
            {
               Serial.print(".");
            }

            //read 16 bytes from crypted Firmware
            crytpedBin.read(data, 16*sizeof(uint8_t));

            //decrypt data and place it in the buffer
            decrypt(&myAES128, data);
            
            //write the decrypted 16bytes to the plain Firmware
            file.write(decryptBuffer, 16*sizeof(uint8_t));

            count+= 16;
        }
        Serial.println("");
        Serial.printf("Done decrypting %uB.", count);
        file.close();
        crytpedBin.close();
        // when finished remove the crypted binary from memory to indicate end of the process
        LittleFS.remove("/SavedCryptedFile.txt");
    }
    else 
    {
        Serial.println("Could not load SavedCryptedFile.bin!");
    }

}

void setSeed(uint8_t* seed)
{
    for(uint8_t index = 0; index < SECURITY_ACCESS_SEED_SIZE; index++)
    {
        securityAccessSeed[index] = seed[index];
    }
    
}

uint8_t getKey(uint8_t* key)
{
    //flush encryptBuffer first !!!
    flushEncryptBuffer();

    //flush data first !!!
    flushDataBuffer();

    for(uint8_t index = 0; index < SECURITY_ACCESS_SEED_SIZE; index++)
    {
        data[index] = securityAccessSeed[index];
    }

    encrypt(&myAES128, data);

    for(uint8_t index = 0; index < SECURITY_ACCESS_KEY_SIZE; index++)
    {
        securityAccessKey[index] = encryptBuffer[index];
        key[index] = encryptBuffer[index];
    }

    return OK;
}

uint8_t* getEncryptedBuffer(void)
{
    return encryptBuffer;
}


void flushEncryptBuffer(void)
{
    for(uint8_t index = 0x00u; index < 16u; index++)
    {
        encryptBuffer[index] = 0x00u;
    }
}

void flushDataBuffer(void)
{
    for(uint8_t index = 0x00u; index < 16u; index++)
    {
        data[index] = 0x00u;
    }
}