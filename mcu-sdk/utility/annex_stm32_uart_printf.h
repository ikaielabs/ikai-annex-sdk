#ifndef ANNEX_LOG_H
#define ANNEX_LOG_H

#include "main.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void annex_log_init(UART_HandleTypeDef *huart);

void annex_log_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* ANNEX_DEBUG_H */
