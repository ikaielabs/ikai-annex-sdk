#include <avr/io.h>
#include <avr/interrupt.h>
#include "annex_lin.h"
#include "annex_hal_gpio_atmega328p.h"
#include "annex_uart_io.h"

FILE uart_stream;

static annex_lin_dev_t *lin_handle;
static annex_hal_uart_dev_t *uart0;
static annex_hal_gpio_dev_t *gpio_portd;
static annex_hal_gpio_dev_t *gpio_portb; /* For LED */

static volatile uint8_t rx_data[8] = {0};
static volatile uint8_t rx_index = 0;
static volatile uint8_t rx_done = 0; /* set when 8 bytes received */
static volatile uint8_t rx_ok = 0;
const annex_gpio_pin_t led_pin = 5; /* PB5 = Arduino Uno pin 13 */

void setup() {
    sei();

    annex_hal_gpio_hw_desc_t gpio_portd_hw = {
        .base_addr     = ANNEX_GPIO_ATMEGA328P_PORTD_BASE,
        .num_pins      = 8,
        .platform_data = NULL,
    };

    gpio_portd = annex_hal_gpio_open(&gpio_portd_hw);

    annex_hal_gpio_hw_desc_t gpio_portb_hw = {
        .base_addr     = ANNEX_GPIO_ATMEGA328P_PORTB_BASE,
        .num_pins      = ANNEX_GPIO_ATMEGA328P_PORT_WIDTH,
        .platform_data = NULL,
    };

    gpio_portb = annex_hal_gpio_open(&gpio_portb_hw);

    annex_hal_gpio_set_mode(gpio_portb, led_pin, ANNEX_GPIO_MODE_OUTPUT_PP);

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
        .lin_baudrate = 115200U,
        .break_width = 13U,
        .flags = ANNEX_LIN_SLAVE,
    };

    lin_handle = annex_lin_open (&hw);
    if(annex_lin_config(lin_handle, &cfg) != LIN_OK)
       return;
    
    if(annex_lin_start(lin_handle) != LIN_OK)
        return;

    delay(150);

    /* Ensure LED off initially */
    annex_hal_gpio_write(gpio_portb, led_pin, ANNEX_GPIO_LOW);
    annex_hal_gpio_toggle(gpio_portb, led_pin);
    annex_hal_gpio_toggle(gpio_portb, led_pin);
}

void loop() { 
    loop_check_rx();
}

/* Forward declarations of callbacks used by the IRQ handler */
static void hdr_cb(uint8_t frame_id);
static void rx_cb(void);
static void st_cb(uint32_t status);

ISR(USART_RX_vect)
{
    annex_lin_handle_rx_irq(hdr_cb, rx_cb, st_cb);
}

/* act on completed reception in the main loop context */
void loop_check_rx(void)
{
    /* nothing to poll here — callbacks will update LED state */
}

static void hdr_cb(uint8_t frame_id)
{
    /* header arrived — we could act on it if needed */
    (void)frame_id;
}

static void rx_cb(void)
{
    annex_hal_uart_close(uart0);
    annex_uart_io_init(16000000UL, 115200, &uart_stream); 
    uint32_t dlc = 0, flags = 0, id = 0;
    uint8_t buf[8] = {0};
    annex_lin_get_last_response(buf, &dlc, &flags, &id);
    if (flags != 0 || dlc != 8) {
        annex_hal_gpio_write(gpio_portb, led_pin, ANNEX_GPIO_LOW);
        return;
    }

    for (uint8_t j = 0; j < 8; j++) {
        printf("%x\t",buf[j]);
    }
    printf("\r\n");
    uint8_t ok = 1;
    for (uint8_t i = 0; i < 8; i++) {
        if (buf[i] != (uint8_t)(i + 1)) {
            ok = 0;
            break;
        }
    }
    if (ok)
        annex_hal_gpio_write(gpio_portb, led_pin, ANNEX_GPIO_HIGH);
    else
        annex_hal_gpio_write(gpio_portb, led_pin, ANNEX_GPIO_HIGH);
}

static void st_cb(uint32_t status)
{
    /* indicate error by turning LED off */
    if (status != 0)
        annex_hal_gpio_write(gpio_portb, led_pin, ANNEX_GPIO_HIGH);
}
