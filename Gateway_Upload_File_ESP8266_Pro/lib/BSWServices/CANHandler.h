#ifndef CANHANDLER_HEADER
#define CANHANDLER_HEADER

#include <Arduino.h>
#include <CAN.h> //SPI_HAS_NOTUSINGINTERRUPT was set and SPI is not able to use ISR onReceive
#include "UDS.h"
#include "myUpdateHandler.h"
#include "myCryptoHandler.h"

#define DEFAULT_STATE 0x00u

void flushRxData();
void canTask(char caracter);
void initCAN(void);
void deinitCAN(void);
bool getCANFinished();
void resetCANFinished();
void setNewImageReceived(bool value);
void canSendUDS(uint8 *data);
void readCAN(void);

#endif
