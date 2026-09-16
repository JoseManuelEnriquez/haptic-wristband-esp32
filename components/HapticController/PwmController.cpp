#include "include/PwmController.hpp"

PwmController::PwmController(int _pin_gpio):xPWMTaskHandle(NULL){
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_CLK_SRC,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = pin_gpio,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0,
        #if CONFIG_PM_ENABLE
                .sleep_mode     = LEDC_SLEEP_MODE_KEEP_ALIVE,
        #endif
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void PwmController::stop_pwm(){
    if(xPWMTaskHandle != NULL){
        vTaskDelete(xPWMTaskHandle);
        xPWMTaskHandle = NULL;
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }
}

void PwmController::start_pwm(int duty){
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    xTaskCreate(vPWM_Task, "PWM_Task", 2048, &pwmConfig, 5, &xPWMTaskHandle); 
}

void PwmController::vPWM_Task(void* vParametersTask) {
    int* duty = (int*)vParametersTask;
    while (1) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, *duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }
}
