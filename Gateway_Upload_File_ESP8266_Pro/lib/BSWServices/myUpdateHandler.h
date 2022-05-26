#ifndef UPDATEHANDLER_HEADER
#define UPDATEHANDLER_HEADER

#include <Arduino.h>
#include <LittleFS.h>
#include "myServer.h"
#include "CANHandler.h"

void updateFromFS(void);
void performUpdate(Stream &updateSource, size_t updateSize);
void updaterTask();

#endif