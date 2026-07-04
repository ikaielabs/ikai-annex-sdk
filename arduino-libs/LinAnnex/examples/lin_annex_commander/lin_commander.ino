#include <avr/io.h>
#include "annex_lin.h"
#include "annex_hal_gpio_atmega328p.h" 

static annex_lin_dev_t *lin_handle;
static annex_hal_uart_dev_t *uart0;
static annex_hal_gpio_dev_t *gpio_portd;

static uint8_t tx_data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

void setup() {

    annex_hal_gpio_hw_desc_t gpio_portd_hw = {
        .base_addr     = ANNEX_GPIO_ATMEGA328P_PORTD_BASE,
        .num_pins      = 8,
        .platform_data = NULL,
    };
    gpio_portd = annex_hal_gpio_open(&gpio_portd_hw);

    annex_hal_uart_hw_desc_t uart_hw = {
        .base_addr    = (uintptr_t)&UCSR0A,
        .input_clk_hz = 16000000UL,
    };
    uart0 = annex_hal_uart_open(&uart_hw);

    annex_lin_hw_desc_t hw = {
        .uart_handle    = uart0,
        .gpio_handle    = gpio_portd,
        .gpio_tx_pin   = 1,
        .input_clk_hz = 16000000,
    };

    annex_lin_config_t cfg = {
        .lin_baudrate = 62500U,
        .break_width = 13U,
        .flags = ANNEX_LIN_MASTER,
    };

    lin_handle = annex_lin_open (&hw);
    if(annex_lin_config(lin_handle, &cfg) != LIN_OK)
       return;
    
    if(annex_lin_start(lin_handle) != LIN_OK)
        return;

    delay(150);

    if(annex_lin_write_message(lin_handle, 0x17U, tx_data, 8U) != LIN_OK)
        return;
}

void loop() { }
