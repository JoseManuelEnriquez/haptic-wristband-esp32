#include "include/HapticController.hpp"

/*******************************************************************************
 * @file    HapticController.cpp
 * @brief   Controlador para el feedback háptico mediante Bluetooth BLE
 * @author  Jose Manuel Enriquez Baena
 * @date    2026
******************************************************************************/

// ! ===========================================================================
// ! SECTION: DEFINICIONES Y MACROS
// ! ===========================================================================

#define BLE_TAG "BLE"
#define GATTS_TAG "GATTS"
#define APP_PROFILE_ID 0
#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)
#define GATTS_SERVICE_UUID 0x00EE
#define HAPTIC_CHAR_UUID 0xEE01
#define GATTS_NUM_HANDLES 4 // ! Alert: Se debe revisar el numero

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

#define STOP_PULSE 0
#define SLOW_PULSE 500
#define FAST_PULSE 50
#define PULSE_DURATION 200
#define NUM_PULSE 3

// ! ===========================================================================
// ! SECTION: DECLARACIONES FUNCIONES STATIC
// ! ===========================================================================

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, 
                                        esp_gatt_if_t gatts_if, 
                                        esp_ble_gatts_cb_param_t *param);

static void gatts_event_handler(esp_gatts_cb_event_t event, 
                                esp_gatt_if_t gatts_if, 
                                esp_ble_gatts_cb_param_t *param);

static void gap_event_handler(esp_gap_ble_cb_event_t event, 
                                esp_ble_gap_cb_param_t *param);


// ! ===========================================================================
// ! SECTION: VARIABLES DE PROFILE
// ! ===========================================================================

// * --- Struct configuracion profile ---
struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

// * --- struct del profile necesario ---
static struct gatts_profile_inst app_profile = {
    .gatts_cb = gatts_profile_event_handler, 
    .gatts_if = ESP_GATT_IF_NONE,
};


// ! ===========================================================================
// ! SECTION: VARIABLES DE AVISOS Y ESCANEO
// ! ===========================================================================

// ? --- array de flags --- */
static uint8_t adv_config_done = 0;

// ? --- definicion UUID del servicio ---
static uint8_t adv_service_uuid128[16] = {
    /* LSB <-----------------------------------------------------------> MSB */
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00
};

// ? --- parametros de avisos ---
static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = ESP_BLE_GAP_ADV_ITVL_MS(20),
    .adv_int_max        = ESP_BLE_GAP_ADV_ITVL_MS(40),
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// ? --- datos de los mensajes de avisos ---
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = ESP_BLE_GAP_CONN_ITVL_MS(7.5), //slave connection min interval
    .max_interval = ESP_BLE_GAP_CONN_ITVL_MS(20), //slave connection max interval
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// ? --- datos de los mensajes de escaneo ---
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    //.min_interval = 0x0006,
    //.max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};


// ! ===========================================================================
// ! SECTION: IMPLEMENTACIÓN DE LA CLASE HAPTIC
// ! ===========================================================================

typedef struct {
    uint32_t tiempo_on_ms;
    uint32_t tiempo_off_ms;
} PWMPulseConfig_t;

PWMPulseConfig_t pwmConfig;
TaskHandle_t xPWMTaskHandle = NULL;

void vPWM_Task(void *pvParameters) {
    PWMPulseConfig_t *config = (PWMPulseConfig_t *)pvParameters;

    while (1) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4096);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(pdMS_TO_TICKS(config->tiempo_on_ms));

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(pdMS_TO_TICKS(config->tiempo_off_ms));
    }
}

static HapticController* haptic_controller = nullptr;

// ? --- Constructor ---
HapticController::HapticController(int _pin_gpio): pin_gpio(_pin_gpio),
                                                   new_write(false),
                                                   last_value(0)
{}

// ? --- Singleton ---
HapticController* HapticController::get_instance(int pin_gpio){
    if(!haptic_controller){
        haptic_controller = new HapticController(pin_gpio);
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
    return haptic_controller;
}

// ? --- Inicialización de Hardware y Stack BLE ---
void HapticController::init(){
    esp_err_t ret;

    // 1. Inicializar NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Inicializar Host Controller
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // 3. Inicializar software que implementa protocolos como L2CAP, ATT...
    esp_bluedroid_config_t cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    ret = esp_bluedroid_init_with_cfg(&cfg);
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // 4. Registrar ciclo de vida del GATT
    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
        return;
    }

    // 5. Registrar ciclo de vida del GAP
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gap register error, error code = %x", ret);
        return;
    }

    // 6. Registrar profile
    ret = esp_ble_gatts_app_register(APP_PROFILE_ID);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    // Configuramos el tamaño maximo de los paquetes ble
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret){
        ESP_LOGE(GATTS_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
}

void HapticController::onWrite(uint8_t value){
    last_value = value;
    new_write = true;
}

uint8_t* HapticController::hay_escritura(){
    if(new_write){
        new_write = false;
        return &last_value;
    }
    return nullptr;
}

void HapticController::emitir_vibracion(int value){
    uint32_t time_pulse = SLOW_PULSE;
    switch (value)
    {
        case 0:
            time_pulse = STOP_PULSE;
            break;
        case 1:
            time_pulse = FAST_PULSE;
            break;
        case 2:
            time_pulse = SLOW_PULSE;
        default:
            break;
    }
    if(time_pulse == STOP_PULSE){
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
        if (xPWMTaskHandle != NULL) {
            vTaskDelete(xPWMTaskHandle);
            xPWMTaskHandle = NULL;
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        }
    }else{
        if (xPWMTaskHandle != NULL) {
            vTaskDelete(xPWMTaskHandle);
            xPWMTaskHandle = NULL;
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        }

        pwmConfig.tiempo_on_ms  = PULSE_DURATION;
        pwmConfig.tiempo_off_ms = time_pulse;

        xTaskCreate(vPWM_Task, "PWM_Task", 2048, &pwmConfig, 5, &xPWMTaskHandle);
    }
}

void HapticController::set_intensity(int value){
    ESP_ERROR_CHECK(ledc_set_duty_with_hpoint(LEDC_MODE, LEDC_CHANNEL, value, value));
}

// ! ===========================================================================
// ! SECTION: IMPLEMENTACIÓN FUNCIONES STATIC
// ! ===========================================================================
 
// ? --- Manejador de eventos del profile ---
static void gatts_profile_event_handler(esp_gatts_cb_event_t event, 
                                        esp_gatt_if_t gatts_if, 
                                        esp_ble_gatts_cb_param_t *param){
    switch(event) 
    {
        case ESP_GATTS_REG_EVT: { // Se configura los datos de escaneo y avisos
            ESP_LOGI(GATTS_TAG, "GATT server register, status %d, app_id %d, gatts_if %d", param->reg.status, param->reg.app_id, gatts_if);
            // * Creacion y configuracion del servicio
            app_profile.service_id.is_primary = true;
            app_profile.service_id.id.inst_id = 0x00;
            app_profile.service_id.id.uuid.len = ESP_UUID_LEN_16;
            app_profile.service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID;

            esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name("pulsera-vibracion");
            if (set_dev_name_ret){
                ESP_LOGE(GATTS_TAG, "set device name failed, error code = %x", set_dev_name_ret);
            }

            //config adv data
            esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
            if (ret){
                ESP_LOGE(GATTS_TAG, "config adv data failed, error code = %x", ret);
            }
            adv_config_done |= adv_config_flag; // Indicamos que se ha configurado en el array de flags

            //config scan response data
            ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
            if (ret){
                ESP_LOGE(GATTS_TAG, "config scan response data failed, error code = %x", ret);
            }
            adv_config_done |= scan_rsp_config_flag; // Indicamos que se ha configurado en el array de flags

            esp_ble_gatts_create_service(gatts_if, &app_profile.service_id, GATTS_NUM_HANDLES); // ! Revisar NUM_HANDLES
            break;
        }
        case ESP_GATTS_CONNECT_EVT:
            app_profile.conn_id = param->connect.conn_id;
            ESP_LOGI(GATTS_TAG, "Cliente conectado, conn_id %d", param->connect.conn_id);
            break;

        case ESP_GATTS_DISCONNECT_EVT:
                ESP_LOGI(GATTS_TAG, "Cliente desconectado");
                esp_ble_gap_start_advertising(&adv_params);  // vuelve a anunciar
                break;

        case ESP_GATTS_CREATE_EVT: { 
            ESP_LOGI(GATTS_TAG, "Service create, status %d,  service_handle %d", param->create.status, param->create.service_handle);
            /*
            Estamos creando la caracteristica con id HAPTIC_CHAR_UUID -> 0xEE01
            service_handle se crea cuando creamos el servicio en el evento ESP_GATTS_REG_EVT,
            solo tenemos que comenzar el servicio y agregar los permisos (write en nuestro caso)
            para permitir hacer acciones con el.
            */
            app_profile.service_handle = param->create.service_handle;
            app_profile.char_uuid.len = ESP_UUID_LEN_16;
            app_profile.char_uuid.uuid.uuid16 = HAPTIC_CHAR_UUID;

            esp_ble_gatts_start_service(app_profile.service_handle);

            // * Configuramos solo WRITE
            app_profile.perm = ESP_GATT_PERM_WRITE;
            app_profile.property = ESP_GATT_CHAR_PROP_BIT_WRITE;
            esp_err_t add_char_ret = esp_ble_gatts_add_char(app_profile.service_handle, 
                                                            &app_profile.char_uuid,
                                                            app_profile.perm,
                                                            app_profile.property,
                                                            NULL, NULL);
            if (add_char_ret){
                ESP_LOGE(GATTS_TAG, "add char failed, error code =%x",add_char_ret);
            }
            break;
        }
        case ESP_GATTS_WRITE_EVT: {

            if (!param->write.is_prep) {  // write completo, no preparatorio
                uint8_t speed = param->write.value[0];
                ESP_LOGI(GATTS_TAG, "Velocidad recibida: %d", speed);
                if(haptic_controller)
                    haptic_controller->onWrite(speed);
            }
            break;
        }
        default:
            ESP_LOGW(GATTS_TAG, "Evento desconocido");
            break;
    }
}

// ? --- Manejador eventos GATTS y configuracion del profile ---
// Esta funcion se registra en el init de HapticController y es para asignar a cada profile
// un manejador de evento para sus conexiones y ademas de un ID.
// No solo sirve para registrar su callback. Como todo evento que ocurra en el GATT pasa por esta
// funcion, tiene que llamar a la callback del id/profile correspondiente.
static void gatts_event_handler(esp_gatts_cb_event_t event, 
                                esp_gatt_if_t gatts_if, 
                                esp_ble_gatts_cb_param_t *param)
{
    /* Cuando ya hemos registrado la funcion y genera el evento, lo usamos para inicialziar
    la interfaz del app_profile.*/
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            app_profile.gatts_if = gatts_if;
        } else {
            ESP_LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    /* Comprobamos que el evento capturado tiene el mismo numero de interfaz que el 
    app_profile y llamamos a la callback correspondiente de ese perfil */
    if( (gatts_if == ESP_GATT_IF_NONE) || (gatts_if == app_profile.gatts_if) ){
        if(app_profile.gatts_cb){
            app_profile.gatts_cb(event, gatts_if, param);
        }
    }
}

/// ? --- Manejador de eventos GAP para configurarlo como peripheral ---
static void gap_event_handler(esp_gap_ble_cb_event_t event, 
                            esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT: // Salta cuando se ha configurado los datos de avisos
        adv_config_done &= (~adv_config_flag); // Limpiamos flag
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT: // Salta cuando se ha configurado los datos de escaneo
        adv_config_done &= (~scan_rsp_config_flag); // Limpiamos flag
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising start failed, status %d", param->adv_start_cmpl.status);
            break;
        }
        ESP_LOGI(GATTS_TAG, "Advertising start successfully");
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising stop failed, status %d", param->adv_stop_cmpl.status);
            break;
        }
        ESP_LOGI(GATTS_TAG, "Advertising stop successfully");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
         ESP_LOGI(GATTS_TAG, "Connection params update, status %d, conn_int %d, latency %d, timeout %d",
                  param->update_conn_params.status,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
        break;
    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
        ESP_LOGI(GATTS_TAG, "Packet length update, status %d, rx %d, tx %d",
                  param->pkt_data_length_cmpl.status,
                  param->pkt_data_length_cmpl.params.rx_len,
                  param->pkt_data_length_cmpl.params.tx_len);
        break;
    default:
        break;
    }
}
