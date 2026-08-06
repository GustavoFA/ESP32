#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_timer.h"
#include "esp_random.h"

const gpio_num_t button = 5;

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
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE
    };
    gpio_config(&io_conf);

    // ISR configuration
    gpio_install_isr_service(0);

    // Add ISR handler
    gpio_isr_handler_add(button, button_isr, NULL); 
}

// ================== Timer ====================

static gptimer_handle_t timer_handle = NULL;

static bool IRAM_ATTR timer_isr (gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    queue_event_t event = {
        .type = EVENT_TIMER,
        .value = 100
    };
    xQueueSendFromISR(queue, &event, &xHigherPriorityTaskWoken);

    return xHigherPriorityTaskWoken == pdTRUE; // tells the driver whether to yield or not after the ISR
}

static void config_timer () {

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000000, // 1 MHz -> 1 tick = 1 microsecond
    };
    gptimer_new_timer(&timer_config, &timer_handle);

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_isr,
    };
    gptimer_register_event_callbacks(timer_handle, &cbs, NULL);

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,
        .alarm_count = 2 * 1000000, // 2 seconds
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(timer_handle, &alarm_config);

    gptimer_enable(timer_handle);
    gptimer_start(timer_handle);
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
