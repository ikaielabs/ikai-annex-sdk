#ifndef ANNEX_HAL_UART_H
#define ANNEX_HAL_UART_H

#include <stdint.h>

void annex_hal_uart_init(void);
int annex_hal_uart_send(const uint8_t *data, uint16_t length);
int annex_hal_uart_receive(uint8_t *data, uint16_t length);

#endif /* ANNEX_HAL_UART_H */
