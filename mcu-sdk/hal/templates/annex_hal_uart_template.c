#include <stdint.h>

/*
 * UART HAL template for porting annex SDK to a new platform.
 * Copy this file and replace TODO stubs with MCU-specific logic.
 */

void annex_hal_uart_init(void)
{
    /* TODO: initialize UART controller and baud rate settings */
}

int annex_hal_uart_send(const uint8_t *data, uint16_t length)
{
    /* TODO: send 'length' bytes over UART */
    (void)data;
    (void)length;
    return 0;
}

int annex_hal_uart_receive(uint8_t *data, uint16_t length)
{
    /* TODO: receive up to 'length' bytes from UART */
    (void)data;
    (void)length;
    return 0;
}
