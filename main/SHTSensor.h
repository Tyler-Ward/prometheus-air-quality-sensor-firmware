#include "stdint.h"


/**
 * @brief Configures the SHT sensor
 */
void SHTSetup();

/**
 * @brief Performs a reading from the sensor.
 */
int SHTPerformReading();

/**
 * @brief Gets the temprature value from the last reading
 */
uint16_t SHTGetValueTemprature();

/**
 * @brief Gets the humidity value from the last reading
 */
uint16_t SHTGetValueHumidity();

/**
 * @brief Converts a temprature reading into a value in celcius
 */
float SHTTemperatureConvertCelcius(uint16_t reading);

/**
 * @brief Converts a humidity reading into a value in percent
 */
float SHTHumidityConvert(uint16_t reading);
