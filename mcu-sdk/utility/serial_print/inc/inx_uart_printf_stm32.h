#ifndef INX_UART_PRINTF_STM32_H
#define INX_UART_PRINTF_STM32_H

#include "main.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void inx_uart_printf_init(UART_HandleTypeDef *huart);

void inx_uart_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* INX_UART_PRINTF_STM32_H */
