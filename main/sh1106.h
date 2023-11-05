#include "stdint.h"


/**
 * @brief Configures the SH1106 display
 */
void sh1106Setup();

/**
 * @brief Clears display contents
 */
void sh1106ClearDisplay();
/**
 * @brief Writes a line to the display
 * 
 * @param line line number to write string to
 * @param text string to write to the display (max 16 chars remainder will be cut off)
 */
void sh1106WriteLine(uint8_t line, char* text);

/**
 * @brief Performs a reading from the sensor.
 */
void sh1106Update();

#define SH1106_CONTROL_COMMAND 0x80
#define SH1106_CONTROL_COMMAND_LAST 0x00
#define SH1106_CONTROL_DATA 0x40

#define SH1106_COMMAND_LOWER_COLUMN_ADDRESS 0x00
#define SH1106_COMMAND_HIGHER_COLUMN_ADDRESS 0x10
#define SH1106_COMMAND_DISPLAY_OFF 0xAE
#define SH1106_COMMAND_DISPLAY_ON 0xAF
#define SH1106_COMMAND_SET_ENTIRE_DISPLAY_OFF 0xA4
#define SH1106_COMMAND_SET_ENTIRE_DISPLAY_ON 0xA5
#define SH1106_COMMAND_SET_PAGE_ADDRESS 0xB0