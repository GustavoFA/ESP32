#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h" // includes the project's configuration file
#include "esp_err.h" // includes the ESP-IDF error handling functions

const gpio_num_t led_blink = GPIO_NUM_2; // GPIO pin for the onboard LED

const uint32_t blink_delay_ms = 1000; // Delay in milliseconds for LED blink

// Configure the GPIO pin for the LED
static void config_gpio () {
    gpio_reset_pin(led_blink);
    gpio_set_direction(led_blink, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
    config_gpio();

    while (1) {
        printf("LED ON\n");
        gpio_set_level(led_blink, 1); // Turn the LED on
        vTaskDelay(blink_delay_ms / portTICK_PERIOD_MS); // Wait for x second
        printf("LED OFF\n");
        gpio_set_level(led_blink, 0); // Turn the LED off
        vTaskDelay(blink_delay_ms / portTICK_PERIOD_MS); // Wait for x second
    }
}
