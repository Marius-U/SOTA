#include "myUpdateHandler.h"
#include "Display.h"

File file;

void initUpdateHandler(void)
{
   if(!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED))
  {
    Serial.println("LITTLEFS Mount Failed");
    while(1);
    return;
  }
}

void writeDataToFile(size_t index, uint8_t *data, size_t len, bool final)
{
   //Handle upload
  if(index)
  {
    //Serial.println("Upload Start!");
    DisplayPrint(tft, "Upload Start! ");

    if(LITTLEFS.exists("/SavedFile.bin"))
    {
      //Remove old file if exists.
      LITTLEFS.remove("/SavedFile.bin");
    }

    //Open the file 
    file = LITTLEFS.open("/SavedFile.bin", FILE_WRITE);
  }

  if(0 != len)
  {
      static int size = 0x00u;
      /*
      for(uint8_t index = 0x00; index <= len ; index++)
      {
         file.write(data[index]);
      }
      */
      file.write(data, len);
      size ++;
      if(size % 100 == 0)
      {
         DisplayPrint(tft, ".");
         Serial.print(".");
      }
  }
  
  if(final)
  {
    //Serial.println("");
    //Serial.println("Upload Success!");
    Display_Init(tft);
    tft.setCursor(0,0,2);
    tft.fillScreen(TFT_GREY);
    DisplayPrint(tft, "Upload Successfully! ");

    file.close();
  }
}

// perform the actual update from a given stream
void performUpdate(Stream &updateSource, size_t updateSize) {
   if (Update.begin(updateSize)) {      
      size_t written = Update.writeStream(updateSource);
      if (written == updateSize) {
         Serial.println("Written : " + String(written) + " successfully");
      }
      else {
         Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      }
      if (Update.end()) 
      {
         Serial.println("OTA done!");
         if (Update.isFinished()) {
            //Serial.println("Update successfully completed. Rebooting.");
            DisplayPrint(tft,"Update successfully completed. Rebooting. ");
            ESP.restart();
         }
         else {
            DisplayPrint(tft," Update not finished! Something went wrong! ");
            Serial.println("Update not finished! Something went wrong!");
         }
      }
      else 
      {
         String message = "Error Occurred. Error #: " + String(Update.getError());
         DisplayPrint(tft, message.c_str());
         Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }

   }
   else
   {
      DisplayPrint(tft,"Not enough space to begin OTA");
      Serial.println("Not enough space to begin OTA");
   }
}

// check given FS for valid update.txt and perform update if available
void updateFromFS(void) 
{
   File updateBin = LITTLEFS.open("/SavedFile.bin", "r");
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
         //Serial.println("Try to start update");
         DisplayPrint(tft,"Try to start update... ");
         performUpdate(updateBin, updateSize);
      }
      else 
      {
         Serial.println("Error, file is empty");
      }

      updateBin.close();
    
      // whe finished remove the binary from sd card to indicate end of the process
      LITTLEFS.remove("/SavedFile.bin");      
   }
   else 
   {
      Serial.println("Could not load SavedFile.bin!");
   }
}
