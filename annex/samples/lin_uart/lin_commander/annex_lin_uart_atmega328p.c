#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "annex_lin.h"
#include "annex_hal_uart.h"

struct annex_lin_dev {
    annex_hal_uart_dev_t *uart_handle;
};

static uint32_t flags = 0U;
static uint32_t break_duration = 0U;
static uint32_t bit_duration = 0U;

annex_lin_dev_t *annex_lin_open (const annex_lin_hw_desc_t *hw) {

    if (!hw) return NULL;

    annex_lin_dev_t *dev = (annex_lin_dev_t *)calloc(1, sizeof(*dev));
    if (!dev) return NULL;

    annex_hal_uart_hw_desc_t hw = {
        .base_addr    = hw->base_addr,            
        .input_clk_hz = hw->input_clk_hz,
        .tx_pin       = 1,           /* board-specific */
        .rx_pin       = 2, 
    };

    dev->uart_handle = annex_hal_uart_open(&hw);

    return dev;
}

void annex_lin_close(annex_lin_dev_t *dev) {
    if (!dev) return;

    annex_hal_uart_close(dev->uart_handle);

    free(dev);
}

annex_lin_config(annex_lin_dev_t *dev, annex_lin_config_t *cfg) {
    if (!dev) 
        return LIN_NOT_INITIALIZED;
    
    if (!cfg)
        return LIN_ERR;

    if(annex_hal_uart_stop(dev->uart_handle) != 0)
        return LIN_ERR;

    annex_hal_uart_config_t uart_cfg = {
        .baud_rate    = cfg->lin_baudrate,
        .data_bits    = 8,
        .stop_bits    = 1,
        .parity       = 0,
        .flow_control = 0
    };

    (void)annex_hal_uart_config(dev->uart_handle, &uart_cfg);

    flags = cfg->flags;

    break_duration = (uint32_t)((float)(cfg->break_width * 1000000) / (float)cfg->lin_baudrate);

    bit_duration = (uint32_t)((float)(1U * 1000000) / (float)cfg->lin_baudrate);

    return LIN_OK;
}


