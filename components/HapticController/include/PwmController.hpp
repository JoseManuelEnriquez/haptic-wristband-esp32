#ifndef PWMCONTROLLER_HPP
#define PWMCONTROLLER_HPP
#include "freertos/task.h"
#include "driver/ledc.h"

class PwmController{
    private:
        void vPWM_Task(void* vParametersTask);
        TaskHandle_t xPWMTaskHandle;
    public:
    PwmController(int _pin_gpio);
    void stop_pwm();
    void start_pwm(int duty);
};

#endif