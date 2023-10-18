#include "co2Sensor.h"

#include <string.h>
#include <sys/param.h>

#include <esp_log.h>

#include "driver/uart.h"
#include "unistd.h"

static const char *TAG = "CO2_sensor";

void CO2Setup()
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));

    // Set UART pins(TX: IO4, RX: IO5, RTS: IO18, CTS: IO19)
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, 4, 36, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Setup UART buffered IO with event queue
    const int uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;
    // Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, uart_buffer_size, \
                                            uart_buffer_size, 10, &uart_queue, 0));
}

uint16_t CO2GetValue()
{
    // Write data to UART.
    char* test_str = "\xFE\x44\x00\x08\x02\x9F\x25";
    uart_write_bytes(UART_NUM_1, (const char*)test_str, 7);

    sleep(1);

    // Read data from UART.
    uint8_t data[128];
    int length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_1, (size_t*)&length));
    length = uart_read_bytes(UART_NUM_1, data, length, 100);

    ESP_LOGI(TAG, "recieved : %d", length);

    uint16_t co2data = (256*data[3])+data[4];
    ESP_LOGI(TAG, "co2 : %d", co2data);

    return(co2data);
}