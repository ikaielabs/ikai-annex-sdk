#include "annex_hal_uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>

struct annex_hal_uart_dev {
    volatile uint8_t *ucsra;       /* Control/Status register A */
    volatile uint8_t *ucsrb;       /* Control/Status register B */
    volatile uint8_t *ucsrc;       /* Control/Status register C */
    volatile uint16_t *ubrr;       /* baud rate register */
    volatile uint8_t  *udr;        /* data register */
    annex_hal_uart_config_t cfg;
    uint32_t   input_clk_hz;        /* Peripheral input clock for baud calculation */
    uint32_t          irq_mask;     /* software-tracked enabled IRQs  */
    uint32_t          irq_status;   /* software-tracked pending flags  */
};

annex_hal_uart_dev_t *annex_hal_uart_open(const annex_hal_uart_hw_desc_t *hw)
{
    if (!hw) return NULL;

    annex_hal_uart_dev_t *dev = (annex_hal_uart_dev_t *)calloc(1, sizeof(*dev));
    if (!dev) return NULL;
    
    uintptr_t base   = hw->base_addr;
    dev->ucsra = (volatile uint8_t  *)(base + 0);
    dev->ucsrb = (volatile uint8_t  *)(base + 1);
    dev->ucsrc = (volatile uint8_t  *)(base + 2);
    dev->ubrr  = (volatile uint16_t *)(base + 4);
    dev->udr   = (volatile uint8_t  *)(base + 6);

    dev->input_clk_hz = hw->input_clk_hz;

    return dev;
}

void annex_hal_uart_close(annex_hal_uart_dev_t *dev)
{
    if (!dev) return;
    *dev->ucsrb = 0;   // disable TX and RX
    free(dev);
}

int annex_hal_uart_config(annex_hal_uart_dev_t *dev, const annex_hal_uart_config_t *cfg)
{
    if (!dev || !cfg) return -1;
    memcpy(&dev->cfg, cfg, sizeof(*cfg));
    return 0;
}

int annex_hal_uart_start(annex_hal_uart_dev_t *dev, bool tx_enable, bool rx_enable)
{
    if (!dev) return -1;

    // Calculate value for baud rate

    *dev->ucsra = (1 << U2X0);

    uint16_t ubrr_val =
    (dev->input_clk_hz / (8UL * dev->cfg.baud_rate)) - 1;

    // if(dev->cfg.baud_rate > (dev->input_clk_hz / 16U))
    //     dev->cfg.baud_rate = (dev->input_clk_hz / 16U);
    
    // uint16_t ubrr_val = (uint16_t)((((dev->input_clk_hz) / dev->cfg.baud_rate) - 8U) / 16U);
    *dev->ubrr = ubrr_val;

    // Frame format — 8N1
    *dev->ucsrc = (1 << UCSZ01) | (1 << UCSZ00);

    // Enable TX / RX
    uint8_t ucsrb = 0;
    if (tx_enable) ucsrb |= (1 << TXEN0);
    if (rx_enable) ucsrb |= (1 << RXEN0);
    *dev->ucsrb |= ucsrb;

    return 0;
}

int annex_hal_uart_stop(annex_hal_uart_dev_t *dev)
{
    if (!dev) return -1;
    *dev->ucsrb &= ~((1 << TXEN0) | (1 << RXEN0));
    return 0;
}

int annex_hal_uart_write_fifo_u8(annex_hal_uart_dev_t *dev, const uint8_t *data, uint32_t len)
{
    if (!dev || !data) return -1;

    for (size_t i = 0; i < len; i++) {
        // Wait for TX buffer empty
        while (!(*dev->ucsra & (1 << UDRE0)));
        *dev->udr = data[i];
    }

    return (int)len;
}

int annex_hal_uart_write_fifo_u16(annex_hal_uart_dev_t *dev, const uint16_t *data, uint32_t len)
{
    if (!dev || !data) return -1;
    for (size_t i = 0; i < len; i++) {
        uint8_t lo = (uint8_t)(data[i] & 0xFF);
        uint8_t hi = (uint8_t)(data[i] >> 8);
        while (!(*dev->ucsra & (1 << UDRE0)));
        *dev->udr = lo;
        while (!(*dev->ucsra & (1 << UDRE0)));
        *dev->udr = hi;
    }
    return (int)(len * 2);
}

int annex_hal_uart_read_fifo_u8(annex_hal_uart_dev_t *dev, uint8_t *data, uint32_t len)
{
    if (!dev || !data) return -1;
    size_t n = 0;
    while (n < len && (*dev->ucsra & (1 << RXC0))) {
        data[n++] = *dev->udr;
    }
    return (int)n;
}

int annex_hal_uart_read_fifo_u16(annex_hal_uart_dev_t *dev, uint16_t *data, uint32_t len)
{
    if (!dev || !data) return -1;
    size_t n = 0;
    while (n < len && (*dev->ucsra & (1 << RXC0))) {
        uint8_t lo = *dev->udr;
        while (!(*dev->ucsra & (1 << RXC0)));
        uint8_t hi = *dev->udr;
        data[n++]  = (uint16_t)(lo | (hi << 8));
    }
    return (int)n;
}

void annex_hal_uart_enable_irq(annex_hal_uart_dev_t *dev, uint32_t irqs)
{
    if (!dev) return;

    dev->irq_mask |= irqs;
    if (irqs & ANNEX_HAL_UART_IRQ_RX_READY)
        *dev->ucsrb |= (1 << RXCIE0);
    if (irqs & ANNEX_HAL_UART_IRQ_TX_EMPTY)
        *dev->ucsrb |= (1 << UDRIE0);
}

void annex_hal_uart_disable_irq(annex_hal_uart_dev_t *dev, uint32_t irqs)
{
    if (!dev) return;

    dev->irq_mask &= ~irqs;
    if (irqs & ANNEX_HAL_UART_IRQ_RX_READY)
        *dev->ucsrb &= ~(1 << RXCIE0);
    if (irqs & ANNEX_HAL_UART_IRQ_TX_EMPTY)
        *dev->ucsrb &= ~(1 << UDRIE0);
}

uint32_t annex_hal_uart_get_irq_status(annex_hal_uart_dev_t *dev)
{
    if (!dev) return 0;
    uint32_t status = 0;
    if (*dev->ucsra & (1 << RXC0))  status |= ANNEX_HAL_UART_IRQ_RX_READY;
    if (*dev->ucsra & (1 << UDRE0)) status |= ANNEX_HAL_UART_IRQ_TX_EMPTY;
    if (*dev->ucsra & (1 << FE0))   status |= ANNEX_HAL_UART_IRQ_FRAME_ERR;
    if (*dev->ucsra & (1 << DOR0))  status |= ANNEX_HAL_UART_IRQ_RX_OVERRUN;
    return status & dev->irq_mask;
}

void annex_hal_uart_clear_irq(annex_hal_uart_dev_t *dev, uint32_t irqs)
{
    if (dev) dev->irq_status &= ~irqs;
}
