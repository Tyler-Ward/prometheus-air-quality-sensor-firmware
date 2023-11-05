#include "co2Sensor.h"

#include <string.h>
#include <sys/param.h>

#include <esp_log.h>

#include "driver/uart.h"
#include "unistd.h"

static const char *TAG = "CO2_sensor";

uint8_t co2DataGood; //!< used to mark if CO2 has good data
uint16_t co2Data;   //!< last CO2 reading

void CO2Setup()
{
    co2DataGood = 0;
    co2Data = 0;

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

    if(length != 7)
    {
        ESP_LOGI(TAG, "recieved invalid length : %d", length);
        co2DataGood = 0;
        return(false);
    }

    // Calculate modbus checksum
    uint16_t crc = 0xFFFF;
    for(int pos = 0; pos < 5; pos++)
    {
        crc ^= data[pos];

        for(int bit = 8; bit > 0; bit--)
        {
            if (crc & 0x0001)
            {
                crc = crc >> 1;
                crc ^= 0xA001;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }
    if(
        (data[5] != (crc & 0xff)) | 
        (data[6] != (crc>>8 & 0xff))
    )
    {
        ESP_LOGI(TAG, "invalid checksum");
        co2DataGood = 0;
        return(false);
    }

    co2Data = (256*data[3])+data[4];

    //after poweron data is 0 till first reading
    if(co2Data == 0x0000)
    {
        ESP_LOGI(TAG, "C02 Empty Data");
        co2DataGood = 0;
        return(false);
    }

    co2DataGood = 1;

    return(true);
}

uint16_t CO2GetValue()
{
    return(co2Data);
}

uint8_t CO2DataGood()
{
    return(co2DataGood);
}