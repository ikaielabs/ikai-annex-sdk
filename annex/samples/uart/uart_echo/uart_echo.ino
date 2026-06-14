#include <avr/io.h>
#include "annex_uart_io.h"
#include <avr/interrupt.h>

FILE uart_stream;
char data[100];   

void setup() {
    sei();
    annex_uart_io_init(16000000UL, 115200, &uart_stream); 
}

void loop() {

    printf("Enter any string\r\n");
    scanf("%s", data);
    printf("Echo: %s\r\n",data);
}
