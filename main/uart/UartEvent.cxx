#include "esp_log.h"
#include "hal/gpio_types.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "UartEvent.hpp"

void UartEvent::taskMain(void *)
{
    init();

    uart_event_t event;

    while (true)
    {
        // Waiting for UART event.
        if (xQueueReceive(eventQueue, &event, portMAX_DELAY))
        {
            ESP_LOGD(PrintTag, "uart[%d] event:", UartNumber);

            switch (event.type)
            {
            // Event of UART receving data
            case UART_DATA:
                ESP_LOGD(PrintTag, "read %d bytes", event.size);
                uart_read_bytes(UartNumber, rawDataBuffer.data(), event.size, portMAX_DELAY);
                // Todo load into rxQueue
                break;

            // Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                ESP_LOGE(PrintTag, "hw fifo overflow");
                uart_flush_input(UartNumber);
                break;

            // Event of UART ring buffer full
            case UART_BUFFER_FULL:
                ESP_LOGE(PrintTag, "ring buffer is full!");
                uart_flush_input(UartNumber);
                break;

            case UART_PARITY_ERR:
                ESP_LOGE(PrintTag, "uart parity error");
                break;

            // Event of UART frame error
            case UART_FRAME_ERR:
                ESP_LOGE(PrintTag, "uart frame error");
                break;

            default:
                break;
            }
        }
    }
}

void UartEvent::init()
{

    uart_config_t uartConfig = {.baud_rate = 115200,
                                .data_bits = UART_DATA_8_BITS,
                                .parity = UART_PARITY_DISABLE,
                                .stop_bits = UART_STOP_BITS_1,
                                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    ESP_ERROR_CHECK(uart_param_config(UartNumber, &uartConfig));
    ESP_ERROR_CHECK(uart_set_pin(UartNumber, gpio_num_t::GPIO_NUM_12, gpio_num_t::GPIO_NUM_14,
                                 gpio_num_t::GPIO_NUM_NC, gpio_num_t::GPIO_NUM_NC));

    // install UART driver, and get the queue.
    ESP_ERROR_CHECK(uart_driver_install(UartNumber, BufferSize, BufferSize, 64, &eventQueue, 0));
}