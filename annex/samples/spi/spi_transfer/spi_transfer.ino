#include <avr/io.h>
#include "annex_hal_spi.h"
#include "annex_hal_gpio.h"
#include "annex_hal_gpio_atmega328p.h" 
#include <avr/interrupt.h>

#define DATA_LENGTH 8U
#define MOSI_PIN 3U //PB3 - Pin 11 on UNO
#define MISO_PIN 4U //PB4 - Pin 12 on UNO
#define SCK_PIN 5U //PB5 - Pin 13 on UNO
#define SS_PIN 2U //PB2 - Pin 10 on UNO

static annex_hal_spi_dev_t *spi0;
static annex_hal_gpio_dev_t *gpio;

static const uint8_t tx_data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
static uint8_t rx_data[8] = {0}; 


static void spi_gpio_init(void);

void setup() {
    annex_hal_gpio_hw_desc_t gpio_hw = {
        .base_addr     = ANNEX_GPIO_ATMEGA328P_PORTB_BASE,
        .num_pins      = 8,
        .platform_data = NULL,
    };

    gpio = annex_hal_gpio_open(&gpio_hw);
    spi_gpio_init(); 

    annex_hal_spi_hw_desc_t spi_hw = {
        .base_addr    = (uintptr_t)&SPCR,            
        .input_clk_hz = 16000000UL,
    };

    annex_hal_spi_config_t cfg = {
        .baud_rate_hz    = 2000000UL,
        .mode    = ANNEX_HAL_SPI_MODE_0,
        .role    = ANNEX_HAL_SPI_MASTER,
        .bit_order       = ANNEX_HAL_SPI_MSB_FIRST,
        .bits_per_word = 8,
        .use_dma = false
    };

    spi0 = annex_hal_spi_open(&spi_hw);

    int ret = annex_hal_spi_config(spi0, &cfg);
    
    annex_hal_spi_enable(spi0);

    // CS LOW
    if (annex_hal_gpio_write(gpio, (annex_gpio_pin_t)SS_PIN, ANNEX_GPIO_LOW) != ANNEX_HAL_GPIO_OK)
       return;
    
    /* Transfer the data */
    annex_hal_spi_transfer(spi0, tx_data, rx_data, DATA_LENGTH);

    // CS High
    if (annex_hal_gpio_write(gpio, (annex_gpio_pin_t)SS_PIN, ANNEX_GPIO_HIGH) != ANNEX_HAL_GPIO_OK)
       return;
}

void loop() {}

static void spi_gpio_init(void) {
    (void)annex_hal_gpio_set_mode(gpio, (annex_gpio_pin_t)MOSI_PIN, ANNEX_GPIO_MODE_OUTPUT_PP);
    (void)annex_hal_gpio_set_mode(gpio, (annex_gpio_pin_t)SCK_PIN, ANNEX_GPIO_MODE_OUTPUT_PP);
    (void)annex_hal_gpio_set_mode(gpio, (annex_gpio_pin_t)SS_PIN, ANNEX_GPIO_MODE_OUTPUT_PP);
    (void)annex_hal_gpio_set_mode(gpio, (annex_gpio_pin_t)MISO_PIN, ANNEX_GPIO_MODE_INPUT_PULLUP);
}
