#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

const gpio_num_t led_gpio = GPIO_NUM_2; // GPIO pin for the LED
const char *message = "Hello my dear friend!"; // Message to be printed
volatile uint32_t led_state = 0; // Variable to hold the LED state

// Task time delay in milliseconds
const uint32_t task1_delay_ms = 1000; 
const uint32_t task2_delay_ms = 2000;

// Task 1 - Prints a message to the console
void message_print (void *pvParameters) {
    while (1) {
        printf("%s\n", message);
        vTaskDelay(task1_delay_ms / portTICK_PERIOD_MS); // Delay for task1_delay_ms milliseconds
    }
}

// Task 2 - Toggles the LED state
void led_toggle (void *pvParameters) {
    while (1) {
        led_state = !led_state; // Toggle the LED state
        gpio_set_level(led_gpio, led_state); // Set the LED GPIO pin to the new state
        vTaskDelay(task2_delay_ms / portTICK_PERIOD_MS); // Delay for task2_delay_ms milliseconds
    }
}

static void initialize_gpio () {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << led_gpio), // Bit mask for the LED GPIO pin
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE, // Disable pull-up resistor
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Disable pull-down resistor
        .intr_type = GPIO_INTR_DISABLE // Disable interrupts
    };
    gpio_config(&io_conf); // Configure the GPIO pin
}

void app_main(void)
{
    initialize_gpio(); // Initialize the GPIO pin for the LED

    // Create the message printing task
    xTaskCreate(message_print, "Message Print Task", 2048, NULL, 1, NULL);

    // Create the LED toggling task
    xTaskCreate(led_toggle, "LED Toggle Task", 2048, NULL, 1, NULL);
}
