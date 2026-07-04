/**
 * @file annex_hal_gpio.h
 * @brief GPIO HAL interface.
 *
 * Each supported MCU (e.g. ATmega328p, STM32) implements this contract
 * in its own annex_hal_gpio_xxx.c, defining the real layout of
 * `struct annex_hal_gpio_dev`. 
 * Callers only ever see the opaque annex_hal_gpio_dev_t pointer.
 */
#ifndef ANNEX_HAL_GPIO_H
#define ANNEX_HAL_GPIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque handle to one GPIO port */
typedef struct annex_hal_gpio_dev annex_hal_gpio_dev_t;

/** Index of a pin within a port (0-based). */
typedef uint32_t annex_gpio_pin_t;

/** Logic level of a pin. */
typedef enum {
    ANNEX_GPIO_LOW  = 0,
    ANNEX_GPIO_HIGH = 1,
} annex_gpio_level_t;

/**
 * Electrical configuration of a pin. Kept to the common subset every
 * vendor can realize; annex_hal_gpio_set_mode() returns
 * ANNEX_HAL_GPIO_ERR_NOT_SUPPORTED for anything a given chip can't do.
 */
typedef enum {
    ANNEX_GPIO_MODE_INPUT,          /* floating input            */
    ANNEX_GPIO_MODE_INPUT_PULLUP,   /* input, internal pull-up   */
    ANNEX_GPIO_MODE_INPUT_PULLDOWN, /* input, internal pull-down */
    ANNEX_GPIO_MODE_OUTPUT_PP,      /* output, push-pull         */
    ANNEX_GPIO_MODE_OUTPUT_OD,      /* output, open-drain        */
} annex_gpio_mode_t;

/** Return codes shared by every function in this HAL. */
typedef enum {
    ANNEX_HAL_GPIO_OK = 0,
    ANNEX_HAL_GPIO_ERR_NULL_PTR,
    ANNEX_HAL_GPIO_ERR_INVALID_PIN,
    ANNEX_HAL_GPIO_ERR_INVALID_PARAM,
    ANNEX_HAL_GPIO_ERR_NOT_SUPPORTED,
    ANNEX_HAL_GPIO_ERR_HW,
} annex_hal_gpio_status_t;

/**
 * Identifies the physical port/bank a dev_t will map to. base_addr
 * covers both memory-mapped AVR port registers (PORTx/DDRx/PINx) and
 * ARM peripheral blocks (GPIOx); each vendor's source knows how to
 * derive its own register offsets from this single base.
 */
typedef struct {
    uintptr_t base_addr;     /* MMIO base address of the port/bank      */
    uint8_t   num_pins;      /* pins implemented, e.g. 8 on an AVR port */
    void     *platform_data; /* vendor-specific extras, optional        */
} annex_hal_gpio_hw_desc_t;

/* --- Lifecycle ------------------------------------------------------ */

/**
 * Bind a HAL handle to the port/bank described by hw_desc.
 * Implementations may back the handle with static storage instead of
 * the heap (recommended on RAM-constrained targets like the
 * ATmega328p); callers must treat the returned pointer as opaque
 * either way.
 * @return handle on success, NULL on failure.
 */
annex_hal_gpio_dev_t *annex_hal_gpio_open(const annex_hal_gpio_hw_desc_t *hw_desc);

/** Release a handle obtained from annex_hal_gpio_open(). */
void annex_hal_gpio_close(annex_hal_gpio_dev_t *dev);

/* --- Configuration ---------------------------------------------------- */

/** Configure the electrical mode/direction of a single pin. */
annex_hal_gpio_status_t annex_hal_gpio_set_mode(annex_hal_gpio_dev_t *dev,
                                                 annex_gpio_pin_t pin,
                                                 annex_gpio_mode_t mode);

/* --- Data I/O ----------------------------------------------------------- */

/** Drive a pin to the given level. Pin must already be in an output mode. */
annex_hal_gpio_status_t annex_hal_gpio_write(annex_hal_gpio_dev_t *dev,
                                              annex_gpio_pin_t pin,
                                              annex_gpio_level_t level);

/** Read the current level of a pin into *level. */
annex_hal_gpio_status_t annex_hal_gpio_read(annex_hal_gpio_dev_t *dev,
                                             annex_gpio_pin_t pin,
                                             annex_gpio_level_t *level);

/** Invert the current output level of a pin. */
annex_hal_gpio_status_t annex_hal_gpio_toggle(annex_hal_gpio_dev_t *dev,
                                               annex_gpio_pin_t pin);

#ifdef __cplusplus
}
#endif

#endif /* ANNEX_HAL_GPIO_H */
