#include "UDS.h"

/**************ModulePrivateVariables***********/

static int8_t acctiveSession = DEFAULT_SESSION;
static uint8_t udsRxBuffer[MAX_DLC];
static uint8_t udsTxBuffer[MAX_DLC];

/**********************************************/
/**************ModulePrivateFunctions**********/

void    UDS_sendPositiveResponse(void);
void    UDS_sendPositiveResponse(uint8_t buffer[]);
void    UDS_sendNegativeResponse(uint8_t NRC);
void    UDS_readDID(void);
void    UDS_writeDID(void);
void    UDS_securityAccess(void);
void    UDS_performRoutine(void);
size_t  UDS_setTxBuffer(uint8_t byte);
size_t  UDS_write(const uint8_t *buffer, size_t size);
void    UDS_sendResponse(void);
void    UDS_flushRxBuffer(void);
void    UDS_flushTxBuffer(void);


/**********************************************/

void UDS_main()
{
    Serial.write("UDS!");
    delay(1000);
}

int8_t UDS_process()
{
    int8_t retVal = ESP_OK;
    uint8_t pci = udsRxBuffer[PCI]; //Protocol Control Information -> DLC
    uint8_t sid = udsRxBuffer[SID]; //Service ID
#if 0 //def DEBUGG_MODE   
    Serial.print(pci);
    Serial.print(" ");
    Serial.print(sid);
    Serial.print(" ");
    Serial.print(udsRxBuffer[2]);
    Serial.print(" ");
    Serial.print(udsRxBuffer[3]);
    Serial.print(" ");
    Serial.print(udsRxBuffer[4]);
    Serial.print(" ");
    Serial.print(udsRxBuffer[5]);
    Serial.print(" ");
    Serial.print(udsRxBuffer[6]);
    Serial.print(" ");
    Serial.print(udsRxBuffer[7]);
    Serial.println("");
#endif
    switch(sid)
    {
        case DIAGNOSTIC_SESION_CONTROL:
        {
            if((PROGRAMMING_SESSION == udsRxBuffer[2])||(DEFAULT_SESSION == udsRxBuffer[2]))
            {
                acctiveSession = udsRxBuffer[2];
                UDS_sendPositiveResponse();
            }
            else
            {
                UDS_sendNegativeResponse(SERVICE_NOT_SUPPORTED);
            }

        }break;
        case ECU_RESET:
        {
            UDS_sendPositiveResponse();
            //Perform HARD Reset
            ESP.restart();
        }break;
        case READ_DID:
        {
            Serial.println("Read did command received!");
            UDS_readDID();
        }break;
        case SECURRITY_ACCESS:
        {
            UDS_securityAccess();
        }break;
        case WRITE_DID:
        {
            UDS_writeDID();

        }break;
        case ROUTINE_CONTROL:
        {
           UDS_performRoutine();
        }break;
        case REQUEST_DOWNALOAD:
        {
            if(getSecurityAccess())
            {
                //create the LittleFS and check preconditions
                writeDataToFile(true, NULL, 0, false);
            }
            else
            {
                UDS_sendNegativeResponse(SECURITY_ACCESS_DENIED);
            }
            

        }break;
        case TRANSFER_DATA:
        {
            
            if(getSecurityAccess())
            {
                //write the received chunks in the LittleFS folder.
                writeDataToFile(false, udsRxBuffer+3, pci, false);
            }
            else
            {
                UDS_sendNegativeResponse(SECURITY_ACCESS_DENIED);
            }
        }break;
        case REQUEST_TRANSFER_EXIT:
        {
            
            if(getSecurityAccess())
            {
               //Close the LittleFS folder and call updateFromFS(file)
                writeDataToFile(false, NULL, 0, true);
                UDS_sendPositiveResponse();
                updateFromFS(); 
            }
            else
            {
                UDS_sendNegativeResponse(SECURITY_ACCESS_DENIED);
            }
            
        }break;
        default:
        {
            //Respond with NRC - SERVICE_NOT_SUPPORTED
            UDS_sendNegativeResponse(SERVICE_NOT_SUPPORTED);
        }
    }

    return retVal;
}

void UDS_flushRxBuffer(void)
{
    uint8_t index = 0x00u;
    while(index < MAX_DLC)
    {
        udsRxBuffer[index] = 0x00u;
        index += 0x01u;
    }
}
void UDS_flushTxBuffer(void)
{
    uint8_t index = 0x00u;
    while(index < MAX_DLC)
    {
        udsTxBuffer[index] = 0x00u;
        index += 0x01u;
    }
}

int8_t UDS_setRxBuffer(uint8_t buffer[])
{
    int8_t retVal = ESP_FAIL;

    //memcpy(udsRxBuffer, buffer, size);
    std::copy(buffer, buffer+MAX_DLC, udsRxBuffer);
    retVal = ESP_OK;

    return retVal;
}

size_t  UDS_setTxBuffer(uint8_t buffer[])
{
    int8_t retVal = ESP_FAIL;

    //memcpy(udsTxBuffer, buffer, size);
    std::copy(buffer, buffer+MAX_DLC, udsTxBuffer);
    retVal = ESP_OK;

    return retVal;
}

void UDS_sendPositiveResponse(void)
{
    
    udsRxBuffer[SID] += 0x40u;

    UDS_setTxBuffer(udsRxBuffer);
    UDS_sendResponse();
}
void UDS_sendPositiveResponse(uint8_t buffer[])
{
    buffer[SID] += 0x40u;
    std::copy(buffer, buffer+MAX_DLC, udsTxBuffer);
    UDS_sendResponse();
}

void UDS_sendNegativeResponse(uint8_t NRC)
{
    udsRxBuffer[PCI] = 0x03u;
    udsRxBuffer[SID + 0x01] = udsRxBuffer[SID];
    udsRxBuffer[SID] = 0x7F;
    udsRxBuffer[SID + 0x02] = NRC;

    UDS_setTxBuffer(udsRxBuffer);
    UDS_sendResponse();
}

void  UDS_sendResponse(void)
{
    uint8_t size = sizeof(udsTxBuffer);
    uint8_t index = 0x00u;

    CAN.beginPacket(UDS_RESPONSE_ID);

    for(index = 0x00u; index < size; index++)
    {
        CAN.write(udsTxBuffer[index]);
        udsTxBuffer[index] = 0x00u;
    }

    CAN.endPacket();
}

void UDS_securityAccess(void)
{
    /* Supported levels:
     *  Level 1
     */
    uint8_t sbf = udsRxBuffer[2];
    Serial.printf("Security access request: %x\n\r", sbf);
    switch (sbf)
    {
    case SECURITY_ACCESS_UNLOCK:
        {
            //"Seed" is requested
            uint8_t seed[SECURITY_ACCESS_SEED_SIZE] = {0,0};
            getSeed(seed);
            Serial.printf("Seed: %x %x\n\r", seed[0], seed[1]);
            uint8_t data[MAX_DLC] = {udsRxBuffer[0], udsRxBuffer[1], udsRxBuffer[2],seed[0],seed[1], 0x55u, 0x55u, 0x55u};
            UDS_sendPositiveResponse(data);
        }
        break;
    case SECURITY_ACCESS_SEND_KEY:
        {
            uint8_t key[SECURITY_ACCESS_KEY_SIZE] = {0xFF,0xAA,0xFF,0xAA};
            getKey(key);
            Serial.printf("Key: %x %x %x %x\n\r", key[0],key[1],key[2],key[3]);
            if((key[0]==udsRxBuffer[3])&&
                (key[1]==udsRxBuffer[4])&&
                (key[2]==udsRxBuffer[5])&&
                (key[3]==udsRxBuffer[6]))
                {
                    //Security Access Unlocked, send Positive response
                    setSecurityAccess(true);
                    uint8_t data[MAX_DLC] = {udsRxBuffer[0], udsRxBuffer[1], udsRxBuffer[2],0x55u,0x55u, 0x55u, 0x55u, 0x55u};
                    UDS_sendPositiveResponse(data);
                }
                else
                {
                    //Negative response: INVALID_KEY 
                    UDS_sendNegativeResponse(INVALID_KEY);
                }
        }
        break;
    
    default:
        break;
    }
}

void UDS_readDID()
{
    /* Supported DID's:
     *  1 - READ_FINGERPRINT
     *  2 - READ_IMAGE_SAVED
     *  3 - READ_ROLLBACK_AVAILABLE
     */
    uint16_t did = (uint16_t)(((uint16_t)((uint16_t)udsRxBuffer[2] << 0x08u)) | ((uint16_t)(udsRxBuffer[3])));
    Serial.printf("Read DID: %x\n\r",did);
    switch (did)
    {
    case READ_FINGERPRINT:
       {
            /* code */
       }
        break;
    case READ_IMAGE_SAVED:
       {
            if(LITTLEFS.exists("/SavedFile.bin"))
            {
                Serial.println("FileSystem is saved internally!");
                //File system stored amd ready to be flashed
                udsRxBuffer[0] = 0x04; //Set PCI to 4
                //No file system present
                uint8_t data[MAX_DLC] = {udsRxBuffer[0], udsRxBuffer[1], udsRxBuffer[2],udsRxBuffer[3],FIRMWARE_STORED_INTERNAL, 0x00u, 0x00u, 0x00u};
                UDS_sendPositiveResponse(data);
            }
            else
            {
                Serial.println("NO FS saved internally!");
                udsRxBuffer[0] = 0x04; //Set PCI to 4
                //No file system present
                uint8_t data[MAX_DLC] = {udsRxBuffer[0], udsRxBuffer[1], udsRxBuffer[2],udsRxBuffer[3],FIRMWARE_NOT_PRESENT, 0x00u, 0x00u, 0x00u};
                UDS_sendPositiveResponse(data);
            }
       }
        break;
    case READ_SW_VERSION:
        {
            uint8_t data[MAX_DLC] = {udsRxBuffer[0], udsRxBuffer[1], udsRxBuffer[2],udsRxBuffer[3],SW_MAJOR, SW_MINOR, SW_PATCH, 0x00u};
            UDS_sendPositiveResponse(data);
        }
        break;
    case READ_ROLLBACK_AVAILABLE:
       {
            if(Update.canRollBack())
            {
                udsRxBuffer[0] = 0x04; //Set PCI to 4
                //Preconditions for RollBack are true - send Positive Response
                uint8_t data[MAX_DLC] = {udsRxBuffer[0], udsRxBuffer[1], udsRxBuffer[2],udsRxBuffer[3],CAN_ROLLBACK,0x00u ,0x00u , 0x00u};
                UDS_sendPositiveResponse(data);
            }
            else
            {
                udsRxBuffer[0] = 0x04; //Set PCI to 4
                //Preconditions for RollBack are true - send Positive Response
                uint8_t data[MAX_DLC] = {udsRxBuffer[0], udsRxBuffer[1], udsRxBuffer[2],udsRxBuffer[3],ROLLBACK_NOT_POSSIBLE,0x00u ,0x00u , 0x00u};
                UDS_sendPositiveResponse(data);
            }
       }
        break;
    default:
    {
        UDS_sendNegativeResponse(REQUEST_OUT_OF_RANGE);
    }
        break;
    }
}

void UDS_writeDID()
{
    /* Supported DID's:
     *  1 - WRITE_FINGERPRINT
     */
    uint16_t did = (uint16_t)(((uint16_t)((uint16_t)udsRxBuffer[2] << 0x08u)) | ((uint16_t)(udsRxBuffer[3])));
    switch (did)
    {
    case WRITE_FINGERPRINT:
       {
            /* code */
       }
        break;
    
    default:
    {
        UDS_sendNegativeResponse(REQUEST_OUT_OF_RANGE);
    }
        break;
    }
}

void UDS_performRoutine()
{
    /* Supported DID's:
     *  1 - PERFORM_ROLLBACK
     */
    uint16_t did = (uint16_t)(((uint16_t)((uint16_t)udsRxBuffer[2] << 0x08u)) | ((uint16_t)(udsRxBuffer[3])));
    Serial.printf("Routine control: %x\n\r",did);
    switch (did)
    {
    case PERFORM_ROLLBACK:
       {
            if(Update.canRollBack())
            {
                uint32_t retVal = 0x00u;
                Serial.println("Rollback seems to be possible, start rollback");
                retVal = Update.rollBack();
                if(true == retVal)
                {
                    Serial.println("Rollback performed! Restarting!");
                    ESP.restart();
                }
                else
                {
                    Serial.print("RollBack error: ");
                    switch (retVal)
                    {
                    case ESP_ERR_INVALID_ARG:
                        Serial.println("ESP_ERR_INVALID_ARG");
                        break;
                    case ESP_ERR_NOT_FOUND:
                        Serial.println("ESP_ERR_NOT_FOUND");
                        break;
                    case ESP_ERR_FLASH_OP_TIMEOUT:
                        Serial.println("ESP_ERR_FLASH_OP_TIMEOUT");
                        break;
                    case ESP_ERR_FLASH_OP_FAIL:
                        Serial.println("ESP_ERR_FLASH_OP_FAIL");
                        break;
                    
                    default:
                        Serial.printf("0x%x\n\r",retVal);
                        break;
                    }
                    
                }
            }
            else
            {
                Serial.println("Rollback is not possible!");
                //RollBack can not be performed - send Negative Response
                UDS_sendNegativeResponse(CONDITIONS_NOT_CORRECT);
            }
       }
        break;
    
    default:
        {
            UDS_sendNegativeResponse(REQUEST_OUT_OF_RANGE);
        }
        break;
    }
}

int8_t UDS_getActiveSession()
{
    return  acctiveSession;
}
