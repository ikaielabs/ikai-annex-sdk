#include <avr/interrupt.h>
#include "annex_hal_uart.h"
#include "annex_uart_io.h"
#include "annex_circular_buffer.h"

annex_circular_buf annex_uart_buffer;
static annex_hal_uart_dev_t *uart0;

static int uart_putchar(char data, FILE *stream);
static int uart_getchar(FILE *stream);

void annex_uart_io_init(uint32_t input_clk_hz, uint32_t baud, FILE *uart_stream) {

    /* Initialize the UART buffer */
    annex_cbuf_init(&annex_uart_buffer);

    annex_hal_uart_hw_desc_t hw = {
        .base_addr    = (uintptr_t)&UCSR0A,            
        .input_clk_hz = input_clk_hz,
        .tx_pin       = 1,           /* board-specific */
        .rx_pin       = 2, 
    };

    annex_hal_uart_config_t cfg = {
        .baud_rate    = baud,
        .data_bits    = 8,
        .stop_bits    = 1,
        .parity       = 0,
        .flow_control = 0
    };

    uart0 = annex_hal_uart_open(&hw);

    int ret = annex_hal_uart_config(uart0, &cfg);
    
    annex_hal_uart_enable_irq(uart0, ANNEX_HAL_UART_IRQ_RX_READY);
    
    annex_hal_uart_start(uart0, true, true);

    /* Redirect STDOUT and STDIN to and from our own function */
    if(uart_stream != NULL) {
        fdev_setup_stream(uart_stream, uart_putchar, uart_getchar, _FDEV_SETUP_RW);
        stdin = uart_stream;
        stdout = uart_stream;

    }
}

static int uart_putchar(char data, FILE *stream) {
    annex_hal_uart_write_fifo_u8(uart0, &data, 1);

    return 0;
}

static int uart_getchar(FILE *stream) {
    static int line[256];
    static uint8_t write_index = 0;
    static uint8_t read_index;
    int ch;

    if(write_index == 0) {
        do {
            while(!annex_cbuf_data_available(&annex_uart_buffer));

            ch = annex_cbuf_get(&annex_uart_buffer);

            switch(ch) {
                case '\b': //handle backspace
                    if(write_index > 0)
                    {
                        write_index--;
                    }
                    break;

                case 0x1A: //handle CTRL Z
                    ch = EOF;
                    line[write_index++] = ch;
                    break;

                case '\r':
                case '\n': //handle line feed
                    if(write_index > 0) {
                        ch = '\n';
                        line[write_index++] = ch;
                    }
                    break;

                default: // store the rest of the character
                    line[write_index++] = ch;
                    break;
            }
        }while (ch != '\n' && (ch != EOF));

        read_index = 0;
    }

    ch = line[read_index++];

    if(read_index == write_index) {
        write_index = 0;
    }

    return ch;
}

ISR(USART_RX_vect) {
    //send each received byte to the buffer
    uint8_t data;
    annex_hal_uart_read_fifo_u8(uart0, &data, 1);
    annex_cbuf_put(&annex_uart_buffer, data);
}