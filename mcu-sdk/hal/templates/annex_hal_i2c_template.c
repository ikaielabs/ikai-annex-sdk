#include <stdint.h>

/*
 * I2C HAL template for porting annex SDK to a new platform.
 * Copy this file and replace TODO stubs with MCU-specific logic.
 */

void annex_hal_i2c_init(void)
{
    /* TODO: initialize I2C controller and configure timing */
}

int annex_hal_i2c_write(uint8_t address, const uint8_t *data, uint16_t length)
{
    /* TODO: write 'length' bytes to the I2C device at 'address' */
    (void)address;
    (void)data;
    (void)length;
    return 0;
}

int annex_hal_i2c_read(uint8_t address, uint8_t *data, uint16_t length)
{
    /* TODO: read 'length' bytes from the I2C device at 'address' */
    (void)address;
    (void)data;
    (void)length;
    return 0;
}
