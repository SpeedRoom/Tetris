#ifndef __LEDCTRL_H__
#define __LEDCTRL_H__

#include <Arduino.h>

typedef enum ledActions{
    LED_ON,
    LED_OFF,
    LED_TOGGLE,
    LED_BLINK,
    LED_IGNORE
} LedAction_t;

class LEDctrl
{
    private:
        // attributes
        uint8_t _pin;
        TaskHandle_t _hBlinkTask;
        QueueHandle_t _ctrlQueue;

        static void startBlinkTaskImpl(void* _this){
            static_cast<LEDctrl*>(_this)->blinkTask();
        }

        void blinkTask();
        void ctrlTask();

        bool isBlinking();
        void resumeBlink();
        void pauseBlink();

    public:
        LEDctrl(uint8_t pin);

        void begin(QueueHandle_t ctrlQueue=NULL);
};

#endif /*__LEDCTRL_H__*/
