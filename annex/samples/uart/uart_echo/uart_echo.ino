#include <LibPrintf.h>
#include "annex_hal_uart.h"

static annex_hal_uart_dev_t *uart0;

// ── Custom Print class that routes to HAL ────────────────────
class AnnexHalUartPrint : public Print {
public:
    size_t write(uint8_t c) override {
        annex_hal_uart_write_fifo_u8(uart0, &c, 1);
        return 1;
    }
    size_t write(const uint8_t *buf, size_t len) override {
        annex_hal_uart_write_fifo_u8(uart0, buf, len);
        return len;
    }
};

static AnnexHalUartPrint annexHalPrint;

void setup() {

    annex_hal_uart_hw_desc_t hw = {
        .base_addr    = (uintptr_t)&UCSR0A,            
        .input_clk_hz = 16000000UL,
        .tx_pin       = 1,           /* board-specific */
        .rx_pin       = 2, 
    };

    annex_hal_uart_config_t cfg = {
        .baud_rate    = 115200,
        .data_bits    = 8,
        .stop_bits    = 1,
        .parity       = 0,
        .flow_control = 0
    };

    /* Open Uart */
    uart0 = annex_hal_uart_open(&hw);
    int ret = annex_hal_uart_config(uart0, &cfg);
    annex_hal_uart_start(uart0, true, true);

    printf_init(annexHalPrint);

    printf("=== UART HAL Test ===\n");
    //printf("[TEST] open:   %s\n", uart0 ? "PASS" : "FAIL");

#if 0
    printf("[TEST] hal_uart_open:   %s\n", uart0 ? "PASS" : "FAIL");

    // Test 2 — config
    int ret = annex_hal_uart_config(uart0, &cfg);
    printf("[TEST] hal_uart_config: %s (ret=%d)\n", ret == 0 ? "PASS" : "FAIL", ret);

    // Test 3 — start
    ret = annex_hal_uart_start(uart0, true, true);
    printf("[TEST] hal_uart_start:  %s (ret=%d)\n", ret == 0 ? "PASS" : "FAIL", ret);

    // Test 4 — write
    uint8_t tx[] = { 0xA5, 0x5A, 0x01, 0xFF };
    ret = annex_hal_uart_write_fifo_u8(uart0, tx, sizeof(tx));
    printf("[TEST] hal_uart_write:  %s (sent=%d)\n", ret == 4 ? "PASS" : "FAIL", ret);

    delay(100);   // wait for loopback (TX pin wired to RX pin)

    // Test 5 — read back (requires TX→RX loopback wire)
    uint8_t rx[4] = {};
    ret = annex_hal_uart_read_fifo_u8(uart0, rx, sizeof(rx));
    bool match = (ret == 4) && (memcmp(tx, rx, 4) == 0);
    printf("[TEST] hal_uart_read:   %s (got=%d)\n", match ? "PASS" : "FAIL", ret);
    printf("       TX: %02X %02X %02X %02X\r\n", tx[0], tx[1], tx[2], tx[3]);
    printf("       RX: %02X %02X %02X %02X\r\n", rx[0], rx[1], rx[2], rx[3]);
#endif
}

void loop() {}