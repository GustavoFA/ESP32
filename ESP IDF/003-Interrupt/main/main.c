#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/tasks.h"
#include "driver/gpio.h"
// #include "driver/timer.h"
#include "esp_timer.h"
#include "esp_random.h"

const gpio_num_t button = 5;

// volatile uint8_t number;

const uint64_t debounce_time = 200; // Debounce time in milliseconds
volatile uint64_t last_interrupt_time = 0;

static QueueHandle_t queue;

typedef enum {
    EVENT_BUTTON,
    EVENT_TIMER
} event_type_t;

typedef struct {
    event_type_t type;
    uint8_t value;
} queue_event_t;

// ================== Button ====================

// 
// static QueueHandle_t button_queue;

static void IRAM_ATTR button_isr (void *arg) {

    uint64_t current_time = esp_timer_get_time(); // get time in microseconds

    if (current_time - last_interrupt_time > debounce_time * 1000) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        /*
            xQueueSendFromISR() will set *pxHigherPriorityTaskWoken to pdTRUE if sending to 
            the queue caused a task to unblock, and the unblocked task has a priority higher 
            than the currently running task.
        */
        queue_event_t event = {
            .type = EVENT_BUTTON,
            .value = esp_random() % 100 
        };
        xQueueSendFromISR(queue, &event, &xHigherPriorityTaskWoken); 
        last_interrupt_time = current_time;
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

static void config_gpios () {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << button),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE
    };
    gpio_config(&io_conf);

    // ISR configuration
    gpio_install_isr_service(0);

    // Add ISR handler
    gpio_isr_handler_add(button, button_isr, NULL); 

    // Create a queue to handle button events
    // button_queue = xQueueCreate(10, sizeof(uint8_t));
}

// ================== Timer ====================

const uint32_t DIVIDER = 80; // 80 MHz / 80 = 1 MHz (1 tick = 1 microsecond)
const uint64_t TIMER_INTERVAL = 2 * 1000000ULL; // 2 seconds

static void IRAM_ATTR timer_isr (void *arg) {

    // Clear the interrupt - it's important so it doesn't keep firing
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);

    // Even with auto-reload, we must manually re-enable the alarm inside the ISR each time
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    queue_event_t event = {
        .type = EVENT_TIMER,
        .value = 100
    };
    xQueueSendFromISR(queue, &event, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

static void config_timer () {

    timer_config_t timer_conf = {
        .alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_EN,
        .divider = DIVIDER
    };

    // Initialize timer
    timer_init(TIMER_GROUP_0, TIMER_0, &timer_conf);

    // Set timer counter value to 0 
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);

    // Set alarm value
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL);

    // Enable timer interrupt
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);

    // Register ISR handler
    timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);

    // Start timer
    timer_start(TIMER_GROUP_0, TIMER_0);

}

// =============================================


void app_main(void) {

    queue = xQueueCreate(10, sizeof(queue_event_t));

    config_gpios();
    config_timer();

    queue_event_t evt;

    while (1) {
        if (xQueueReceive(queue, &evt, portMAX_DELAY)) {
            if (evt.type == EVENT_BUTTON) {
                printf("Button pressed!\nYou got the number %d\n", evt.value);
            }
            else if (evt.type == EVENT_TIMER) {
                printf("Timer event!\nYou got the number %d\n", evt.value);
            }
        }
    }

}
