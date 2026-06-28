#include <avr/io.h>
#include "annex_adc.h"
#include "annex_hal_gpio_atmega328p.h"
#include "annex_uart_io.h"
#include <avr/interrupt.h> 

#define MOSI_PIN 3U //PB3 - Pin 11 on UNO
#define MISO_PIN 4U //PB4 - Pin 12 on UNO
#define SCK_PIN 5U //PB5 - Pin 13 on UNO
#define SS_PIN 2U //PB2 - Pin 10 on UNO

static annex_adc_dev_t *adc;
static annex_hal_spi_dev_t *spi0;
static annex_hal_gpio_dev_t *gpio_portb;

FILE uart_stream;
char data[100];

uint16_t raw0,raw1;
uint16_t mv0,mv1;

void setup() {

    sei();
    annex_uart_io_init(16000000UL, 115200, &uart_stream); 

    annex_hal_gpio_hw_desc_t gpio_hw = {
        .base_addr     = ANNEX_GPIO_ATMEGA328P_PORTB_BASE,
        .num_pins      = 8,
        .platform_data = NULL,
    };

    gpio_portb = annex_hal_gpio_open(&gpio_hw);
    spi_gpio_init(); 

    annex_hal_spi_hw_desc_t spi_hw = {
        .base_addr    = (uintptr_t)&SPCR,            
        .input_clk_hz = 16000000UL,
    };
    
    spi0 = annex_hal_spi_open(&spi_hw);

    annex_adc_hw_desc_t adc_hw = {
        .spi_handle = spi0,
        .spi_baud_rate_hz = 2000000UL,
        .cs_port_handle = gpio_portb,
        .cs_pin = (annex_gpio_pin_t)SS_PIN,            
        .vref_mv =  ADC_VREF_5000MV          
    };
    
    adc = annex_adc_open(&adc_hw);
}

void loop() {
    /* Read Channel 0 */
    adc_annex_read_raw(adc, ANNEX_ADC_CH0, &raw0);
    adc_annex_read_mv(adc, ANNEX_ADC_CH0, &mv0);

    /* Read Channel 1 */
	adc_annex_read_raw(adc, ANNEX_ADC_CH1, &raw1);
	adc_annex_read_mv(adc, ANNEX_ADC_CH1, &mv1);

	printf("\r\n");
	printf("+------+--------+-----------+\r\n");
	printf("| CH   | RAW    | VOLTAGE (mV)  |\r\n");
	printf("+------+--------+-----------+\r\n");
	printf("| CH0  | %-6u | %-6u mV |\r\n", raw0, mv0);
	printf("| CH1  | %-6u | %-6u mV |\r\n", raw1, mv1);
	printf("+------+--------+-----------+\r\n");
	delay(2000);
}

static void spi_gpio_init(void) {
    /* All Pins are part of Port B, reusing the CS port handle */
    (void)annex_hal_gpio_set_mode(gpio_portb, (annex_gpio_pin_t)MOSI_PIN, ANNEX_GPIO_MODE_OUTPUT_PP);
    (void)annex_hal_gpio_set_mode(gpio_portb, (annex_gpio_pin_t)SCK_PIN, ANNEX_GPIO_MODE_OUTPUT_PP);
    (void)annex_hal_gpio_set_mode(gpio_portb, (annex_gpio_pin_t)MISO_PIN, ANNEX_GPIO_MODE_INPUT_PULLUP);
}
