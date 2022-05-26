#include "CANHandler.h"

uint8_t rxData[8];
uint8_t transferData[5];
uint8_t updateSMSatate = DEFAULT_STATE;
bool canFinished = false;
bool newImageReceived = false;
File udsFile;



void initCAN(void)
{
      // start the CAN bus at 500 kbps
  if (!CAN.begin(1000E3)) /* At 1000kBaud config we actualy get a 500kBaud config!!!*/
  {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  Serial.println("CAN initialized!");
}

void deinitCAN(void)
{
  CAN.end();
  Serial.println("CAN stoped!");
  delay(3000);
}

void flushRxData()
{
  uint8_t index = 0x00u;
  
  for(index = 0x00u; index < (uint8_t)(sizeof(rxData)/sizeof(rxData[0])); index++)
  {
    rxData[index] = 0x00u;
  }

}

bool getCANFinished()
{
  return canFinished;
}

void resetCANFinished()
{
  canFinished = false;
}

void setNewImageReceived(bool value)
{
  newImageReceived = value;
}

void updateSM(void)
{
  static uint64_t totalSize = 0x00u;
  switch (updateSMSatate)
  {
    case SECURRITY_ACCESS:
    {
      Serial.println("Request seed!");
      CAN.beginPacket(0x100);
      CAN.write(2);
      CAN.write(0x27u);
      CAN.write(0x01u);
      CAN.endPacket();

      delay(500);
      readCAN();
      
      Serial.println("Compute key!");
      uint8_t key[SECURITY_ACCESS_KEY_SIZE] = {0,0,0,0};
      getKey(key);

      Serial.printf("Send key: %x %x %x %x\n\r", key[0],key[1],key[2],key[3]);
      CAN.beginPacket(0x100);
      CAN.write(6);
      CAN.write(0x27u);
      CAN.write(0x02u);
      CAN.write(key[0]);
      CAN.write(key[1]);
      CAN.write(key[2]);
      CAN.write(key[3]);
      CAN.endPacket();

      delay(500);
      readCAN();
      if(updateSMSatate != REQUEST_DOWNALOAD)
      {
        updateSMSatate = DEFAULT_STATE;
      }
    }break;
    case REQUEST_DOWNALOAD:
    {
      CAN.beginPacket(0x100);
      CAN.write(1);
      CAN.write(0x34u);
      CAN.endPacket();
      updateSMSatate = TRANSFER_DATA;
    }
    break;
    case TRANSFER_DATA:
    {
      if(LittleFS.exists("/SavedFile.txt"))
      {
        //Open the file 
        udsFile = LittleFS.open("/SavedFile.txt", "r");
        if(udsFile.isDirectory())
        {
          Serial.println("Error, SavedFile.bin is not a file");
          udsFile.close();
          return;
        }
        size_t updateSize = udsFile.size();
        if (updateSize > 0) 
        {
          Serial.println("Start CAN transfer!");
          uint8_t counter = 0x00u;

          while(udsFile.streamRemaining())
          {
            int size = udsFile.read(transferData, 5);

            CAN.beginPacket(0x100);

            CAN.write(size);
            CAN.write(0x36u);
            CAN.write(counter);
            if(5 == size)
            {
              CAN.write(transferData[0]);
              CAN.write(transferData[1]);
              CAN.write(transferData[2]);
              CAN.write(transferData[3]);
              CAN.write(transferData[4]);
            }
            else if(4 == size)
            {
              CAN.write(transferData[0]);
              CAN.write(transferData[1]);
              CAN.write(transferData[2]);
              CAN.write(transferData[3]);
            }
            else if(3 == size)
            {
              CAN.write(transferData[0]);
              CAN.write(transferData[1]);
              CAN.write(transferData[2]);
            }
            else if(2 == size)
            {
              CAN.write(transferData[0]);
              CAN.write(transferData[1]);
            }
            else if(1 == size)
            {
              CAN.write(transferData[0]);
            }
            else
            {
              Serial.printf("Read size is not ok: %u\n\r", size);
            }
            CAN.endPacket();

#if 0 
            Serial.print(size);
            Serial.print(" ");
            Serial.print(0x36u);
            Serial.print(" ");
            Serial.print(counter);
            Serial.print(" ");
            Serial.print(transferData[0]);
            Serial.print(" ");
            Serial.print(transferData[1]);
            Serial.print(" ");
            Serial.print(transferData[2]);
            Serial.print(" ");
            Serial.print(transferData[3]);
            Serial.print(" ");
            Serial.print(transferData[4]);
            Serial.println("");
#endif

            totalSize += size;
            delay(15);
            
            counter ++;
            if(counter%100 == 0)
            {
              Serial.print(".");
            }

            //read the UDS response here
          }
          Serial.println("");
          Serial.println("Reached the end of the file!");       
        }
        else 
        {
          Serial.println("Error, file is empty");
        }

        udsFile.close();
      }
      updateSMSatate = REQUEST_TRANSFER_EXIT;
    }
    break;
    case REQUEST_TRANSFER_EXIT:
    {
      Serial.println("");
      Serial.printf("CAN transfer complete %lluB, request transfer exit!\n\r",totalSize);
      setInfo("CAN transfer complete "+String(totalSize)+"B, request transfer exit sent!");
      totalSize = 0x00u;

      CAN.beginPacket(0x100);
      CAN.write(1);
      CAN.write(0x37u);
      CAN.endPacket();

      canFinished = true;
      updateSMSatate = DEFAULT_STATE;
    }
    break;
    default:
    {
      //do nothing
    }
    break;
  }
}

void canTask(char caracter)
{
  updateSM();

  if((newImageReceived)||('5' == caracter))
  {
    newImageReceived = false; //consume event first

    //Sending PROGRAMMING_SESSION
    CAN.beginPacket(0x100);
    CAN.write(2);
    CAN.write(16);
    CAN.write(2);
    CAN.endPacket();

    delay(500);
    readCAN();
    if(LittleFS.exists("/SavedFile.txt"))
    {
      Serial.println("Begin Security Access Unlock!");
      updateSMSatate = SECURRITY_ACCESS;
    }
    else
    {
      Serial.println("No file has been uploaded!");
    }
  }

  if('d' == caracter)
  {
    Serial.print("Sending DEFAULT_SESSION");

    CAN.beginPacket(0x100);
    CAN.write(2);
    CAN.write(16);
    CAN.write(1);
    CAN.endPacket();

    Serial.println(" ...done");

  }
  else if('b' == caracter)
  {
    //Read DID is Firmware downloaded in FS
    CAN.beginPacket(0x100);
    CAN.write(2);
    CAN.write(0x22);
    CAN.write(0xE1);
    CAN.write(0x01);
    CAN.endPacket();
  }
  else if ('n' == caracter)
  {
    //Perform Roll Back Routine
    CAN.beginPacket(0x100);
    CAN.write(2);
    CAN.write(0x31);
    CAN.write(0xC1);
    CAN.write(0x21);
    CAN.endPacket();
  }

  readCAN();
}

void readCAN(void)
{
  uint16_t receivedCANID = 0x0000u;
  uint8_t index = 0x00u;

  // try to parse packet
  if (CAN.parsePacket()) 
  {
    receivedCANID = CAN.packetId();
    Serial.print( "ID: 0x");
    Serial.print( receivedCANID, HEX);
    Serial.print( " "); 

    if (CAN.packetRtr()) 
    {
      Serial.print( " DLC: ");
      Serial.print( CAN.packetDlc());  
      Serial.print( " ");      
    } 
    else 
    {      
      // only print packet data for non-RTR packets
      while (CAN.available()) 
      {
        rxData[index] = (uint8_t)CAN.read();
        Serial.print( (int)rxData[index], HEX);   
        Serial.print( " ");
        index++;  
      }
      Serial.println("");
      if(receivedCANID == UDS_RESPONSE_ID)
      {
        if(rxData[SID] == 0x67u) //Positive Response to Security Access
        {
          switch (rxData[SBF])
          {
            case SECURITY_ACCESS_UNLOCK:
              {
                //Seed is received
                uint8_t seed[SECURITY_ACCESS_SEED_SIZE] = {rxData[3], rxData[4]};
                Serial.printf("Received seed: %x %x \n\r", seed[0],seed[1]);
                setSeed(seed);
              }
              break;
            case SECURITY_ACCESS_SEND_KEY:
              {
                //Security access UNLOCKED, flash on UDS can start
                Serial.println("Begin CAN transfer!");
                setInfo("Begin CAN transfer!");
                Serial.println("");
                delay(500);
                updateSMSatate = REQUEST_DOWNALOAD;
              }
              break;
            
            default:
              {
                //do nothing
              }
              break;
          }
        }  
      }
      index = 0x00u;
      flushRxData();
      
      Serial.println("");
    }
  }
}

void canSendUDS(uint8 *data)
{
  Serial.print("Sending CAN message:");
  switch(data[0])
  {
    case 0x01:
    {
      Serial.printf("%x %x\n\r", data[0], data[1]);
      CAN.beginPacket(0x100);
      CAN.write(data[0]);
      CAN.write(data[1]);
      CAN.endPacket();
    }
    break;
    case 0x02:
    {
      Serial.printf("%x %x %x\n\r", data[0], data[1], data[2]);
      CAN.beginPacket(0x100);
      CAN.write(data[0]);
      CAN.write(data[1]);
      CAN.write(data[2]);
      CAN.endPacket();     
    }
    break;
    case 0x03:
    {
      Serial.printf("%x %x %x %x\n\r", data[0], data[1], data[2], data[3]);
      CAN.beginPacket(0x100);
      CAN.write(data[0]);
      CAN.write(data[1]);
      CAN.write(data[2]);
      CAN.write(data[3]);
      CAN.endPacket();
    }
    break;
    case 0x04:
    {
      Serial.printf("%x %x %x %x %x\n\r", data[0], data[1], data[2], data[3], data[4]);
      CAN.beginPacket(0x100);
      CAN.write(data[0]);
      CAN.write(data[1]);
      CAN.write(data[2]);
      CAN.write(data[3]);
      CAN.write(data[4]);
      CAN.endPacket();
    }
    break;
    case 0x05:
    {
      Serial.printf("%x %x %x %x %x %x\n\r", data[0], data[1], data[2], data[3], data[4], data[5]);
      CAN.beginPacket(0x100);
      CAN.write(data[0]);
      CAN.write(data[1]);
      CAN.write(data[2]);
      CAN.write(data[3]);
      CAN.write(data[4]);
      CAN.write(data[5]);
      CAN.endPacket();
    }
    break;
    case 0x06:
    {
      Serial.printf("%x %x %x %x %x %x %x\n\r", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
      CAN.beginPacket(0x100);
      CAN.write(data[0]);
      CAN.write(data[1]);
      CAN.write(data[2]);
      CAN.write(data[3]);
      CAN.write(data[4]);
      CAN.write(data[5]);
      CAN.write(data[6]);
      CAN.endPacket();
    }
    break;
    case 0x07:
    {
      Serial.printf("%x %x %x %x %x %x %x %x\n\r", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
      CAN.beginPacket(0x100);
      CAN.write(data[0]);
      CAN.write(data[1]);
      CAN.write(data[2]);
      CAN.write(data[3]);
      CAN.write(data[4]);
      CAN.write(data[5]);
      CAN.write(data[6]);
      CAN.write(data[7]);
      CAN.endPacket();
    }
    break;
    case 0x08:
    {
      Serial.printf("%x %x %x %x %x %x %x %x %x\n\r", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
      CAN.beginPacket(0x100);
      CAN.write(data[0]);
      CAN.write(data[1]);
      CAN.write(data[2]);
      CAN.write(data[3]);
      CAN.write(data[4]);
      CAN.write(data[5]);
      CAN.write(data[6]);
      CAN.write(data[7]);
      CAN.write(data[8]);
      CAN.endPacket();
    }
    break;
    default:
    break;
  }
}