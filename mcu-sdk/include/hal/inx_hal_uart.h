#ifndef INX_HAL_UART_H
#define INX_HAL_UART_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INX_HAL_UART_IRQ_RX_READY   (1U << 0)
#define INX_HAL_UART_IRQ_TX_EMPTY   (1U << 1)
#define INX_HAL_UART_IRQ_RX_OVERRUN (1U << 2)
#define INX_HAL_UART_IRQ_FRAME_ERR  (1U << 3)
/* LIN break detection (hardware LBD) */
#define INX_HAL_UART_IRQ_BREAK_DET  (1U << 4)

typedef struct inx_hal_uart_dev inx_hal_uart_dev_t;

typedef struct {
    uint32_t baud_rate;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t flow_control;
} inx_hal_uart_config_t;

typedef struct {
    uintptr_t  base_addr;     /* MMIO base                                   */
    uint32_t   input_clk_hz;  /* Peripheral input clock for baud calculation */
    int8_t     tx_pin;        /* Board-level pin number (-1 = unused)        */
    int8_t     rx_pin;        /* Board-level pin number (-1 = unused)        */
    void      *platform_data; /* Anything else (optional)                    */
} inx_hal_uart_hw_desc_t;

inx_hal_uart_dev_t *inx_hal_uart_open (const inx_hal_uart_hw_desc_t *hw);
void inx_hal_uart_close(inx_hal_uart_dev_t *dev);
int inx_hal_uart_config(inx_hal_uart_dev_t *dev, const inx_hal_uart_config_t *cfg);
int inx_hal_uart_write_fifo_u8(inx_hal_uart_dev_t *dev, const uint8_t *data, uint32_t len, uint32_t timeout);
int inx_hal_uart_write_fifo_u16(inx_hal_uart_dev_t *dev, const uint16_t *data, uint32_t len, uint32_t timeout);
int inx_hal_uart_read_fifo_u8(inx_hal_uart_dev_t *dev, uint8_t *data, uint32_t len, uint32_t timeout);
int inx_hal_uart_read_fifo_u16(inx_hal_uart_dev_t *dev, uint16_t *data, uint32_t len, uint32_t timeout);
int inx_hal_uart_start(inx_hal_uart_dev_t *dev, bool tx_enable, bool rx_enable);
int inx_hal_uart_stop(inx_hal_uart_dev_t *dev);
/*
 * Enter LIN mode with a given break-detection length (10 or 11 bits).
 * This is the real entry point into LIN mode (uses HAL_LIN_Init under
 * the hood on ST parts) and should be called once, after
 * inx_hal_uart_config(), before the first inx_hal_uart_send_break().
 *
 * break_length_bits: 11 selects an 11-bit break detection length
 * (LIN 2.x), any other value selects 10-bit (LIN 1.x).
 * Returns 0 on success, -1 on failure.
 */
int inx_hal_uart_lin_init(inx_hal_uart_dev_t *dev, uint8_t break_length_bits);

int inx_hal_uart_set_lin_mode(inx_hal_uart_dev_t *dev, bool enable);
int inx_hal_uart_send_break(inx_hal_uart_dev_t *dev);
/*
 * Blocking wait for LIN break detection. Waits until the hardware LIN
 * Break Detection flag (LBD) is set or timeout expires.
 *
 * timeout_ms: number of milliseconds to wait. Use `HAL_MAX_DELAY` for
 * an infinite wait.
 * Returns 0 on success (break detected), -1 on error or timeout.
 */
int inx_hal_uart_wait_for_break(inx_hal_uart_dev_t *dev, uint32_t timeout_ms);
void inx_hal_uart_enable_irq(inx_hal_uart_dev_t *dev, uint32_t irqs);
void inx_hal_uart_disable_irq(inx_hal_uart_dev_t *dev, uint32_t irqs);
uint32_t inx_hal_uart_get_irq_status(inx_hal_uart_dev_t *dev);
void inx_hal_uart_clear_irq(inx_hal_uart_dev_t *dev, uint32_t irqs);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* INX_HAL_UART_H */



