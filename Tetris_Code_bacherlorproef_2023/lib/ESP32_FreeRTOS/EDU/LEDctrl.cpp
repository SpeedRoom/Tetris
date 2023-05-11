#include "LEDctrl.h"

LEDctrl::LEDctrl(uint8_t pin){
    this->_pin = pin;
    pinMode(this->_pin, OUTPUT);
    digitalWrite(this->_pin, LOW);
}

void LEDctrl::blinkTask(void){
    while (1) {
        digitalWrite(this->_pin, !digitalRead(this->_pin));
        vTaskDelay(configTICK_RATE_HZ/2);
    }
}

void LEDctrl::ctrlTask(void){
    // TODO: add code
}

void LEDctrl::begin(QueueHandle_t ctrlQueue){
    this->_ctrlQueue = ctrlQueue;

    // start task for blinking the led
    xTaskCreate(this->startBlinkTaskImpl, "blinkTask", 2048, (void *)this, 2, &(this->_hBlinkTask));
    // and immediately pause blink task
    // TODO: add code

    // now make sure the led is off
    // TODO: add code

    // start task for reading ctrl messages from the queue
    // TODO: add code
}
