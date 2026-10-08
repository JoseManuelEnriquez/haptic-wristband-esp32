#ifndef BATTERY_HPP
#define BATTERY_HPP

#include <vector>
#include "esp_adc/adc_oneshot.h"

#define MAX_VECTOR_LEN 5
#define NUMBER_OF_READS 15

class Battery{
    public:
        Battery(int pin_gpio);
        void init();
        esp_err_t read_adc(double* voltage);
        esp_err_t voltToPercentage(float* percentage);
    private:
        int _pin_gpio;
        uint8_t index;
        uint8_t number_reads;
        bool calibrated;
        std::vector<double> buf;
        adc_cali_handle_t adc1_cali_chan0_handle;
        adc_oneshot_unit_handle_t adc1_handle;
};

#endif