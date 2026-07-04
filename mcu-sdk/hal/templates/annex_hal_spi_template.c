#include <stdint.h>

/*
 * SPI HAL template for porting annex SDK to a new platform.
 * Copy this file and replace TODO stubs with MCU-specific logic.
 */

void annex_hal_spi_init(void)
{
    /* TODO: initialize SPI controller and configure clock/polarity */
}

int annex_hal_spi_transfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length)
{
    /* TODO: perform a full-duplex SPI transfer of 'length' bytes */
    (void)tx_data;
    (void)rx_data;
    (void)length;
    return 0;
}
