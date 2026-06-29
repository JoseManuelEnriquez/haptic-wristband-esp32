#ifndef TIMER_HPP
#define TIMER_HPP
#include "hal/timer_types.h"
#include "driver/gptimer.h"

class TimerController{
    public:
        TimerController();
        void start();
        void stop();
        double get_time_seconds();
        void set_count(int value);
    private:
        gptimer_handle_t timer;
};

#endif