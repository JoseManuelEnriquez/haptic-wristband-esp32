#ifndef PWMCONTROLLER_HPP
#define PWMCONTROLLER_HPP
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "driver/ledc.h"

class PwmController{
    private:
        // TaskHandle_t xPWMTaskHandle;
    public:
        PwmController(int _pin_gpio);
        void set_freq(int freq);
        void set_intensity(int value);
        void stop_pwm();
        void start_pwm(int duty);
};

#endif