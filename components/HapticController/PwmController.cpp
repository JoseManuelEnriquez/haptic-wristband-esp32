#include "include/PwmController.hpp"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (2) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (8192) // Set duty to 50%. (2 ** 13) * 50% = 4096
#if CONFIG_PM_ENABLE
#define LEDC_CLK_SRC            LEDC_USE_RC_FAST_CLK // choose a clock source that can maintain during light sleep
#define LEDC_FREQUENCY          (400) // Frequency in Hertz. Set frequency at 400 Hz
#else
#define LEDC_CLK_SRC            LEDC_AUTO_CLK
#define LEDC_FREQUENCY          (1000) // Frequency in Hertz. Set frequency at 4 kHz
#endif

PwmController::PwmController(int _pin_gpio){
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
        .gpio_num       = _pin_gpio,
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
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void PwmController::start_pwm(int duty){
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void PwmController::set_intensity(int value){
    ESP_ERROR_CHECK(ledc_set_duty_with_hpoint(LEDC_MODE, LEDC_CHANNEL, value, value));
}

