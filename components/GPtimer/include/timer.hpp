#ifndef TIMER_HPP
#define TIMER_HPP
#include "hal/timer_types.h"

class TimerController{
    public:
        TimerController();
        void start();
        void stop();
        void get_time_seconds();
        void set_count(int value);
    private:
        gptimer_handler_t gptimer;
}

#endif