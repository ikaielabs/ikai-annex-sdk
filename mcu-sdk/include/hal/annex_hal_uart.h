#ifndef ANNEX_HAL_UART_H
#define ANNEX_HAL_UART_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ANNEX_HAL_UART_IRQ_RX_READY   (1U << 0)
#define ANNEX_HAL_UART_IRQ_TX_EMPTY   (1U << 1)
#define ANNEX_HAL_UART_IRQ_RX_OVERRUN (1U << 2)
#define ANNEX_HAL_UART_IRQ_FRAME_ERR  (1U << 3)

typedef struct annex_hal_uart_dev annex_hal_uart_dev_t;

typedef struct {
    uint32_t baud_rate;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t flow_control;
} annex_hal_uart_config_t;

typedef struct {
    uintptr_t  base_addr;     /* MMIO base                                   */
    uint32_t   input_clk_hz;  /* Peripheral input clock for baud calculation */
    int8_t     tx_pin;        /* Board-level pin number (-1 = unused)        */
    int8_t     rx_pin;        /* Board-level pin number (-1 = unused)        */
    void      *platform_data; /* Anything else (optional)                    */
} annex_hal_uart_hw_desc_t;

annex_hal_uart_dev_t *annex_hal_uart_open (const annex_hal_uart_hw_desc_t *hw);
void annex_hal_uart_close(annex_hal_uart_dev_t *dev);
int annex_hal_uart_config(annex_hal_uart_dev_t *dev, const annex_hal_uart_config_t *cfg);
int annex_hal_uart_write_fifo_u8(annex_hal_uart_dev_t *dev, const uint8_t *data, uint32_t len, uint32_t timeout);
int annex_hal_uart_write_fifo_u16(annex_hal_uart_dev_t *dev, const uint16_t *data, uint32_t len, uint32_t timeout);
int annex_hal_uart_read_fifo_u8(annex_hal_uart_dev_t *dev, uint8_t *data, uint32_t len, uint32_t timeout);
int annex_hal_uart_read_fifo_u16(annex_hal_uart_dev_t *dev, uint16_t *data, uint32_t len, uint32_t timeout);
int annex_hal_uart_start(annex_hal_uart_dev_t *dev, bool tx_enable, bool rx_enable);
int annex_hal_uart_stop(annex_hal_uart_dev_t *dev);
/*
 * Enter LIN mode with a given break-detection length (10 or 11 bits).
 * This is the real entry point into LIN mode (uses HAL_LIN_Init under
 * the hood on ST parts) and should be called once, after
 * annex_hal_uart_config(), before the first annex_hal_uart_send_break().
 *
 * break_length_bits: 11 selects an 11-bit break detection length
 * (LIN 2.x), any other value selects 10-bit (LIN 1.x).
 * Returns 0 on success, -1 on failure.
 */
int annex_hal_uart_lin_init(annex_hal_uart_dev_t *dev, uint8_t break_length_bits);

int annex_hal_uart_set_lin_mode(annex_hal_uart_dev_t *dev, bool enable);
int annex_hal_uart_send_break(annex_hal_uart_dev_t *dev);
void annex_hal_uart_enable_irq(annex_hal_uart_dev_t *dev, uint32_t irqs);
void annex_hal_uart_disable_irq(annex_hal_uart_dev_t *dev, uint32_t irqs);
uint32_t annex_hal_uart_get_irq_status(annex_hal_uart_dev_t *dev);
void annex_hal_uart_clear_irq(annex_hal_uart_dev_t *dev, uint32_t irqs);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ANNEX_HAL_UART_H */



