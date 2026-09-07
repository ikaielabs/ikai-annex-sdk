#ifndef INX_HAL_I2C_H
#define INX_HAL_I2C_H

#include <stdint.h>

void inx_hal_i2c_init(void);
int inx_hal_i2c_write(uint8_t address, const uint8_t *data, uint16_t length);
int inx_hal_i2c_read(uint8_t address, uint8_t *data, uint16_t length);

#endif /* INX_HAL_I2C_H */
