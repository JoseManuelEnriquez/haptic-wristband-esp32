#include "./include/battery.hpp"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

#define ADC_UNIT ADC_UNIT_2
#define ADC_ATTEN ADC_ATTEN_DB_0
#define ADC_CHAN ADC_CHANNEL_5 

const static char *TAG = "ADC"; 
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);

Battery::Battery(int pin_gpio):
_pin_gpio(pin_gpio), 
index(0), 
number_reads(0),
calibrated(false), 
buf(std::vector<double>(MAX_VECTOR_LEN)),
adc1_cali_chan0_handle(NULL),
adc1_handle(NULL)
{}
void Battery::init(){
    //-------------ADC1 Init---------------//
    
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHAN, &config));

    //-------------ADC1 Calibration Init---------------//
    calibrated = adc_calibration_init(ADC_UNIT, ADC_CHAN, ADC_ATTEN, &adc1_cali_chan0_handle);
}

esp_err_t Battery::read_adc(double* voltage){
    int raw_data;
    int volt = 0;
    *voltage = 0.0;

    for(int i = 0; i < NUMBER_OF_READS; i++){
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHAN, &raw_data));
        if(calibrated){
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, raw_data, &volt));
            ESP_LOGI(TAG, "Voltaje calibrado: %f", volt);
            *voltage += volt;
        }
    }

    if(number_reads < MAX_VECTOR_LEN)
        number_reads++;

    *voltage = *voltage / number_reads;
    ESP_LOGI(TAG, "Voltaje promedio: %f V", *voltage);

    buf[index] = *voltage;
    index = (index + 1) % MAX_VECTOR_LEN;

    return ESP_OK;
}

esp_err_t Battery::voltToPercentage(float* percentage){
    
    double voltage = 0.0;
    for(int i = 0; i < number_reads; i++) // Hago el promedio segun el numero de lecturas realizadas
        voltage += buf[i];

    voltage = voltage / number_reads; 

    *percentage = (voltage * 1) / 3.3;
    
    return ESP_OK;
}


static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}


