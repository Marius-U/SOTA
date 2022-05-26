#include "myServer.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
AsyncEventSource events("/events"); // event source (Server-Sent events)


const char* ssid = "ULM";
const char* password = "Nowomannocry";
const char* http_username = "admin";
const char* http_password = "admin";
String infoLogged = "Select the Firmware you want to reflash.";
File file;

bool shouldReboot = false;
bool fileWasUploaded = false;

const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8"/>
  <title>SOTA</title>
</head>
	<body>
    <iframe 
			style="width:100%; height: 80px;" 
			frameborder="0" 
			scrolling="no" 
			src="http://192.168.100.73/info">
		</iframe>
		<form action="http://192.168.100.73/upload" method="post" enctype="multipart/form-data">
			<p><input type="file" name="file1">
			<p><button type="submit">Submit</button>
		</form>
	</body>
</html>
)rawliteral";


void initWifi(void)
{
   WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n\r");
    return;
  }

  Serial.println("WIFI initialized and connected!");
  IPAddress ip =  WiFi.localIP();
  String strIP = ip.toString();
  const char* str = strIP.c_str();
  Serial.print("IP: ");
  Serial.println(str);

  initServer();
}

void deinitWifi(void)
{
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("WIFI stoped!");
  delay(3000);

}

void setInfo(String s)
{
  infoLogged = s;
}

bool getShouldReboot()
{
    return shouldReboot;
}

void setShouldReboot(bool value)
{
  shouldReboot = value;
}
bool getFileIsUploaded()
{
  return fileWasUploaded;
}
void setFileIsUploaded(bool state)
{
  fileWasUploaded = state;
}
void onRequest(AsyncWebServerRequest *request)
{
  //Handle Unknown Request
  request->send(404);
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  //Handle body
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  //Handle upload
  if(!index)
  {
    infoLogged = "Upload Start: "+ filename;
    Serial.printf("Upload Start: %s\n\r", filename.c_str());
    Serial.println("");

    if(LittleFS.exists("/SavedCryptedFile.txt"))
    {
      //Remove old file if exists.
      LittleFS.remove("/SavedCryptedFile.txt");
    }

    //Open the file 
    file = LittleFS.open("/SavedCryptedFile.txt", "w+");
  }

  if(0 != len)
  {
    uint8_t size = file.write(data, len);
    Serial.print(".");
    delay(1);
    if(size != len)
    {
     // Serial.printf("Uploaded pack failed! size = %u; len = %u write error = %u \n\r", size, len, file.getWriteError());
      
    }
  }
  
  if(final)
  {
    infoLogged += " \n\rUpload Success: "+ String(index+len)+"B start update via CAN!";
    Serial.println("");
    Serial.printf("Upload Success: %uB\n\r", index+len);
    file.close();
    fileWasUploaded = true;
  }
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  //Handle WebSocket event
}

void initServer()
{
  // attach AsyncWebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // attach AsyncEventSource
  server.addHandler(&events);

  // respond to GET requests on URL /heap
  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request){
    String value = "<html><head><meta http-equiv=\"refresh\" content=\"2\">"
                + infoLogged + "</body></html>";
    request->send(200, "text/html", value);
  });
  // respond to GET requests on URL /heap
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });
    // Send web page with input fields to client
  server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request){
    
    request->send_P(200, "text/html", index_html);
  });
  // upload a file to /upload
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  }, onUpload);

  // send a file when /index is requested
  server.on("/index", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.htm");
  });

  // HTTP basic authentication
  server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password))
        return request->requestAuthentication();
    request->send(200, "text/plain", "Login Success!");
  });

  // Simple Firmware Update Form
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      Serial.printf("Update Start: %s\n\r", filename.c_str());
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        Serial.printf("Update Success: %uB\n\r", index+len);
      } else {
        Update.printError(Serial);
      }
    }
  });

  // attach filesystem root at URL /fs
  server.serveStatic("/fs", SPIFFS, "/");

  // Catch-All Handlers
  // Any request that can not find a Handler that canHandle it
  // ends in the callbacks below.
  server.onNotFound(onRequest);
  server.onFileUpload(onUpload);
  server.onRequestBody(onBody);

  server.begin();
}