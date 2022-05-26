#include "myUpdateHandler.h"

void updaterTask()
{
   if(getFileIsUploaded())
  {
     setFileIsUploaded(false);
    //Upload finished, start update via CAN!
    deinitWifi();
    initCAN();
    decryptFromFS();
    //delay(1000);  //give time to the HTML response to be sent 
    setNewImageReceived(true); //set event for the Update to start
  }
  if(getCANFinished())
  {
      resetCANFinished();
      deinitCAN();
      initWifi();
  }
}
// perform the actual update from a given stream
void performUpdate(Stream &updateSource, size_t updateSize) {
   if (Update.begin(updateSize)) 
   {      
      size_t written = Update.writeStream(updateSource);
      if (written == updateSize) 
      {
         Serial.println("Written : " + String(written) + " successfully");
      }
      else 
      {
         Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      }
      if (Update.end()) 
      {
         Serial.println("OTA done!");
         if (Update.isFinished()) 
         {
            Serial.println("Update successfully completed. Rebooting.");
            setShouldReboot(true);
         }
         else 
         {
            Serial.println("Update not finished? Something went wrong!");
         }
      }
      else 
      {
         Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }

   }
   else
   {
      Serial.println("Not enough space to begin OTA");
   }
}

// check given FS for valid update.txt and perform update if available
void updateFromFS(void) 
{
   File updateBin = LittleFS.open("/SavedFile.txt", "r");
   if (updateBin) 
   {
      if(updateBin.isDirectory())
      {
         Serial.println("Error, update.txt is not a file");
         updateBin.close();
         return;
      }

      size_t updateSize = updateBin.size();

      if (updateSize > 0) 
      {
         Serial.println("Try to start update");
         performUpdate(updateBin, updateSize);
      }
      else 
      {
         Serial.println("Error, file is empty");
      }

      updateBin.close();
    
      // whe finished remove the binary from sd card to indicate end of the process
      LittleFS.remove("/SavedFile.txt");      
   }
   else 
   {
      Serial.println("Could not load SavedFile.bin!");
   }
}
