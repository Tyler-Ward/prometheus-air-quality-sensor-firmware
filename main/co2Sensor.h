#include "stdint.h"


/**
 * @brief Configures the CO2 sensor
 */
void CO2Setup();

/**
 * @brief Performs a reading from the sensor.
 */
int CO2PerformReading();

/**
 * @brief Gets the CO2 value from the last reading
 */
uint16_t CO2GetValue();

/**
 * @brief Returns 1 if the last CO2 reading was good
 */
uint8_t CO2DataGood();