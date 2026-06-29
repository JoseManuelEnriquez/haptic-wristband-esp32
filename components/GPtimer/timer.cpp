#include "./include/timer.hpp"

TimerController::TimerController(): timer(NULL){
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
        .direction = GPTIMER_COUNT_UP,      // Counting direction is up
        .resolution_hz = 1000000,   // Resolution is 1 Hz, i.e., 1 tick equals 1 second
    };
    // Create a timer instance
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer));
    // Enable the timer
    ESP_ERROR_CHECK(gptimer_enable(timer));
}

void TimerController::start(){
    gptimer_start(timer);
}

void TimerController::stop(){
    gptimer_stop(timer);
}

double TimerController::get_time_seconds(){
    // Check the timer's resolution
    uint32_t resolution_hz;
    ESP_ERROR_CHECK(gptimer_get_resolution(timer, &resolution_hz));
    
    // Read the current count value
    uint64_t count;
    ESP_ERROR_CHECK(gptimer_get_raw_count(timer, &count));
    // (Optional) Convert the count value to time units (seconds)
    
    return (double)count / resolution_hz;
}

void TimerController::set_count(int value){
    gptimer_set_raw_count(timer, value);
}
