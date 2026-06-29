#include "./include/timer.hpp"

TimerController::TimerController(): gptimer(NULL){
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
        .direction = GPTIMER_COUNT_UP,      // Counting direction is up
        .resolution_hz = 1,   // Resolution is 1 Hz, i.e., 1 tick equals 1 second
    };
    // Create a timer instance
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    // Enable the timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    // Start the timer
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}

void TimerController::start(){
    gptimer_start();
}

void TimerController::stop(){
    gptimer_stop();
}

void TimerController::get_time_seconds(){
    // Check the timer's resolution
    uint32_t resolution_hz;
    ESP_ERROR_CHECK(gptimer_get_resolution(gptimer, &resolution_hz));
    
    // Read the current count value
    uint64_t count;
    ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &count));
    // (Optional) Convert the count value to time units (seconds)
    
    return (double)count / resolution_hz;
}

void TimerController::set_count(int value){
    gptimer_set_raw_count(gptimer, value);
}
