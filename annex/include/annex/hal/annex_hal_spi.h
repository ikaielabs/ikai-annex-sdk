#ifndef ANNEX_HAL_SPI_H
#define ANNEX_HAL_SPI_H

#include <stdint.h>

void annex_hal_spi_init(void);
int annex_hal_spi_transfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length);

#endif /* ANNEX_HAL_SPI_H */
