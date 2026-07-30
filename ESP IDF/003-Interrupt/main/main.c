#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/tasks.h"
#include "driver/gpio.h"

const gpio_num_t button = 5;

volatile uint8_t number;

const uint64_t debounce_time = 200; // Debounce time in milliseconds
volatile uint64_t last_interrupt_time = 0;

static QueueHandle_t button_queue;

static void IRAM_ATTR button_isr (void *arg) {

    uint64_t current_time = esp_timer_get_time(); // get time in microseconds

    if (current_time - last_interrupt_time > debounce_time * 1000) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        /*
            xQueueSendFromISR() will set *pxHigherPriorityTaskWoken to pdTRUE if sending to 
            the queue caused a task to unblock, and the unblocked task has a priority higher 
            than the currently running task.
        */
        xQueueSendFromISR(button_queue, arg, &xHigherPriorityTaskWoken); 
        last_interrupt_time = current_time;
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
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

    // Create a queue to handle button events
    button_queue = xQueueCreate(10, sizeof(uint8_t));
}

static void config_timer () {

    timer_config_t timer_conf = {
        .alarm_en = TIEMR_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .intr_type = ,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_EN,
        .divider = 
    };

}

void app_main(void) {

    config_gpios();

    while (1) {
        if (xQueueReceive(button_queue, &number, portMAX_DELAY)) {
            printf("Button pressed!\nYou got the number %d\n", number);
        }
    }

}
