#include <Arduino.h>
#include <CAN.h>
#include "Cluster.h"
#include "Display.h"
#include "UDS.h"
#include <Update.h>
#include "myUpdateHandler.h"


uint8_t rxData[8];
uint16_t receivedCANID = 0x0000u;
uint8_t packetSize;
uint8_t isDisplayInit = 0x00u;

void diagSesion(void);

void CAN_Main();

void setup(void) {
  Serial.begin(921600); // For debug
  
  initUpdateHandler();

  CAN.setPins(27,26);
  // start the CAN bus at 1000 kbps
  if (!CAN.begin(1000E3))
  {
    Serial.println("Starting CAN failed!");
    while (1);
  }
}


void loop() 
{
  CAN_Main();

  if(DEFAULT_SESSION == UDS_getActiveSession())
  {
    isDisplayInit = 0x00u;

    Cluster_Main(tft);
  }
  else if(PROGRAMMING_SESSION == UDS_getActiveSession())
  {
    diagSesion();
  }
  
}

void flushRxData()
{
  uint8_t index = 0x00u;
  
  for(index = 0x00u; index < MAX_DLC; index++)
  {
    rxData[index] = 0x00u;
  }

}

void diagSesion()
{

  if(!isDisplayInit)
  {
    Cluster_DeInit();
    Display_Init(tft);
    DisplayPrint(tft, "Diagnostic Session! ");
    isDisplayInit = 0x01u;
  }

  if(tft.getCursorY() > 270)
  {
    tft.setCursor(0,0,2);
    tft.fillScreen(TFT_GREY);
  }
}

void CAN_Main() 
{
  
  uint8_t index = 0x00u;

  // try to parse packet
  packetSize = CAN.parsePacket();

  if (packetSize)
  {
    receivedCANID = CAN.packetId();
  
    while (CAN.available()) 
    {
      rxData[index] = (uint8_t)CAN.read();
      index++; 
    }
    index = 0x00u;

    if(UDS_REQUEST_ID == receivedCANID)
    {
      UDS_setRxBuffer(rxData);
      UDS_process();
      flushRxData();
    }
  }
}
