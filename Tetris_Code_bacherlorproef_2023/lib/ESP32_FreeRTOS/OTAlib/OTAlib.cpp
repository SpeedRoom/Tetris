#include "OTAlib.h"

OTAlib::OTAlib(const char * wifi_ssid, const char * wifi_password)
{
    int len;
    // copy wifi ssid
    len = strlen(wifi_ssid);
    if(len > 0){
        this->_wifi_ssid = (char *) malloc(len+1);
    }
    strcpy(this->_wifi_ssid, wifi_ssid);

    // copy wifi password
    len = strlen(wifi_password);
    if(len > 0){
        this->_wifi_password = (char *) malloc(len+1);
    }
    strcpy(this->_wifi_password, wifi_password);
}

void OTAlib::begin(){
    // connect to wifi
    if(!WiFi.isConnected()){
        Serial.print(F("Connecting to "));
        Serial.println(this->_wifi_ssid);
        WiFi.mode(WIFI_STA);
        WiFi.begin(this->_wifi_ssid, this->_wifi_password);
        while (WiFi.waitForConnectResult() != WL_CONNECTED) {
            Serial.println(F("Connection Failed! Rebooting..."));
            delay(5000);
            ESP.restart();
        }
    }
    Serial.print(F("We got IP: "));
    Serial.println(WiFi.localIP());

    // configure OTA callbacks
    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
      else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
      else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
      else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
      else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
    });
    
    // start OTA
    ArduinoOTA.begin();

    // start task to handle OTA
    xTaskCreate(this->runOtaTask, "OtaTask", 10000, NULL, 1, this->_otaTask); 

    Serial.println("OTA is running.");
}
