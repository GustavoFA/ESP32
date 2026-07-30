#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/tasks.h"
#include "driver/gpio.h"

const gpio_num_t button = 5;

volatile uint8_t number;

static void IRAM_ATTR button_isr (void *arg) {

}

static void config_gpios () {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << button),ç
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE
    };
    gpio_config(&io_conf);

    // ISR configuration
    gpio_install_isr_service(0);

    // Add ISR handler
    gpio_isr_handler_add(button, button_isr, (void*) number);
}

void app_main(void)
{

}
