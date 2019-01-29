//ESP8266 Flash FW by SPIFFS
//Vasco Santos 01/08/2018
//V0.1

#define header_begin  "E9 01 02 40 9C"
#define header_end  "00 00 00"


String ICACHE_FLASH_ATTR formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  }
  else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  }
  else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

  void RebootDevice()
  {
    WiFi.mode(WIFI_OFF);
    WiFi.persistent(false);
    server.stop();
    WiFi.forceSleepBegin();
    ESP.restart();
    ESP.reset();
  }

void ICACHE_FLASH_ATTR ListDir()
{
   Serial.print(F("SPIFFS File List:\n"));
 String str = "";
 Dir dir = SPIFFS.openDir("/");
 while (dir.next()) {
    str += dir.fileName();
    str += " = ";
    str += dir.fileSize();
    str += "\r\n";
 }
Serial.print(str);
}

void FormatORMount(bool force=false)
{

  if (!SPIFFS.begin() || force)
  {
    SPIFFS.end();
   Serial.print(F("Format SPIFFS..."));
   SPIFFS.format();
   RebootDevice();
  }else{

  ListDir();

  }
  
}

String getMAC()
{
  uint8_t mac[6];
char macAddr[14];
WiFi.macAddress(mac);

sprintf(macAddr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); /// capital letters at MAC address

  return String( macAddr );
}

void checkwifi(){

 if (  WiFi.status() != WL_CONNECTED) {
      ESP.wdtFeed();
     //delay(10);
  //  digitalWrite(blueled_pin,HIGH);
    delay(300);
    Serial.print(".");
    //ESP.wdtFeed();
    

 // digitalWrite(blueled_pin,LOW);

      //if (cnt == 5000){
      if (  millis() > 50000){
        Serial.println(F("\nNo wifi, force reset!"));
        WiFi.mode(WIFI_OFF);
        WiFi.persistent(false);
        server.stop();
        WiFi.forceSleepBegin();
        ESP.restart();
        ESP.reset();
                 
      }

      //  cnt++;
      
  }

}

void upgradeFW()
{
  
 char buff[5];
 char buff2[4];

char file_begin[15];
char file_end[15];
  
  if (SPIFFS.exists("/fw.bin") && SPIFFS.exists("/fwsize.txt")){
        
    Serial.setDebugOutput(true);
    WiFiUDP::stopAll();


    UploadFile = SPIFFS.open("/fwsize.txt", "r");
    String s=UploadFile.readStringUntil('\n');
    Serial.println(s);
    UploadFile.close();



UploadFile = SPIFFS.open("/fw.bin", "r");

if (UploadFile.size() != s.toInt())
{
  Serial.printf("Invalid fw size of %d, must be %d!",UploadFile.size(),s.toInt());
  return;
}

UploadFile.seek(0,SeekSet);
UploadFile.readBytes(buff,5);
sprintf(file_begin,"%02X %02X %02X %02X %02X",buff[0],buff[1],buff[2],buff[3],buff[4]);
Serial.printf("DATA1: %s\n",file_begin);

UploadFile.seek(4,SeekEnd);
UploadFile.readBytes(buff2,4);
sprintf(file_end,"%02X %02X %02X",buff2[0],buff2[1],buff2[2]);
Serial.printf("DATA2: %s\n",file_end);


UploadFile.seek(0,SeekSet);

if (strcmp(file_begin, header_begin) == 0 && strcmp(file_end, header_end) == 0)
{
  Serial.println(F("Binary file signature OK"));
}else{
  Serial.printf("Binary file signature invalid: %s => %s\n", file_begin, file_end);
  return;
}

    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

     if (!Update.begin(maxSketchSpace, U_FLASH)) {
       Update.printError(Serial);
     }else{
          Serial.println(F("Update start!"));
         // UploadFile = SPIFFS.open("/fw.bin", "r");

          //Update.write(UploadFile.readString(), UploadFile.size());
          
        while (UploadFile.available()) {
          uint8_t ibuffer[128];
          UploadFile.read((uint8_t *)ibuffer, 128);
          //Serial.println((char *)ibuffer);
          Update.write(ibuffer, sizeof(ibuffer));  
        }

          
        if (Update.end(true)) {
          
          if (UploadFile)
            UploadFile.close();

            Serial.println(F("Update Complete!"));
            SPIFFS.remove("/fw.bin");
            SPIFFS.remove("/fwsize.txt");
            
          ESP.restart();
        } else {
          Update.printError(Serial);
        }
          
      }
      
     }else{
      Serial.println(F("Update File not Found!"));
     }
  
}

