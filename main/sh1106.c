
#include "sh1106.h"

#include <string.h>
#include <sys/param.h>

#include <esp_log.h>

#include "driver/i2c.h"
#include "unistd.h"

#include "co2Sensor.h"
#include "SHTSensor.h"
#include <esp_netif.h>
#include "font.h"

static const char *TAG = "SH1106_display";

static const uint8_t SH1106addr = 0x3C;


void sh1106Setup()
{
    int ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SH1106addr << 1 | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, SH1106_CONTROL_COMMAND, 1);
    i2c_master_write_byte(cmd, SH1106_COMMAND_DISPLAY_ON, 1);
    i2c_master_write_byte(cmd, SH1106_CONTROL_COMMAND, 1);
    i2c_master_write_byte(cmd, SH1106_COMMAND_SET_ENTIRE_DISPLAY_OFF, 1);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "I2C error %d", ret);
        return;
    }

    sh1106ClearDisplay();
}

void sh1106ClearDisplay()
{
    i2c_cmd_handle_t cmd;
    int ret;

    for(int i = 0; i<8; i++)
    {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, SH1106addr << 1 | I2C_MASTER_WRITE, 1);
        i2c_master_write_byte(cmd, SH1106_CONTROL_COMMAND, 1);
        i2c_master_write_byte(cmd, SH1106_COMMAND_SET_PAGE_ADDRESS | i , 1);
        i2c_master_write_byte(cmd, SH1106_CONTROL_COMMAND, 1);
        i2c_master_write_byte(cmd, SH1106_COMMAND_LOWER_COLUMN_ADDRESS | 2, 1);
        i2c_master_write_byte(cmd, SH1106_CONTROL_COMMAND, 1);
        i2c_master_write_byte(cmd, SH1106_COMMAND_HIGHER_COLUMN_ADDRESS, 1);
        i2c_master_write_byte(cmd, SH1106_CONTROL_DATA, 1);

        for(int j = 0; j<128; j++)
        {
            i2c_master_write_byte(cmd, 0x00, 1);
        }

        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
        if (ret != ESP_OK) {
            ESP_LOGI(TAG, "I2C error %d", ret);
            return;
        }
    }
}

void sh1106WriteLine(uint8_t line, char* text)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    int ret;
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SH1106addr << 1 | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, SH1106_CONTROL_COMMAND, 1);
    i2c_master_write_byte(cmd, SH1106_COMMAND_SET_PAGE_ADDRESS | line , 1);
    i2c_master_write_byte(cmd, SH1106_CONTROL_COMMAND, 1);
    i2c_master_write_byte(cmd, SH1106_COMMAND_LOWER_COLUMN_ADDRESS | 2, 1);
    i2c_master_write_byte(cmd, SH1106_CONTROL_COMMAND, 1);
    i2c_master_write_byte(cmd, SH1106_COMMAND_HIGHER_COLUMN_ADDRESS, 1);
    i2c_master_write_byte(cmd, SH1106_CONTROL_DATA, 1);


    for(int z = 0; z< MIN(strlen(text),16); z++)
    {
        uint8_t character = text[z];

        for(int j = 0; j<8; j++)
        {
            uint8_t line = (
                (font8x8_basic[character][0] >> j & 0x01) |
                (font8x8_basic[character][1] >> j & 0x01) <<1 |
                (font8x8_basic[character][2] >> j & 0x01) <<2 |
                (font8x8_basic[character][3] >> j & 0x01) <<3 |
                (font8x8_basic[character][4] >> j & 0x01) <<4 |
                (font8x8_basic[character][5] >> j & 0x01) <<5 |
                (font8x8_basic[character][6] >> j & 0x01) <<6 |
                (font8x8_basic[character][7] >> j & 0x01) <<7
            );

            i2c_master_write_byte(cmd, line, 1);
        }
    }

    ret = i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "I2C error %d", ret);
        return;
    }
}

void sh1106Update()
{
    while(true)
    {

        //get data
        uint16_t co2 = CO2GetValue();
        uint16_t temperature = SHTGetValueTemprature();
        uint16_t humidity = SHTGetValueHumidity();

        extern esp_ip4_addr_t ip_address;

        char buffer[128];

        sprintf(buffer, "co2 = %d ppm     ", co2);
        sh1106WriteLine(1, buffer);

        sprintf(buffer, "temp = %.1fc   ", SHTTemperatureConvertCelcius(temperature));
        sh1106WriteLine(2, buffer);

        sprintf(buffer, "hum = %.1f%%   ", SHTHumidityConvert(humidity));
        sh1106WriteLine(3, buffer);

        sh1106WriteLine(5, "ip addr");
        sprintf(buffer, "%d.%d.%d.%d", IP2STR(&ip_address));
        sh1106WriteLine(6, buffer);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}