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
    haptic_controller->init();
    pm_init();
    uint8_t* has_read;

    while(1){
        has_read = haptic_controller->hay_escritura();
        if(has_read != nullptr){
            ESP_LOGI("MAIN","Escritura recibida: %d", *has_read);
            haptic_controller->emitir_vibracion(*has_read);
        }
        vTaskDelay(200);
    } 
}

void pm_init(){
    esp_pm_config_t pm_config = {
        .max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ, // Max Frequency: 80MHz
        .min_freq_mhz = 10,
        .light_sleep_enable = true, // Se desactiva porque igualmente no podemos entrar en modo light sleep por BLE
    };

    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
}