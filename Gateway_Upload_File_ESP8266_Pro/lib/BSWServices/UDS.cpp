#include "UDS.h"

/**************ModulePrivateVariables***********/

static int8_t acctiveSession = DEFAULT_SESSION;
static uint8_t udsRxBuffer[MAX_DLC];
static uint8_t udsTxBuffer[MAX_DLC];

/**********************************************/
/**************ModulePrivateFunctions**********/

void    UDS_sendPositiveResponse(void);
void    UDS_sendNegativeResponse(uint8_t NRC);
void    UDS_writeDID();
size_t  UDS_setTxBuffer(uint8_t byte);
size_t  UDS_write(const uint8_t *buffer, size_t size);
void    UDS_sendResponse(void);

/**********************************************/

void UDS_main()
{
    Serial.write("UDS!");
    delay(1000);
}

int8_t UDS_process()
{
    int8_t retVal = ESP_OK;
    //uint8_t pci = udsRxBuffer[0]; //Protocol Control Information -> DLC
    uint8_t sid = udsRxBuffer[1]; //Service ID
    switch(sid)
    {
        case DIAGNOSTIC_SESION_CONTROL:
        {
            acctiveSession = udsRxBuffer[2];
            UDS_sendPositiveResponse();

        }break;
        case ECU_RESET:
        {
            //Perform HARD Reset
            ESP.restart();
        }break;
        case READ_DID:
        {
            //Respond with NRC - SERVICE_NOT_SUPPORTED
            UDS_sendNegativeResponse(SERVICE_NOT_SUPPORTED);
        }break;
        case SECURRITY_ACCESS:
        {
            //To do: Implement if time permited.
        }break;
        case WRITE_DID:
        {
            UDS_writeDID();

        }break;
        case ROUTINE_CONTROL:
        {

        }break;
        case REQUEST_DOWNALOAD:
        {

        }break;
        case TRANSFER_DATA:
        {

        }break;
        case REQUEST_TRANSFER_EXIT:
        {

        }break;
        default:
        {
            //Respond with NRC - SERVICE_NOT_SUPPORTED
            UDS_sendNegativeResponse(SERVICE_NOT_SUPPORTED);
        }
    }

    return retVal;
}

int8_t UDS_setRxBuffer(uint8_t *buffer)
{
    int8_t retVal = ESP_FAIL;
    size_t size = (size_t)(sizeof(buffer)/sizeof(buffer[0]));

    if (size <= (sizeof(udsRxBuffer))) 
    {
        memcpy(udsRxBuffer, buffer, size);
        retVal = ESP_OK;
    }
    else
    {
        //Size of the received CAN mesage is bigger than the size of the UDS buffer
    }

    return retVal;
}

size_t  UDS_setTxBuffer(uint8_t *buffer)
{
        int8_t retVal = ESP_FAIL;
    size_t size = (size_t)(sizeof(buffer)/sizeof(buffer[0]));

    if (size <= (sizeof(udsTxBuffer))) 
    {
        memcpy(udsTxBuffer, buffer, size);
        retVal = ESP_OK;
    }
    else
    {
        //Size of the transmit CAN mesage is bigger than the size of the UDS buffer
    }

    return retVal;
}

void UDS_sendPositiveResponse(void)
{
    
    udsRxBuffer[SID] += 0x40u;

    UDS_setTxBuffer(udsRxBuffer);
    UDS_sendResponse();
}

void UDS_sendNegativeResponse(uint8_t NRC)
{
    udsRxBuffer[SID] = 0x7F;
    udsRxBuffer[SID + 0x01] = NRC;

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

void UDS_writeDID()
{
    /* Supported DID's:
     *  1 - WRITE_FINGERPRINT
     */
    uint16_t did = (uint16_t)(((uint16_t)(udsRxBuffer[2] << 0x08u)) & ((uint16_t)(udsRxBuffer[3])));
    switch (did)
    {
    case WRITE_FINGERPRINT:
       {
            /* code */
       }
        break;
    
    default:
        break;
    }
}

int8_t UDS_getActiveSession()
{
    return  acctiveSession;
}
