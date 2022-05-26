#include <Arduino.h>
#include "CANHandler.h"
#include "UDS.h"
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <LittleFS.h>
#include "myServer.h"
#include "myUpdateHandler.h"
#include <string>

char caracter = 0x00u;

void setup()
{

  Serial.begin(921600);

  initWifi();

  //Start LittleFS
  if(!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  Serial.println("1 - Initialize CAN driver.");
  Serial.println("2 - Initialize WIFI connection.");
  Serial.println("3 - Send UDS commands.");
  Serial.println("4 - Display stored firmware info.");
  Serial.println("5 - Reflash Cluster with stored Firmware.");
  /*
  if(!LittleFS.format())
  {
    Serial.println("An Error has occurred while formating LittleFS");
    return;
  }
  */

}

void loop()
{

  if(getShouldReboot())
  {
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }

  if(Serial.available())
  {
    caracter = Serial.read();
  }

  if('1' == caracter)
  {
    deinitWifi();
    initCAN();
  }
  else if('2' == caracter)
  {
    deinitCAN();
    initWifi();
  }
  else if('3' == caracter)
  {
    char value = 0x00u;
    char dlc = 0x00u;
    uint8_t index = 0x00u;
    uint8_t data[MAX_DLC] = {0,0,0,0,0,0,0,0};

    Serial.println("To exit press '~' ");
    while('~' != dlc)
    {
       //Read and process UDS commands
      if(Serial.available())
      {
        dlc = Serial.read();
        if('~' == dlc)
        {
          break;
        }
        else
        {
          //dlc -= 0x30;
          data[index] = dlc;
          index ++;
          Serial.print("dlc: ");
          Serial.printf("%x\n\r", dlc);
          while(index <= dlc)
          {
            if(Serial.available())
            {
              value = Serial.read();
              //value -= 0x30;
              data[index] = value;
              Serial.print("value: ");
              Serial.printf("%x\n\r", value);
              index ++;
            }
          }
          canSendUDS(data);
          delay(500);
          readCAN();
          //flush data
          for(index = 0; index < MAX_DLC; index++)
          {
            data[index] = 0x00u;
          }
          index = 0x00;
        }
      }
      if(index == MAX_DLC)
      {
        index = 0x00u;
      }
    }
    Serial.println("Exited UDS command menu!");
  }
  else if ('4' == caracter)
  {
    File f = LittleFS.open("/SavedFile.txt", "r");
    Serial.print(f.fullName());
    Serial.print(": ");
    Serial.println(f.available());
    f.close(); 
  }
  else if('?' == caracter)
  {
    Serial.println("1 - Initialize CAN driver.");
    Serial.println("2 - Initialize WIFI connection.");
    Serial.println("3 - Send UDS commands.");
    Serial.println("4 - Display stored firmware info.");
    Serial.println("5 - Reflash Cluster with stored Firmware.");
  }

  updaterTask();
  canTask(caracter);
  caracter = 0x00u;
}



