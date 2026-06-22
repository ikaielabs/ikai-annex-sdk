/**
 * @file annex_hal_gpio_atmega328p.h
 * @brief ATmega328p port identifiers for use with annex_hal_gpio.h.
 *
 * Application code includes this alongside the generic annex_hal_gpio.h
 * to get a named base_addr for each physical port instead of a magic
 * number. These values are the data-memory addresses of each port's
 * PINx register (DDRx and PORTx live at PINx+1 and PINx+2
 * respectively on this device); the HAL implementation derives the
 * other two registers from base_addr internally.
 */
#ifndef ANNEX_HAL_GPIO_ATMEGA328P_H
#define ANNEX_HAL_GPIO_ATMEGA328P_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ANNEX_GPIO_ATMEGA328P_PORTB_BASE ((uintptr_t)0x23)
#define ANNEX_GPIO_ATMEGA328P_PORTC_BASE ((uintptr_t)0x26)
#define ANNEX_GPIO_ATMEGA328P_PORTD_BASE ((uintptr_t)0x29)

/* All three ports on this device are 8 bits wide. PORTC only bonds
 * out 7 of them (PC0-PC6, no PC7), so num_pins is left at 8 here and
 * pin 7 on that port will simply read/drive nothing on real silicon;
 * the HAL has no way to know that without a per-pin capability map. */
#define ANNEX_GPIO_ATMEGA328P_PORT_WIDTH ((uint8_t)8)

#ifdef __cplusplus
}
#endif

#endif /* ANNEX_HAL_GPIO_ATMEGA328P_H */