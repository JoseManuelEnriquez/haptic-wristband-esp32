#ifndef HAPTICCONTROLLER_HPP
#define HAPTICCONTROLLER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"

#include "sdkconfig.h"

#include "PwmController.hpp"

#define STOP 0
#define SLOW 1
#define FAST 2
#define SLOW_PULSE 50
#define FAST_PULSE 20
#define PULSE_DURATION 30

class HapticController {
    private:
        int pin_gpio;
        bool new_write;
        uint8_t last_value;
        HapticController(int _pin_gpio);
        PwmController* pwm_controller;
    public:
        void init();
        static HapticController* get_instance(int pin_gpio);
        void onWrite(uint8_t value);
        uint8_t* hay_escritura();
        void emitir_vibracion(int value);
        friend void vHapticTask(void* pvHapticTask);
};

#endif