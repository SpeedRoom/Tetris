#ifndef __OTA_LIB_H__
#define __OTA_LIB_H__

#include <Arduino.h>
#include <ArduinoOTA.h>

class OTAlib
{
    private:
        /* attributes */
        char * _wifi_ssid;
        char * _wifi_password;

        TaskHandle_t * _otaTask;

        static void runOtaTask(void * param){
            Serial.print(F("Running OTA on core "));
            Serial.println(xPortGetCoreID());

            while(1){
                ArduinoOTA.handle();
                taskYIELD();
            }
        }

    public:
        OTAlib(const char * wifi_ssid, const char * wifi_password);

        void setPort(uint16_t port){
            ArduinoOTA.setPort(port);
        }

        void setTimeout(int timeoutInMillis){
            ArduinoOTA.setTimeout(timeoutInMillis);
        }

        void setHostname(const char * hostname){
            ArduinoOTA.setHostname(hostname);
        }

        void setPassword(const char * pw, bool isHash=false){
            if(isHash){
                ArduinoOTA.setPasswordHash(pw);
            }
            else{
                ArduinoOTA.setPassword(pw);
            }
        }

        void begin();
};

#endif /*__OTA_LIB_H__*/
