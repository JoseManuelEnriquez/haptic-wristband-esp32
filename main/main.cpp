#include <stdio.h>
#include "HapticController.hpp"
#include "driver/gpio.h"

#define PIN_PWM GPIO_NUM_13

extern "C" void app_main(void)
{
    HapticController* haptic_controller = HapticController::get_instance(PIN_PWM);
    haptic_controller->init();
    uint8_t* value;
    while(1){
        value = haptic_controller->hay_escritura();
        if(value != nullptr){
            ESP_LOGI("MAIN","Escritura recibida: %d", *value);
            haptic_controller->emitir_vibracion(*value);
        }
        vTaskDelay(10);
    } 
}
