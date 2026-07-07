#include "annex_log.h"
#include <string.h>

static UART_HandleTypeDef *g_uart = NULL;

void annex_log_init(UART_HandleTypeDef *huart)
{
    g_uart = huart;
}

void annex_log_printf(const char *fmt, ...)
{
    char buffer[256];

    va_list args;

    if (g_uart == NULL)
    {
        return;
    }

    va_start(args, fmt);

    vsnprintf(buffer,
              sizeof(buffer),
              fmt,
              args);

    va_end(args);

    HAL_UART_Transmit(
        g_uart,
        (uint8_t *)buffer,
        strlen(buffer),
        HAL_MAX_DELAY);
}

/*------------------------------------------------------------------
 * printf() Redirection
 *-----------------------------------------------------------------*/

#ifdef __GNUC__

int __io_putchar(int ch)

#else

int fputc(int ch, FILE *f)

#endif
{
    if (g_uart != NULL)
    {
        HAL_UART_Transmit(
            g_uart,
            (uint8_t *)&ch,
            1,
            HAL_MAX_DELAY);
    }

    return ch;
}
