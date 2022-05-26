#include "myTestHandler.h"

typedef unsigned char byte;
typedef unsigned long uint64;

struct TestVector
{
    const char* name;
    byte key[32];
    byte plaintext[16];
    byte ciphertext[16];
};
struct timeval tp;

uint64 micros()
{
    LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
    LARGE_INTEGER Frequency;

    QueryPerformanceFrequency(&Frequency);
    QueryPerformanceCounter(&StartingTime);

    // Activity to be timed

    QueryPerformanceCounter(&EndingTime);

    ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;


    //
    // We now have the elapsed number of ticks, along with the
    // number of ticks-per-second. We use these values
    // to convert to the number of elapsed microseconds.
    // To guard against loss-of-precision, we convert
    // to microseconds *before* dividing by ticks-per-second.
    //

    ElapsedMicroseconds.QuadPart *= 1000000;
    ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

    return (uint64)ElapsedMicroseconds.QuadPart;
}

// Define the ECB test vectors from the FIPS specification.
static TestVector const testVectorAES128 = {
    .name = "AES-128-ECB",
    .key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    .plaintext = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
    .ciphertext = {0x69, 0xC4, 0xE0, 0xD8, 0x6A, 0x7B, 0x04, 0x30,
                    0xD8, 0xCD, 0xB7, 0x80, 0x70, 0xB4, 0xC5, 0x5A}
};
static TestVector const testVectorAES192 = {
    .name = "AES-192-ECB",
    .key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17},
    .plaintext = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
    .ciphertext = {0xDD, 0xA9, 0x7C, 0xA4, 0x86, 0x4C, 0xDF, 0xE0,
                    0x6E, 0xAF, 0x70, 0xA0, 0xEC, 0x0D, 0x71, 0x91}
};
static TestVector const testVectorAES256 = {
    .name = "AES-256-ECB",
    .key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    .plaintext = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
    .ciphertext = {0x8E, 0xA2, 0xB7, 0xCA, 0x51, 0x67, 0x45, 0xBF,
                    0xEA, 0xFC, 0x49, 0x90, 0x4B, 0x49, 0x60, 0x89}
};

AES128 aes128;
AES192 aes192;
AES256 aes256;

byte buffer[16];

void testCipher(BlockCipher* cipher, const struct TestVector* test)
{
    crypto_feed_watchdog();
    std::cout << test->name;
    std::cout << " Encryption ... ";
    cipher->setKey(test->key, cipher->keySize());
    cipher->encryptBlock(buffer, test->plaintext);
    if (memcmp(buffer, test->ciphertext, 16) == 0)
        std::cout << "Passed\n";
    else
        std::cout << "Failed\n";

    std::cout << test->name;
    std::cout << " Decryption ... ";
    cipher->decryptBlock(buffer, test->ciphertext);
    if (memcmp(buffer, test->plaintext, 16) == 0)
        std::cout << "Passed\n";
    else
        std::cout << "Failed\n";
}

void perfCipher(BlockCipher* cipher, const struct TestVector* test)
{
    unsigned long start;
    unsigned long elapsed;
    int count;

    crypto_feed_watchdog();

    std::cout << test->name;
    std::cout << " Set Key ... ";
    start = micros();

    for (count = 0; count < 10000; ++count) {
        cipher->setKey(test->key, cipher->keySize());
    }
    elapsed = micros() - start;
    std::cout << elapsed / 10000.0;
    std::cout << "us per operation, ";
    std::cout << (10000.0 * 1000000.0) / elapsed;
    std::cout << " per second\n";

    std::cout << test->name;
    std::cout << " Encrypt ... ";
    start = micros();
    for (count = 0; count < 5000; ++count)
    {
        cipher->encryptBlock(buffer, buffer);
    }
    elapsed = micros() - start;
    std::cout << elapsed / (5000.0 * 16.0);
    std::cout << "us per byte, ";
    std::cout << (16.0 * 5000.0 * 1000000.0) / elapsed;
    std::cout << " bytes per second\n";

    std::cout << test->name;
    std::cout << " Decrypt ... ";
    start = micros();
    for (count = 0; count < 5000; ++count) {
        cipher->decryptBlock(buffer, buffer);
    }
    elapsed = micros() - start;
    std::cout << elapsed / (5000.0 * 16.0);
    std::cout << "us per byte, ";
    std::cout << (16.0 * 5000.0 * 1000000.0) / elapsed;
    std::cout << " bytes per second\n";

    std::cout << "\n";
}

void test()
{
    std::cout << "\n";

    std::cout << "State Sizes:\n";
    std::cout << "AES128 ... ";
    std::cout << sizeof(AES128);
    std::cout << "\n";
    std::cout << "AES192 ... ";
    std::cout << sizeof(AES192);
    std::cout << "\n";
    std::cout << "AES256 ... ";
    std::cout << sizeof(AES256);
    std::cout << "\n";

    std::cout << "Test Vectors:\n";
    testCipher(&aes128, &testVectorAES128);
    testCipher(&aes192, &testVectorAES192);
    testCipher(&aes256, &testVectorAES256);

    std::cout << "\n";

    std::cout << "Performance Tests:\n";
    perfCipher(&aes128, &testVectorAES128);
    perfCipher(&aes192, &testVectorAES192);
    perfCipher(&aes256, &testVectorAES256);
}