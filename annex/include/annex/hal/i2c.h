#ifndef ANNEX_HAL_I2C_H
#define ANNEX_HAL_I2C_H

#include <stdint.h>

void annex_hal_i2c_init(void);
int annex_hal_i2c_write(uint8_t address, const uint8_t *data, uint16_t length);
int annex_hal_i2c_read(uint8_t address, uint8_t *data, uint16_t length);

#endif /* ANNEX_HAL_I2C_H */
