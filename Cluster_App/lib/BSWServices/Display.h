#ifndef _DISPLAY_H_INCLUDE
#define _DISPLAY_H_INCLUDE

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>

 #define TFT_GREY 0x5AEB
 
extern TFT_eSPI tft;

void Display_Init(TFT_eSPI &tft);
void DisplayWrite(TFT_eSPI &tft, const char *ifsh);
void DisplayWrite(TFT_eSPI &tft, long value,int base);
void DisplayWrite(TFT_eSPI &tft, int value);
void DisplayPrint(TFT_eSPI &tft, long value, int base);
void DisplayPrint(TFT_eSPI &tft, int value, int base);
void DisplayPrint(TFT_eSPI &tft, int value);
void DisplayPrint(TFT_eSPI &tft, char value);
void DisplayPrint(TFT_eSPI &tft, const char* value);

#endif