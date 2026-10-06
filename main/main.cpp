#include <stdio.h>
#include "HapticController.hpp"
#include "driver/gpio.h"
#include "timer.hpp"
#include "esp_pm.h"

#define PIN_PWM GPIO_NUM_13
#define RESTART 0

void pm_init();

extern "C" void app_main(void)
{
    HapticController* haptic_controller = HapticController::get_instance(PIN_PWM);
    TimerController timer_controller = TimerController();
    haptic_controller->init();
    pm_init();
    uint8_t* has_read;

    timer_controller.start();
    timer_controller.set_count(RESTART);
    while(1){
        has_read = haptic_controller->hay_escritura();
        if(has_read != nullptr){
            ESP_LOGI("MAIN","Escritura recibida: %d", *has_read);
            haptic_controller->emitir_vibracion(*has_read);
            timer_controller.set_count(RESTART);
        }
        if(timer_controller.get_time_seconds() > 5){
            timer_controller.set_count(RESTART);
            haptic_controller->emitir_vibracion(0);
        }
        ESP_LOGI("Timer", "get_time: %f", timer_controller.get_time_seconds());
        timer_controller.stop();
        vTaskDelay(200);
    } 
}

void pm_init(){
    esp_pm_config_t pm_config = {
        .max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ, // Max Frequency: 80MHz
        .light_sleep_enable = false, // Se desactiva porque igualmente no podemos entrar en modo light sleep por BLE
    };

    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
}