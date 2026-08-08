#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "soc/adc_channel.h"
#include "esp_log.h"
#include "sdkconfig.h"

const gpio_num_t GPIO = GPIO_NUM_4; // ADC2 CH0
const adc_channel_t CHANNEL = ADC_CHANNEL_0; // GPIO 4
const adc_unit_t UNIT = ADC_UNIT_2;
const adc_bitwidth_t WIDTH = ADC_BITWIDTH_12; // 12 bits resolution = 4096 (0 to 4095)
const adc_atten_t ATTEN = ADC_ATTEN_DB_12; // measurable input voltage range = 150mV ~ 2450mV

static int adc_reading = 0; // variable to store the ADC reading

// // ADC pattern configuration
// adc_digi_pattern_config_t adc_pattern = {
//     .atten = ATTEN,
//     .channel = CHANNEL,
//     .unit = UNIT,
//     .bit_width = WIDTH,
// };

// // convert GPIO number to ADC channel
// adc_continuous_io_to_channel()

// // convert ADC channel to GPIO number
// adc_continuous_channel_to_io();

// ============= ONESHOT MODE =============

static adc_oneshot_unit_handle_t adc_handle;

// ADC configuration for OneShot mode
static void adc_config_oneshot () {

    // Initialize ADC unit on OneShot mode
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    // Configure ADC channel
    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = WIDTH,
        .atten = ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, CHANNEL, &channel_config));
}



// ============= CONTINUOUS MODE =============

// ADC configuration for Continuous mode
static void adc_config_continuous () {
    ;;
}



static const char *TAG = "ADC2";

void app_main (void) {

    adc_config_oneshot();
    adc_config_continuous();

    while (1) {
        
        // Read ADC value
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, CHANNEL, &adc_reading));
        ESP_LOGI(TAG, "ADC value = %d", adc_reading);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }

}
