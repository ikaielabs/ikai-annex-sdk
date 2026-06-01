#ifndef ANNEX_HAL_GPIO_H
#define ANNEX_HAL_GPIO_H

#include <stdint.h>

typedef enum {
    ANNEX_GPIO_LOW = 0,
    ANNEX_GPIO_HIGH = 1,
} annex_gpio_level_t;

void annex_hal_gpio_init(void);
void annex_hal_gpio_write(uint8_t pin, annex_gpio_level_t level);
uint8_t annex_hal_gpio_read(uint8_t pin);

#endif /* ANNEX_HAL_GPIO_H */
