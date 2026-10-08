#include <stdio.h>
#include "HapticController.hpp"
#include "battery.hpp"
#include "driver/gpio.h"
#include "timer.hpp"
#include "esp_pm.h"

#define PIN_PWM GPIO_NUM_13
#define PIN_ADC GPIO_NUM_12
#define PIN_LED GPIO_NUM_14
#define MIN_UMBRAL_BATTERY 0.33
#define LOW 0
#define HIGH 1

void pm_init();
void gpio_init();
static void vReadADCTask(void* pReadADCTask);
static void vPercentageTask(void* pPercentageTask);
static void vLedTask(void* pLedTask);

void pm_init(){
    esp_pm_config_t pm_config = {
        .max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ, // Max Frequency: 80MHz
        .min_freq_mhz = 10,
        .light_sleep_enable = true, // Se desactiva porque igualmente no podemos entrar en modo light sleep por BLE
    };

    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
}

void gpio_init(){
    gpio_config_t config = {
        .pin_bit_mask = (PIN_LED) << 1,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&config));
}

// Funcionalidad: Una tarea que cada minuto realiza lecturas en el ADC.
static void vReadADCTask(void* pReadADCTask){
    Battery* battery = (Battery*) pReadADCTask;
    TickType_t xLastWakeTime;
    // Definimos periodos de 1 minuto
    const TickType_t xFrequency = pdMS_TO_TICKS(60000);

    // Inicializa xLastWakeTime con el tiempo actual del sistema
    xLastWakeTime = xTaskGetTickCount();
    double voltage;
    while(1){
        battery->read_adc(&voltage);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }

    vTaskDelete(NULL);
}

// Funcionalidad: Una tarea que cada 10 segundos comprueba porcentaje de bateria
static void vPercentageTask(void* pPercentageTask){
    Battery* battery = (Battery*) pPercentageTask;
    float percentage;

    TickType_t xLastWakeTime;
    // Definimos periodos de 10 segundos
    const TickType_t xFrequency = pdMS_TO_TICKS(10000);
    // Inicializa xLastWakeTime con el tiempo actual del sistema
    xLastWakeTime = xTaskGetTickCount();
    
    TaskHandle_t xLedTaskHandle = NULL;

    while(1){
        battery->voltToPercentage(&percentage);
        if(percentage < MIN_UMBRAL_BATTERY && percentage != 0.0){
            xTaskCreate(vLedTask, "LED_Task", 2048, NULL, 5, &xLedTaskHandle);
        }else{
            if(xLedTaskHandle != NULL){
                vTaskDelete(xLedTaskHandle);
                xLedTaskHandle = NULL;
            }
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }

    vTaskDelete(NULL);
}

static void vLedTask(void* pLedTask){
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    // Cambia de estado cada 500 ms (frecuencia de parpadeo completa = 1 Hz)
    const TickType_t xFrequency = pdMS_TO_TICKS(500);
    uint8_t level = 0;
    for (;;) {
        level ^= level;
        gpio_set_level(PIN_LED, level);
        // Bloquea la tarea hasta que transcurran exactamente 500 ms
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }

    vTaskDelete(NULL);
}

extern "C" void app_main(void)
{
    Battery battery = Battery(PIN_ADC);
    xTaskCreate(vReadADCTask, "ReadADC_Task", 2048, NULL, 4, NULL);
    xTaskCreate(vPercentageTask, "Percentage_Task", 2048, NULL, 5, NULL);
    gpio_init();
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