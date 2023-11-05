#include "co2Sensor.h"

#include <string.h>
#include <sys/param.h>

#include <esp_log.h>

#include "driver/uart.h"
#include "unistd.h"

static const char *TAG = "CO2_sensor";

uint16_t co2data;

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

int CO2PerformReading()
{
    // Send a modbus packet to read register 3 containing the CO2 reading for the CO2 Sensor
    char command[7];
    command[0] = 0xFE;  //Address (all sensors)
    command[1] = 0x04;  //Functon code (Read Input registers)
    command[2] = 0x00;  //Addr High (0x0003)
    command[3] = 0x03;  //Addr Low (0x0003)
    command[4] = 0x00;  //Qty High (0x0001)
    command[5] = 0x01;  //Qty High (0x0001)
    command[6] = 0xD5; // CRC_low
    command[7] = 0xC5; // CRC_high
    uart_write_bytes(UART_NUM_1, (const char*)command, 8);

    vTaskDelay(1000 / portTICK_RATE_MS);

    // Read and check response.
    uint8_t data[128];
    int length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_1, (size_t*)&length));
    length = uart_read_bytes(UART_NUM_1, data, length, 100);

    //TODO check that message is not an error code and checksum matches

    ESP_LOGI(TAG, "recieved : %d", length);

    co2data = (256*data[3])+data[4];
    ESP_LOGI(TAG, "co2 : %d", co2data);

    return(true);
}

uint16_t CO2GetValue()
{
    return(co2data);
}