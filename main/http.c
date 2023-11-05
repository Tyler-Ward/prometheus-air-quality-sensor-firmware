#include <string.h>
#include <sys/param.h>

#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_spiffs.h>

#include "http.h"
#include "co2Sensor.h"
#include "SHTSensor.h"

static const char *TAG = "co2_http";

static esp_err_t metrics_get_handler(httpd_req_t *req)
{
    //get data
    uint16_t co2 = CO2GetValue();
    uint16_t temperature = SHTGetValueTemprature();
    uint16_t humidity = SHTGetValueHumidity();

    httpd_resp_set_type(req,"text/plain");

    char buffer[128];

    //dont include CO2 data if sensor doesent have good data
    if(CO2DataGood())
    {
        httpd_resp_sendstr_chunk(req, "# HELP sensor_co2_level_ppm CO2 value, in ppm\n");
        httpd_resp_sendstr_chunk(req, "# TYPE sensor_co2_level_ppm gauge\n");

        sprintf(buffer, "sensor_co2_level_ppm{id=\"%s\"} %d\n","1", co2);
        httpd_resp_sendstr_chunk(req,buffer);
    }

    httpd_resp_sendstr_chunk(req, "# HELP sensor_temperature_celsius temperature in Celsius\n");
    httpd_resp_sendstr_chunk(req, "# TYPE sensor_temperature_celsius gauge\n");

    sprintf(buffer, "sensor_temperature_celsius{id=\"%s\"} %.1f\n","1", SHTTemperatureConvertCelcius(temperature));
    httpd_resp_sendstr_chunk(req,buffer);


    httpd_resp_sendstr_chunk(req, "# HELP sensor_humidity_percent relative humidity in percent\n");
    httpd_resp_sendstr_chunk(req, "# TYPE sensor_humidity_percent gauge\n");

    sprintf(buffer, "sensor_humidity_percent{id=\"%s\"} %.1f\n","1", SHTHumidityConvert(humidity));
    httpd_resp_sendstr_chunk(req,buffer);


    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

static const httpd_uri_t page_metrics = {
    .uri = "/metrics",
    .method = HTTP_GET,
    .handler = metrics_get_handler,
    .user_ctx  = NULL
};

static esp_err_t index_get_handler(httpd_req_t *req)
{

    httpd_resp_sendstr_chunk(req, "<html><body>\n");
    httpd_resp_sendstr_chunk(req, "<h1>CO2 monitor interface</h1>\n");
    httpd_resp_sendstr_chunk(req, "<a href=\"/metrics\">metrics</a>\n");
    httpd_resp_sendstr_chunk(req, "</html></body>\n");
    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

static const httpd_uri_t page_index = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_get_handler,
    .user_ctx  = NULL
};

void httpSetup()
{
    ESP_LOGI(TAG,"WEBSERVER THREAD STARTED");

    static httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    if(httpd_start(&server,&config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server,&page_metrics);
        httpd_register_uri_handler(server,&page_index);
    }
}
