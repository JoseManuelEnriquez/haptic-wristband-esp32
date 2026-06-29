#include <stdio.h>
#include "HapticController.hpp"
#include "driver/gpio.h"
#include "timer.hpp"

#define PIN_PWM GPIO_NUM_13
#define RESTART 0

extern "C" void app_main(void)
{
    HapticController* haptic_controller = HapticController::get_instance(PIN_PWM);
    TimerController timer_controller = TimerController();
    haptic_controller->init();
    uint8_t* value;

    timer_controller.start();
    timer_controller.set_count(RESTART);
    while(1){
        value = haptic_controller->hay_escritura();
        if(value != nullptr){
            ESP_LOGI("MAIN","Escritura recibida: %d", *value);
            haptic_controller->emitir_vibracion(*value);
            timer_controller.set_count(RESTART);
        }
        if(timer_controller.get_time_seconds() > 5){
            timer_controller.set_count(RESTART);
            haptic_controller->emitir_vibracion(0);
        }
        ESP_LOGI("Timer", "get_time: %f", timer_controller.get_time_seconds());
        vTaskDelay(10);
    } 
}
