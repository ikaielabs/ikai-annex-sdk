#ifndef ANNEX_UART_IO_H
#define ANNEX_UART_IO_H

#include<stdio.h>
#include<avr/io.h>

#ifdef __cplusplus
extern "C" {
#endif

void annex_uart_io_init(uint32_t input_clk_hz, uint32_t baud, FILE *uart_stream);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ANNEX_UART_IO_H */