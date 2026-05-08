#include <stdio.h>
#include "HapticController.hpp"

extern "C" void app_main(void)
{
    HapticController* haptic_controller = HapticController::get_instance(18);
    haptic_controller->init();
    uint8_t* value;
    while(1){
        value = haptic_controller->hay_escritura();
        if(value != nullptr){
            ESP_LOGI("MAIN","Escritura recibida: %d", *value);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    } 
}
