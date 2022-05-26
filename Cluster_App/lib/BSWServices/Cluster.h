#ifndef _CLUSTER_H_INCLUDED
#define _CLUSTER_H_INCLUDED


#include <Arduino.h>

// Define meter size
#define M_SIZE 1
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>


/**************Macro definitions*********/
#define TFT_GREY 0x5AEB
// #define TFT_ORANGE      0xFD20      /* 255, 165,   0 */


/***************Public Variables*********/


/***************Public Functions*********/
uint8_t Cluster_Init(TFT_eSPI &passedTft);
void Cluster_Main(TFT_eSPI &passedTft);
void analogMeter(void);
void plotNeedle(int value, byte ms_delay);
void Cluster_DeInit(void);

#endif //multiple inclusion guard