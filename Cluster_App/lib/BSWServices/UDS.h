#ifndef UDS_HEADER
#define UDS_HEADER

#include <Arduino.h>
#include <string.h>
#include <CAN.h>
#include "myUpdateHandler.h"
#include "myCryptoHandler.h"
#include "Version.h"

/*************MacroDefinitions***********/

#define DEBUGG_MODE

#define MAX_DLC 0x08u
#define PCI     0x00u
#define SID     0x01u

#define UDS_REQUEST_ID      0x100u
#define UDS_RESPONSE_ID     0x101u

//Diagnostic and Communication Management Functional Unit 
#define DIAGNOSTIC_SESION_CONTROL   0x10u
#define ECU_RESET                   0x11u
#define READ_DID                    0x22u
#define SECURRITY_ACCESS            0x27u
#define WRITE_DID                   0x2Eu
#define ROUTINE_CONTROL             0x31u
#define REQUEST_DOWNALOAD           0x34u
#define TRANSFER_DATA               0x36u
#define REQUEST_TRANSFER_EXIT       0x37u

//Sub-Function ID (SBF)
#define DEFAULT_SESSION             0x01u
#define PROGRAMMING_SESSION         0x02u
#define EXTENDED_DIAG_SESSION       0x03u
#define SECURITY_ACCESS_UNLOCK      0x01u
#define SECURITY_ACCESS_SEND_KEY    (SECURITY_ACCESS_UNLOCK + 0x01u)


#define READ_FINGERPRINT            0xF15Bu
#define READ_IMAGE_SAVED            0xE101u
#define READ_SW_VERSION             0xF189u
#define READ_ROLLBACK_AVAILABLE     0xA121u

#define WRITE_FINGERPRINT           0xF15Au
#define PERFORM_ROLLBACK            0xC121u

#define FIRMWARE_STORED_INTERNAL    0x01u
#define CAN_ROLLBACK                0x01u
#define FIRMWARE_NOT_PRESENT        0xFFu
#define ROLLBACK_NOT_POSSIBLE       0xFFu



#define HARD_RESET                  0x01u

//Negative Response code (NRC)
#define SERVICE_NOT_SUPPORTED   0x11u
#define CONDITIONS_NOT_CORRECT  0x22u
#define REQUEST_OUT_OF_RANGE    0x31u
#define SECURITY_ACCESS_DENIED  0x33u
#define INVALID_KEY             0x35u

/***************************************/
/*************GlobalVariables***********/

/***************************************/
/*************GlobalFunctions***********/

void UDS_main(void);
int8_t UDS_process(void);
int8_t UDS_getActiveSession(void);
int8_t UDS_setRxBuffer(uint8_t buffer[]);

/***************************************/

#endif