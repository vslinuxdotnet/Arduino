#include "Switch.h"

//<<constructor>>
Switch::Switch(){
    //Serial.println(F("default constructor called"));
}
//Switch::Switch(String alexaInvokeName,unsigned int port){
//Switch::Switch(String alexaInvokeName, unsigned int port, CallbackFunction oncb, CallbackFunction offcb){
Switch::Switch(String alexaInvokeName, unsigned short port, CallbackFunction actionfunct){
    uint32_t chipId = ESP.getChipId();
    char uuid[64];
    //sprintf_P(uuid, PSTR("38323636-4558-4dda-9188-cda0e6%02x%02x%02x"),
    sprintf(uuid, "38323636-4558-4dda-9188-cda0e6%02x%02x%02x",
          (uint16_t) ((chipId >> 16) & 0xff),
          (uint16_t) ((chipId >>  8) & 0xff),
          (uint16_t)   chipId        & 0xff);
    
    serial = String(uuid);
    persistent_uuid = "Socket-1_0-" + serial+"-"+ String(port);
        
    device_name = alexaInvokeName;
    localPort = port;
    //onCallback = oncb;
    //offCallback = offcb;
    ActionCallback = actionfunct;
     
    startWebServer();
}


 
//<<destructor>>
Switch::~Switch(){/*nothing to destruct*/}

/*
void Switch::serverLoop(){
    if (server != NULL) {
       // server->handleClient();
        //delay(1);
    }
}
*/

ICACHE_FLASH_ATTR void notFound(AsyncWebServerRequest *request) {
    request->send(404, F("text/plain"), F("404 Url Not found"));
}


void ICACHE_FLASH_ATTR Switch::startWebServer(){
 //server = new ESP8266WebServer(localPort);
 server = new AsyncWebServer(localPort);
 
 server->onNotFound(notFound);

 server->on("/", HTTP_GET, [&](AsyncWebServerRequest *request){
  String entertxt = F("Please tell Alexa to discover devices :P\n");
  entertxt += "Device: "+device_name+"\n";
  entertxt += F("Free Heap: ");
  entertxt += String(ESP.getFreeHeap());
  
    request->send(200, F("text/plain"), entertxt);
 });

 server->on("/setup.xml", HTTP_GET, [&](AsyncWebServerRequest *request){

    Serial.println(F("### Responding to setup.xml ... ###\n"));
  
    IPAddress localIP = WiFi.localIP();
    char s[16];
    sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
   
    char setup_xml[strlen_P(BELKIN_setup_xml_model) + strlen(device_name.c_str()) + strlen(persistent_uuid.c_str())];
    snprintf_P(setup_xml, sizeof(setup_xml),  BELKIN_setup_xml_model, device_name.c_str(), persistent_uuid.c_str());

    Serial.print(F("Sending :"));
    Serial.println(setup_xml);
    
    request->send(200, F("text/xml"), setup_xml);
    
     memset(setup_xml, 0, sizeof setup_xml);
     memset(s, 0, sizeof s);
  
 });

  //server->on("/setup.xml", [&]() {
 // server->on("/setup.xml", HTTP_GET, handleSetupXml);

  //server->on("/upnp/control/basicevent1", [&]() {
  //  handleUpnpControl();
  //});



  server->on("/upnp/control/basicevent1", /*HTTP_ANY*/ HTTP_POST, [](AsyncWebServerRequest *request){
    //none is ok
     }, NULL, [&](AsyncWebServerRequest *request, uint8_t *vdata, size_t len, size_t index, size_t total){
   
      Serial.println(F("### Responding to  /upnp/control/basicevent1 ... ###"));      
  
  //for (int x=0; x <= HTTP.args(); x++) {
  //  Serial.println(HTTP.arg(x));
  //}

  
String requestit = "";

/*
int headers = request->headers();
int i;
for(i=0;i<headers;i++){
  Serial.printf("HEADER[%s]: %s\n", request->headerName(i).c_str(), request->header(i).c_str());
}

int params = request->params();

for(int i=0;i<params;i++){
  AsyncWebParameter* p = request->getParam(i);
  if(p->isFile()){ //p->isPost() is also true
    Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
  } else if(p->isPost()){
    Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
  } else {
    Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
  }
}

//Serial.printf(request->url().c_str());  

int args = request->args();
for(int i=0;i<args;i++){
  Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
}
*/

  //String request = server->arg(0);  
 
  requestit = (char*)vdata;

 // Serial.print("request:");
 // Serial.println(requestit);
  

  if(requestit.indexOf(F("SetBinaryState")) >= 0) {

    char body[strlen_P(BELKIN_body_model) + sizeof(switchStatus)];
    
    if(requestit.indexOf(F("<BinaryState>1</BinaryState>")) >= 0) {
        Serial.println(F("Got Turn on request"));
        requestit = "";
        switchStatus =  ActionCallback(true);//onCallback();

        //char body[strlen_P(BELKIN_body_model) + sizeof(switchStatus)];
        snprintf_P(body, sizeof(body),  BELKIN_body_model, switchStatus);
        request->send(200, F("text/xml"), body);
        memset(body, 0, sizeof body);
        return;
    }

    if(requestit.indexOf(F("<BinaryState>0</BinaryState>")) >= 0) {
        Serial.println(F("Got Turn off request"));
        requestit = "";
        switchStatus =  ActionCallback(false);
        
        snprintf_P(body, sizeof(body),  BELKIN_body_model, switchStatus);
        request->send(200, F("text/xml"), body);
        memset(body, 0, sizeof body);
        return;
    }
    
  }

  if(requestit.indexOf(F("GetBinaryState")) >= 0) {
    Serial.println(F("Got binary state request"));
    requestit = "";
    //sendRelayState(request);
    char body[strlen_P(BELKIN_body_model) + sizeof(switchStatus)];
    snprintf_P(body, sizeof(body),  BELKIN_body_model, switchStatus);
    request->send(200, F("text/xml"), body);
    memset(body, 0, sizeof body);
    return;
  }
  
  request->send(200, F("text/plain"), F("No Data!"));
  
  });

 server->on("/eventservice.xml", HTTP_GET, [&](AsyncWebServerRequest *request){

  Serial.println(F("### Responding to eventservice.xml ... ###\n")); 

  char sbody[strlen_P(BELKIN_eventservice_xml_model)];
   
   sprintf_P(sbody, BELKIN_eventservice_xml_model);
   request->send(200, F("text/plain"),sbody);
   memset(sbody, 0, sizeof sbody);
    
 });
  //server->on("/eventservice.xml", [&]() {
   // handleEventservice();
  //});

  //server->onNotFound(handleNotFound);
  server->begin();

  Serial.print(F("WebServer started on port: "));
  Serial.println(localPort);
}
 

String Switch::getAlexaInvokeName() {
    return device_name;
}

void ICACHE_FLASH_ATTR Switch::sendRelayState() {

  

  //Serial.print(F("Sending :"));
  //Serial.println(body);
}

void ICACHE_FLASH_ATTR Switch::respondToSearch(IPAddress& senderIP, unsigned short senderPort) {

  Serial.print(F("Sending response to "));
  Serial.println(senderIP);
  Serial.print(F("Port : "));
  Serial.println(senderPort);

  IPAddress localIP = WiFi.localIP();
  char s[16];
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
//FPSTR(
char response[strlen_P( BELKIN_response_model) + strlen_P(s) + sizeof(localPort)+strlen_P(persistent_uuid.c_str())];
    snprintf_P(response, sizeof(response),  BELKIN_response_model, s, localPort, persistent_uuid.c_str());
    
  UDP.beginPacket(senderIP, senderPort);
  UDP.write(response);
  UDP.endPacket();
  Serial.println(F("Response sent!"));
   memset(response, 0, sizeof response);
   memset(s, 0, sizeof s);
  
}
