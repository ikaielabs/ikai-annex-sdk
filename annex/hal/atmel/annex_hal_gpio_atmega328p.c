/**
 * @file annex_hal_gpio_atmega328p.c
 * @brief ATmega328p implementation for annex_hal_gpio.h.
 *
 * Register layout:
 *
 *      base_addr + 0  ->  PINx   (read pin state; writing a 1 to a
 *                                 bit here toggles the matching
 *                                 PORTx bit, independent of DDRx)
 *      base_addr + 1  ->  DDRx   (1 = output, 0 = input)
 *      base_addr + 2  ->  PORTx  (output level, or pull-up enable
 *                                 when the pin is an input)
 *
 * Access goes through plain volatile pointers built from a runtime
 * base_addr rather than avr-libc's named registers. That is a
 * deliberate trade-off: because base_addr is only known at runtime,
 * the compiler must emit indirect LD/ST instructions instead of the
 * single-cycle IN/OUT/SBI/CBI forms it could use for a compile-time
 * constant address. That's the cost of a backend that has to satisfy
 * a vendor-agnostic header instead of hardcoding DDRB/PORTB/PINB.
 *
 * No dynamic allocation: annex_hal_gpio_open() hands out handles from
 * a fixed-size static pool sized for this chip's three ports, which
 * keeps the HAL usable on a part with 2KB of SRAM.
 */
#include "annex_hal_gpio.h"

#include <stddef.h>
#include <avr/io.h>        /* SREG */
#include <avr/interrupt.h> /* cli() */

/* Real definition of the opaque type declared in annex_hal_gpio.h. */
struct annex_hal_gpio_dev {
    uintptr_t base_addr;
    uint8_t   num_pins;
    uint8_t   in_use;
};

/* The ATmega328p exposes exactly three GPIO ports (B, C, D). */
#define ANNEX_HAL_GPIO_MAX_DEVS 3U

static struct annex_hal_gpio_dev s_dev_pool[ANNEX_HAL_GPIO_MAX_DEVS];

static inline volatile uint8_t *reg_pin(uintptr_t base)
{
    return (volatile uint8_t *)base;
}

static inline volatile uint8_t *reg_ddr(uintptr_t base)
{
    return (volatile uint8_t *)(base + 1u);
}

static inline volatile uint8_t *reg_port(uintptr_t base)
{
    return (volatile uint8_t *)(base + 2u);
}

annex_hal_gpio_dev_t *annex_hal_gpio_open(const annex_hal_gpio_hw_desc_t *hw_desc)
{
    if (hw_desc == NULL) {
        return NULL;
    }
    /* Every port on this chip is 8 bits wide */
    if (hw_desc->num_pins == 0U || hw_desc->num_pins > 8U) {
        return NULL;
    }

    for (size_t i = 0; i < ANNEX_HAL_GPIO_MAX_DEVS; i++) {
        if (!s_dev_pool[i].in_use) {
            s_dev_pool[i].base_addr = hw_desc->base_addr;
            s_dev_pool[i].num_pins  = hw_desc->num_pins;
            s_dev_pool[i].in_use    = 1u;
            return &s_dev_pool[i];
        }
    }
    return NULL; /* All 3 ports already open */
}

void annex_hal_gpio_close(annex_hal_gpio_dev_t *dev)
{
    if (dev != NULL) {
        dev->in_use = 0U;
    }
}

annex_hal_gpio_status_t annex_hal_gpio_set_mode(annex_hal_gpio_dev_t *dev,
                                                 annex_gpio_pin_t pin,
                                                 annex_gpio_mode_t mode)
{
    if (dev == NULL) {
        return ANNEX_HAL_GPIO_ERR_NULL_PTR;
    }
    if (pin >= dev->num_pins) {
        return ANNEX_HAL_GPIO_ERR_INVALID_PIN;
    }

    volatile uint8_t *ddr  = reg_ddr(dev->base_addr);
    volatile uint8_t *port = reg_port(dev->base_addr);
    uint8_t mask = (uint8_t)(1U << pin);
    uint8_t sreg_save = SREG;
    cli(); /* DDRx/PORTx read-modify-write must not be interrupted by
            * an ISR touching another bit of the same port */

    switch (mode) {
    case ANNEX_GPIO_MODE_INPUT:
        *ddr  &= (uint8_t)~mask;
        *port &= (uint8_t)~mask;
        break;
    case ANNEX_GPIO_MODE_INPUT_PULLUP:
        *ddr  &= (uint8_t)~mask;
        *port |= mask;
        break;
    case ANNEX_GPIO_MODE_OUTPUT_PP:
        *ddr |= mask;
        break;
    case ANNEX_GPIO_MODE_INPUT_PULLDOWN:
    case ANNEX_GPIO_MODE_OUTPUT_OD:
        /* Plain AVR I/O pins have no pull-down and no true
         * open-drain mode; only the dedicated TWI pins do, and they
         * aren't addressable as generic GPIO. Report it rather than
         * silently configuring something else. */
        SREG = sreg_save;
        return ANNEX_HAL_GPIO_ERR_NOT_SUPPORTED;
    default:
        SREG = sreg_save;
        return ANNEX_HAL_GPIO_ERR_INVALID_PARAM;
    }

    SREG = sreg_save;
    return ANNEX_HAL_GPIO_OK;
}

annex_hal_gpio_status_t annex_hal_gpio_write(annex_hal_gpio_dev_t *dev,
                                              annex_gpio_pin_t pin,
                                              annex_gpio_level_t level)
{
    if (dev == NULL) {
        return ANNEX_HAL_GPIO_ERR_NULL_PTR;
    }
    if (pin >= dev->num_pins) {
        return ANNEX_HAL_GPIO_ERR_INVALID_PIN;
    }

    volatile uint8_t *port = reg_port(dev->base_addr);
    uint8_t mask = (uint8_t)(1U << pin);
    uint8_t sreg_save = SREG;
    cli();
    if (level == ANNEX_GPIO_HIGH) {
        *port |= mask;
    } else {
        *port &= (uint8_t)~mask;
    }
    SREG = sreg_save;
    return ANNEX_HAL_GPIO_OK;
}

annex_hal_gpio_status_t annex_hal_gpio_read(annex_hal_gpio_dev_t *dev,
                                             annex_gpio_pin_t pin,
                                             annex_gpio_level_t *level)
{
    if (dev == NULL || level == NULL) {
        return ANNEX_HAL_GPIO_ERR_NULL_PTR;
    }
    if (pin >= dev->num_pins) {
        return ANNEX_HAL_GPIO_ERR_INVALID_PIN;
    }

    volatile uint8_t *pinreg = reg_pin(dev->base_addr);
    uint8_t mask = (uint8_t)(1U << pin);
    *level = ((*pinreg) & mask) ? ANNEX_GPIO_HIGH : ANNEX_GPIO_LOW;
    return ANNEX_HAL_GPIO_OK;
}

annex_hal_gpio_status_t annex_hal_gpio_toggle(annex_hal_gpio_dev_t *dev,
                                               annex_gpio_pin_t pin)
{
    if (dev == NULL) {
        return ANNEX_HAL_GPIO_ERR_NULL_PTR;
    }
    if (pin >= dev->num_pins) {
        return ANNEX_HAL_GPIO_ERR_INVALID_PIN;
    }

    /* On this device, writing a 1 to a PINx bit toggles the matching
     * PORTx bit directly in hardware (datasheet section "Toggling
     * the Pin"), independent of cli()/sei() since it's a single
     * OUT instruction - no read-modify-write race to protect. */
    volatile uint8_t *pinreg = reg_pin(dev->base_addr);
    *pinreg = (uint8_t)(1U << pin);
    return ANNEX_HAL_GPIO_OK;
}