/**
 * @file annex_hal_gpio_st.c
 * @brief STM32 implementation for annex_hal_gpio.h using STM HAL APIs.
 *
 * This implementation uses STMicroelectronics HAL library instead of direct
 * register access, providing better maintainability and compatibility with
 * the STM32Cube ecosystem.
 *
 * HAL API USED:
 * =============
 *   - GPIO_InitTypeDef: Configuration structure for HAL_GPIO_Init()
 *   - HAL_GPIO_Init(): Initialize GPIO pin(s) with configuration
 *   - HAL_GPIO_DeInit(): Deinitialize GPIO pin(s)
 *   - HAL_GPIO_WritePin(): Drive a pin to HIGH or LOW
 *   - HAL_GPIO_ReadPin(): Read the current level of a pin
 *   - HAL_GPIO_TogglePin(): Toggle a pin's output state
 *   - GPIO_TypeDef: Port structure (GPIOA, GPIOB, etc.)
 *
 * MEMORY MANAGEMENT:
 * ==================
 * - Uses static pool (no malloc/free) for up to 16 GPIO ports
 * - Each port instance pre-allocates GPIO port data and pin tracking
 * - Suitable for real-time embedded systems with fixed resource allocation
 *
 * PIN MODE MAPPING:
 * =================
 *   ANNEX_GPIO_MODE_INPUT           → GPIO_MODE_INPUT (floating)
 *   ANNEX_GPIO_MODE_INPUT_PULLUP    → GPIO_MODE_INPUT + Pull-Up
 *   ANNEX_GPIO_MODE_INPUT_PULLDOWN  → GPIO_MODE_INPUT + Pull-Down
 *   ANNEX_GPIO_MODE_OUTPUT_PP       → GPIO_MODE_OUTPUT_PP (push-pull)
 *   ANNEX_GPIO_MODE_OUTPUT_OD       → GPIO_MODE_OUTPUT_OD (open-drain)
 *
 * GPIO PORTS:
 * ===========
 *   STM32 typically has GPIOA, GPIOB, GPIOC, ... GPIOJ (depending on variant)
 *   Each port has 16 pins (GPIO_PIN_0 through GPIO_PIN_15)
 *   Base address is used to identify which port instance to use
 *
 * INTERRUPT CAPABILITY:
 * =====================
 * Basic GPIO HAL does not provide interrupt control via this interface.
 * For interrupt-driven GPIO, use EXTI (External Interrupt) directly or
 * implement a separate EXTI wrapper layer.
 *
 */

#include "annex_hal_gpio.h"
#include "stm32f4xx_hal.h"

#include <stddef.h>
#include <string.h>

/* Real definition of the opaque type declared in annex_hal_gpio.h.
 * Wraps STM32 HAL GPIO port with Annex-specific metadata.
 */
struct annex_hal_gpio_dev {
    GPIO_TypeDef         *port;            /* STM32 GPIO port (GPIOA, GPIOB, etc.) */
    annex_hal_gpio_hw_desc_t hw_desc;      /* Hardware descriptor copy */
    uint8_t              in_use;           /* Pool occupancy flag */
};

/* STM GPIO device pool (supporting multiple GPIO ports) */
#define ANNEX_HAL_GPIO_MAX_DEVS 16U

static struct annex_hal_gpio_dev s_dev_pool[ANNEX_HAL_GPIO_MAX_DEVS];

/**
 * Helper function to convert STM base address to GPIO_TypeDef pointer.
 * Base addresses are MCU-specific (e.g., GPIOA_BASE, GPIOB_BASE, etc.)
 */
static GPIO_TypeDef *get_gpio_port_from_base(uintptr_t base_addr)
{
    return (GPIO_TypeDef *)base_addr;
}

/**
 * Helper function to convert Annex GPIO mode to STM32 HAL GPIO mode.
 */
static uint32_t mode_to_hal_mode(annex_gpio_mode_t mode)
{
    switch (mode) {
    case ANNEX_GPIO_MODE_INPUT:
        return GPIO_MODE_INPUT;
    case ANNEX_GPIO_MODE_INPUT_PULLUP:
        return GPIO_MODE_INPUT;
    case ANNEX_GPIO_MODE_INPUT_PULLDOWN:
        return GPIO_MODE_INPUT;
    case ANNEX_GPIO_MODE_OUTPUT_PP:
        return GPIO_MODE_OUTPUT_PP;
    case ANNEX_GPIO_MODE_OUTPUT_OD:
        return GPIO_MODE_OUTPUT_OD;
    default:
        return GPIO_MODE_INPUT;
    }
}

/**
 * Helper function to get the pull-up/pull-down configuration from mode.
 */
static uint32_t mode_to_hal_pull(annex_gpio_mode_t mode)
{
    switch (mode) {
    case ANNEX_GPIO_MODE_INPUT_PULLUP:
        return GPIO_PULLUP;
    case ANNEX_GPIO_MODE_INPUT_PULLDOWN:
        return GPIO_PULLDOWN;
    case ANNEX_GPIO_MODE_INPUT:
    case ANNEX_GPIO_MODE_OUTPUT_PP:
    case ANNEX_GPIO_MODE_OUTPUT_OD:
    default:
        return GPIO_NOPULL;
    }
}

/* ---------- Lifecycle ------------------------------------------------- */

annex_hal_gpio_dev_t *annex_hal_gpio_open(const annex_hal_gpio_hw_desc_t *hw_desc)
{
    if (hw_desc == NULL) {
        return NULL;
    }

    /* Find unused device in pool */
    for (size_t i = 0; i < ANNEX_HAL_GPIO_MAX_DEVS; i++) {
        if (!s_dev_pool[i].in_use) {
            /* Initialize device structure */
            s_dev_pool[i].port = get_gpio_port_from_base(hw_desc->base_addr);
            memcpy(&s_dev_pool[i].hw_desc, hw_desc, sizeof(annex_hal_gpio_hw_desc_t));
            s_dev_pool[i].in_use = 1U;

            return &s_dev_pool[i];
        }
    }

    return NULL;  /* All GPIO pools exhausted */
}

void annex_hal_gpio_close(annex_hal_gpio_dev_t *dev)
{
    if (dev == NULL) {
        return;
    }

    dev->in_use = 0U;
}

/* ---------- Configuration ---------------------------------------------- */

annex_hal_gpio_status_t annex_hal_gpio_set_mode(annex_hal_gpio_dev_t *dev,
                                                 annex_gpio_pin_t pin,
                                                 annex_gpio_mode_t mode)
{
    if (dev == NULL) {
        return ANNEX_HAL_GPIO_ERR_NULL_PTR;
    }

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure GPIO pin */
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = mode_to_hal_mode(mode);
    GPIO_InitStruct.Pull = mode_to_hal_pull(mode);
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  /* Default to low speed */

    HAL_GPIO_Init(dev->port, &GPIO_InitStruct);
    return ANNEX_HAL_GPIO_OK;
}

/* ---------- Data I/O -------------------------------------------------------- */

annex_hal_gpio_status_t annex_hal_gpio_write(annex_hal_gpio_dev_t *dev,
                                              annex_gpio_pin_t pin,
                                              annex_gpio_level_t level)
{
    if (dev == NULL) {
        return ANNEX_HAL_GPIO_ERR_NULL_PTR;
    }

    HAL_GPIO_WritePin(dev->port, pin,
                     (level == ANNEX_GPIO_HIGH ? GPIO_PIN_SET : GPIO_PIN_RESET));

    return ANNEX_HAL_GPIO_OK;
}

annex_hal_gpio_status_t annex_hal_gpio_read(annex_hal_gpio_dev_t *dev,
                                             annex_gpio_pin_t pin,
                                             annex_gpio_level_t *level)
{
    if (dev == NULL || level == NULL) {
        return ANNEX_HAL_GPIO_ERR_NULL_PTR;
    }

    if (pin >= dev->hw_desc.num_pins) {
        return ANNEX_HAL_GPIO_ERR_INVALID_PIN;
    }

    GPIO_PinState pin_state = HAL_GPIO_ReadPin(dev->port, pin);

    *level = (pin_state == GPIO_PIN_SET) ? ANNEX_GPIO_HIGH : ANNEX_GPIO_LOW;

    return ANNEX_HAL_GPIO_OK;
}

annex_hal_gpio_status_t annex_hal_gpio_toggle(annex_hal_gpio_dev_t *dev,
                                               annex_gpio_pin_t pin)
{
    if (dev == NULL) {
        return ANNEX_HAL_GPIO_ERR_NULL_PTR;
    }

    if (pin >= dev->hw_desc.num_pins) {
        return ANNEX_HAL_GPIO_ERR_INVALID_PIN;
    }

    HAL_GPIO_TogglePin(dev->port, pin);

    return ANNEX_HAL_GPIO_OK;
}
