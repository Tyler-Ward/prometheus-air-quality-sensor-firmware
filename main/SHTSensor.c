#include "SHTSensor.h"

#include <string.h>
#include <sys/param.h>

#include <esp_log.h>

#include "driver/i2c.h"
#include "unistd.h"

static const char *TAG = "SHT_sensor";

static const uint8_t SHTaddr = 0x44;

uint16_t temperature;
uint16_t humidity;

void SHTSetup()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 32,
        .scl_io_num = 33,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000
    };

    i2c_param_config(0,&conf);

    ESP_ERROR_CHECK(i2c_driver_install(0, conf.mode, 0, 0, 0));

    //reset and put into periodic mode

    int ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHTaddr << 1 | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, 0x30, 1);
    i2c_master_write_byte(cmd, 0xA2, 0);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(0,cmd,1000/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SHT reset error (%x)", ret);
        return;
    }

    vTaskDelay(100 / portTICK_RATE_MS);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHTaddr << 1 | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, 0x22, 1);
    i2c_master_write_byte(cmd, 0x36, 1);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(0,cmd,1000/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SHT setup error");
        return;
    }

    ESP_LOGI(TAG, "SHT sensor configured");

}

int SHTPerformReading()
{
    uint8_t data[10];
    int ret;

    memset(data,0,6);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHTaddr << 1 | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, 0xE0, 1);
    i2c_master_write_byte(cmd, 0x00, 1); 
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHTaddr << 1 | I2C_MASTER_READ, 1);
    i2c_master_read(cmd,data,5,0x00);
    i2c_master_read(cmd,&data[5],1,0x01);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "I2C error %d", ret);
        return false;
    }

    temperature = (data[0]<<8) + data[1];
    humidity = (data[3]<<8) + data[4];

    //ESP_LOGI(TAG, "temp : %.1f", SHTTemperatureConvertCelcius(temperature));
    //ESP_LOGI(TAG, "humid : %.1f", SHTHumidityConvert(humidity));

    return true;
}

uint16_t SHTGetValueTemprature()
{
    return temperature;
}

uint16_t SHTGetValueHumidity()
{
    return humidity;
}

float SHTTemperatureConvertCelcius(uint16_t reading)
{
    float temperature = -45 + (175*((float)reading)/65535.0);
    return(temperature);
}

float SHTHumidityConvert(uint16_t reading)
{
    float humidity = 100*((float)reading/65535.0);
    return(humidity);
}