#ifndef UPDATEHANDLER_HEADER
#define UPDATEHANDLER_HEADER

#include <Arduino.h>
#include <LITTLEFS.h>
#include <Update.h>

#define FORMAT_LITTLEFS_IF_FAILED true

void initUpdateHandler(void);
void writeDataToFile(size_t index, uint8_t *data, size_t len, bool final);
void updateFromFS(void);
void performUpdate(Stream &updateSource, size_t updateSize);

#endif