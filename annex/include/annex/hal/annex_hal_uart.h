#ifndef ANNEX_HAL_UART_H
#define ANNEX_HAL_UART_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ANNEX_HAL_UART_IRQ_RX_READY   (1u << 0)
#define ANNEX_HAL_UART_IRQ_TX_EMPTY   (1u << 1)
#define ANNEX_HAL_UART_IRQ_RX_OVERRUN (1u << 2)
#define ANNEX_HAL_UART_IRQ_FRAME_ERR  (1u << 3)

typedef struct annex_hal_uart_dev annex_hal_uart_dev_t;

typedef struct {
    uint32_t baud_rate;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t flow_control;
} annex_hal_uart_config_t;

typedef struct {
    uintptr_t  base_addr;     /* MMIO base                               */
    uint32_t   input_clk_hz;  /* Peripheral input clock     for baud calculation */
    int8_t     tx_pin;        /* Board-level pin number (-1 = unused)        */
    int8_t     rx_pin;        /* Board-level pin number (-1 = unused)        */
    void      *platform_data; /* Anything else (optional)                    */
} annex_hal_uart_hw_desc_t;

annex_hal_uart_dev_t *annex_hal_uart_open (const annex_hal_uart_hw_desc_t *hw);
void annex_hal_uart_close(annex_hal_uart_dev_t *dev);
int annex_hal_uart_config(annex_hal_uart_dev_t *dev, const annex_hal_uart_config_t *cfg);
int annex_hal_uart_write_fifo_u8(annex_hal_uart_dev_t *dev, const uint8_t *data, uint32_t len);
int annex_hal_uart_write_fifo_u16(annex_hal_uart_dev_t *dev, const uint16_t *data, uint32_t len);
int annex_hal_uart_read_fifo_u8(annex_hal_uart_dev_t *dev, uint8_t *data, uint32_t len);
int annex_hal_uart_read_fifo_u16(annex_hal_uart_dev_t *dev, uint16_t *data, uint32_t len);
int annex_hal_uart_start(annex_hal_uart_dev_t *dev, bool tx_enable, bool rx_enable);
int annex_hal_uart_stop(annex_hal_uart_dev_t *dev);
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



