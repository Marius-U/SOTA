// AES_GatewayFirmwareEncrypt.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string.h>
#include "Crypto/myTestHandler.h"
#include <filesystem>

using namespace std;
using namespace std::filesystem;

void encrypt(BlockCipher* cipher, uint8_t* plainData);
void decrypt(BlockCipher* cipher, uint8_t* encryptedData);

ifstream plainFirmwareFile;
fstream cypheredFirmwareFile;

const uint8_t key[16] = {(uint8_t)0x00, (uint8_t)0x01, (uint8_t)0x02, (uint8_t)0x03,
                         (uint8_t)0x04, (uint8_t)0x05, (uint8_t)0x06, (uint8_t)0x07,
                         (uint8_t)0x08, (uint8_t)0x09, (uint8_t)0x0A, (uint8_t)0x0B,
                         (uint8_t)0x0C, (uint8_t)0x0D, (uint8_t)0x0E, (uint8_t)0x0F };
uint8_t encryptBuffer[16];
AES128 myAES128;


int main(int argc, char* argv[], char* envp[])
{
    //Create the PATH object from the first passed argument
    path pathToFirmware(argv[1]);

    string inPath = pathToFirmware.root_path().string().append(pathToFirmware.relative_path().string());
    string outPath;
    std::streampos fileSize;

    if (!strcmp(argv[2], "1"))
    {
        //decrypt data and pace it in the buffer
        cout << "Decrypt!" << endl;
        outPath = pathToFirmware.parent_path().string().append("\\plain_").append(pathToFirmware.filename().string());
    }
    else
    {
        //encrypt data and pace it in the buffer
        cout << "Encrypt!" << endl;
        outPath = pathToFirmware.parent_path().string().append("\\cyphered_").append(pathToFirmware.filename().string());
    }


    unsigned char data[16] = {0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
                              0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u};

    plainFirmwareFile.open(inPath, ios::in| ios::binary);
    cypheredFirmwareFile.open(outPath, ios::out| ios::binary);


    if (plainFirmwareFile)
    {
        
        plainFirmwareFile.seekg(0, std::ios::end);
        fileSize = plainFirmwareFile.tellg();
        cout << (int)fileSize;
        int count = 0x00u;

        plainFirmwareFile.seekg(ios::beg);

        if (cypheredFirmwareFile)
        {
            cypheredFirmwareFile.seekp(ios::beg);

            while(count < (int)fileSize) //(!plainFirmwareFile.eof())
            {
                //read 16 bytes from plain Firmware
                plainFirmwareFile.read(reinterpret_cast<char*>(data), 16 * sizeof(unsigned char));

                if (!strcmp(argv[2],"1"))
                {
                    //decrypt data and place it in the buffer
                    decrypt(&myAES128, data);
                }
                else
                {
                    //encrypt data and place it in the buffer
                    encrypt(&myAES128, data);
                }

                //write the encrypted 16bytes to the cyphered Firmware
                cypheredFirmwareFile.write(reinterpret_cast<char*>(encryptBuffer), 16 * sizeof(unsigned char));

                count += 16;
            }

            //close output file when done
            cypheredFirmwareFile.close();
        }
        else
        {
            cout << "Out file could not be opened!" << endl;
        }

        //close input file when done
        plainFirmwareFile.close();
    }
    else
    {
        cout << "Input file could not be opened!" << endl;
    }   

   // system("pause");
}

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

    cipher->decryptBlock(encryptBuffer, encryptedData);
}

/*
* with this function you can easily load your file into the vector like this:
* std::vector<BYTE> fileData = readfile("myfile.bin");
*/

std::vector<BYTE> readFile(const char* filename)
{
    // open the file:
    std::streampos fileSize;
    std::ifstream file(filename, std::ios::binary);

    // get its size:
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // read the data:
    std::vector<BYTE> fileData(fileSize);
    file.read((char*)&fileData[0], fileSize);
    return fileData;
}

