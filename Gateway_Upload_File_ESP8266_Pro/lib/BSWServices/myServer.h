#ifndef MYSERVER_HEADER
#define MYSERVER_HEADER

#include <Arduino.h>
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <LittleFS.h>



bool getShouldReboot();
bool getFileIsUploaded();
void setFileIsUploaded(bool state);
void setShouldReboot(bool value);
void setInfo(String s);
void initServer();
void initWifi(void);
void deinitWifi(void);

#endif

