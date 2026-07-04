#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "annex_lin.h"
#include "annex_hal_uart.h"
#include "annex_hal_gpio.h"

#ifdef __AVR__
#include <util/delay.h>
#endif

struct annex_lin_dev {
    annex_hal_uart_dev_t *uart_handle;
    annex_hal_gpio_dev_t *gpio_handle;
    int8_t               gpio_tx_pin;
    uint32_t flags;
    uint32_t break_duration;
    uint32_t bit_duration;
    uint32_t last_request_id;
};

static uint8_t lin_compute_protected_id(uint8_t id)
{
    uint8_t p0 = ((id >> 0) ^ (id >> 1) ^ (id >> 2) ^ (id >> 4)) & 0x01;
    uint8_t p1 = (~((id >> 1) ^ (id >> 3) ^ (id >> 4) ^ (id >> 5)) & 0x01);
    return (uint8_t)(id | (p0 << 6) | (p1 << 7));
}

static bool lin_validate_protected_id(uint8_t pid, uint8_t *id)
{
    uint8_t raw = pid & 0x3FU;
    uint8_t expected = lin_compute_protected_id(raw);
    if (expected != pid)
        return false;
    *id = raw;
    return true;
}

static uint8_t lin_compute_checksum(uint8_t id, const uint8_t *data, uint32_t len, bool enhanced)
{
    uint16_t sum = 0;

    if (enhanced)
        sum += id;

    for (uint32_t i = 0; i < len; i++)
        sum += data[i];

    sum = (uint8_t)(sum & 0xFF) + (uint8_t)(sum >> 8);
    sum = (uint8_t)(sum & 0xFF) + (uint8_t)(sum >> 8);

    return (uint8_t)(~sum);
}

static void lin_delay_us(uint32_t us)
{
#ifdef __AVR__
    while (us--) {
        _delay_us(1);
    }
#else
    volatile uint32_t count = us * 5U;
    while (count--) {
        ;
    }
#endif
}

static annex_lin_status_t lin_send_break(annex_hal_uart_dev_t *uart,
                                           annex_hal_gpio_dev_t *gpio,
                                           int8_t gpio_tx_pin,
                                           uint32_t break_duration,
                                           uint32_t bit_duration)
{
    if (!uart || !gpio || gpio_tx_pin < 0 || bit_duration == 0)
        return LIN_ERR;

    /* Stop UART so the pin can be driven manually for the break. */
    if (annex_hal_uart_stop(uart) != 0)
        return LIN_ERR;

    if (annex_hal_gpio_set_mode(gpio, (annex_gpio_pin_t)gpio_tx_pin, ANNEX_GPIO_MODE_OUTPUT_PP) != ANNEX_HAL_GPIO_OK)
        return LIN_ERR;

    if (annex_hal_gpio_write(gpio, (annex_gpio_pin_t)gpio_tx_pin, ANNEX_GPIO_LOW) != ANNEX_HAL_GPIO_OK)
        return LIN_ERR;

    lin_delay_us(break_duration);

    if (annex_hal_gpio_write(gpio, (annex_gpio_pin_t)gpio_tx_pin, ANNEX_GPIO_HIGH) != ANNEX_HAL_GPIO_OK)
        return LIN_ERR;

    /* Delimiter: leave the line high for at least one bit time. */
    lin_delay_us(bit_duration);

    /* Release GPIO pin so UART TX can take over the line. */
    annex_hal_gpio_set_mode(gpio, (annex_gpio_pin_t)gpio_tx_pin, ANNEX_GPIO_MODE_INPUT);

    if (annex_hal_uart_start(uart, true, true) != 0)
        return LIN_ERR;

    return LIN_OK;
}

static annex_lin_status_t lin_send_header(annex_lin_dev_t *dev, uint32_t id)
{
    if (!dev || id > 0x3FU)
        return LIN_ERR;

    uint8_t header[2] = {
        0x55,
        lin_compute_protected_id((uint8_t)id)
    };

    if (annex_hal_uart_write_fifo_u8(dev->uart_handle, header, sizeof(header)) != (int)sizeof(header))
        return LIN_ERR;

    return LIN_OK;
}

static int lin_read_bytes_wait(annex_hal_uart_dev_t *uart, uint8_t *buf, uint32_t len, uint32_t timeout)
{
    if (!uart || !buf || len == 0)
        return -1;

    uint32_t total = 0;

    while (total < len) {
        int n = annex_hal_uart_read_fifo_u8(uart, buf + total, len - total);
        if (n > 0) {
            total += (uint32_t)n;
            continue;
        }

        if (timeout == 0)
            break;

        if (!(annex_hal_uart_get_irq_status(uart) & ANNEX_HAL_UART_IRQ_RX_READY)) {
            timeout--; 
            continue;
        }
    }

    return (int)total;
}

static annex_lin_status_t lin_read_header(annex_lin_dev_t *dev, uint32_t *id, uint32_t *dlc, uint32_t *flags, uint32_t timeout)
{
    if (!dev || !id || !dlc || !flags)
        return LIN_ERR;

    uint8_t byte = 0;
    while (1) {
        int n = lin_read_bytes_wait(dev->uart_handle, &byte, 1, timeout);
        if (n != 1)
            return LIN_ERR;

        if (byte == 0x55)
            break;
    }

    uint8_t pid = 0;
    if (lin_read_bytes_wait(dev->uart_handle, &pid, 1, timeout) != 1)
        return LIN_ERR;

    uint8_t raw_id = 0;
    if (!lin_validate_protected_id(pid, &raw_id))
        return LIN_ERR;

    *id    = raw_id;
    *dlc   = 0;
    *flags = ANNEX_LIN_NODATA;

    return LIN_OK;
}

static annex_lin_status_t lin_read_response(annex_lin_dev_t *dev, uint32_t *id, void *msg, uint32_t *dlc, uint32_t *flags, uint32_t timeout)
{
    if (!dev || !id || !msg || !dlc || !flags)
        return LIN_ERR;

    uint8_t buffer[9];
    uint32_t received = 0;

    while (received < sizeof(buffer)) {
        int n = annex_hal_uart_read_fifo_u8(dev->uart_handle, buffer + received, (uint32_t)sizeof(buffer) - received);
        if (n > 0) {
            received += (uint32_t)n;
            continue;
        }

        if (received >= 2)
            break;

        if (timeout == 0)
            break;

        timeout--;
    }

    if (received < 2)
        return LIN_ERR;

    uint32_t data_len = received - 1;
    if (data_len > 8)
        data_len = 8;

    uint8_t checksum = buffer[received - 1];
    uint8_t expected = lin_compute_checksum((uint8_t)dev->last_request_id, buffer, data_len, (dev->flags & ANNEX_LIN_ENHANCED_CHECKSUM) != 0);
    if (checksum != expected) {
        *flags = ANNEX_LIN_CSUM_ERROR;
        return LIN_ERR;
    }

    memcpy(msg, buffer, data_len);
    *id    = dev->last_request_id;
    *dlc   = data_len;
    *flags = 0;

    return LIN_OK;
}

annex_lin_dev_t *annex_lin_open (const annex_lin_hw_desc_t *hw)
{
    if (!hw)
        return NULL;

    annex_lin_dev_t *dev = (annex_lin_dev_t *)calloc(1, sizeof(*dev));
    if (!dev)
        return NULL;

    annex_hal_uart_hw_desc_t uart_hw = {
        .base_addr    = hw->uart_base_addr,
        .input_clk_hz = hw->input_clk_hz,
        .tx_pin       = 1,
        .rx_pin       = 2,
        .platform_data= hw->platform_data,
    };

    annex_hal_gpio_hw_desc_t gpio_hw = {
        .base_addr     = hw->gpio_base_addr,
        .num_pins      = 8,
        .platform_data = NULL,
    };

    dev->uart_handle = annex_hal_uart_open(&uart_hw);
    if (!dev->uart_handle) {
        free(dev);
        return NULL;
    }

    dev->gpio_tx_pin = hw->gpio_tx_pin;
    dev->gpio_handle = NULL;
    if (hw->gpio_base_addr != 0U && hw->gpio_tx_pin >= 0) {
        dev->gpio_handle = annex_hal_gpio_open(&gpio_hw);
        if (!dev->gpio_handle) {
            annex_hal_uart_close(dev->uart_handle);
            free(dev);
            return NULL;
        }
    }

    return dev;
}

void annex_lin_close(annex_lin_dev_t *dev)
{
    if (!dev)
        return;

    annex_hal_uart_close(dev->uart_handle);
    if (dev->gpio_handle)
        annex_hal_gpio_close(dev->gpio_handle);
    free(dev);
}

annex_lin_status_t annex_lin_config(annex_lin_dev_t *dev, annex_lin_config_t *cfg)
{
    if (!dev)
        return LIN_NOT_INITIALIZED;

    if (!cfg)
        return LIN_ERR;

    if (annex_hal_uart_stop(dev->uart_handle) != 0)
        return LIN_ERR;

    annex_hal_uart_config_t uart_cfg = {
        .baud_rate    = cfg->lin_baudrate,
        .data_bits    = 8,
        .stop_bits    = 1,
        .parity       = 0,
        .flow_control = 0
    };

    if (annex_hal_uart_config(dev->uart_handle, &uart_cfg) != 0)
        return LIN_ERR;

    dev->flags = cfg->flags;
    dev->break_duration = (uint32_t)((float)(cfg->break_width * 1000000U) / (float)cfg->lin_baudrate);
    dev->bit_duration   = (uint32_t)((float)(1U * 1000000U) / (float)cfg->lin_baudrate);
    dev->last_request_id = 0;

    return LIN_OK;
}

annex_lin_status_t annex_lin_start(annex_lin_dev_t *dev)
{
    if (!dev)
        return LIN_NOT_INITIALIZED;

    if (annex_hal_uart_start(dev->uart_handle, true, true) != 0)
        return LIN_ERR;

    return LIN_OK;
}

annex_lin_status_t annex_lin_stop(annex_lin_dev_t *dev)
{
    if (!dev)
        return LIN_NOT_INITIALIZED;

    if (annex_hal_uart_stop(dev->uart_handle) != 0)
        return LIN_ERR;

    return LIN_OK;
}

annex_lin_status_t annex_lin_write_message(annex_lin_dev_t *dev, uint32_t id, void *msg, uint32_t dlc)
{
    if (!dev)
        return LIN_NOT_INITIALIZED;

    if (dlc > 8)
        return LIN_ERR;

    if (dev->flags & ANNEX_LIN_MASTER) {
        if (!msg || dlc == 0 || id > 0x3FU)
            return LIN_ERR;

        if (lin_send_break(dev->uart_handle, dev->gpio_handle, dev->gpio_tx_pin, dev->break_duration, dev->bit_duration) != LIN_OK)
            return LIN_ERR;

        if (lin_send_header(dev, id) != LIN_OK)
            return LIN_ERR;

        uint8_t *payload = (uint8_t *)msg;
        if (annex_hal_uart_write_fifo_u8(dev->uart_handle, payload, dlc) != (int)dlc)
            return LIN_ERR;

        uint8_t checksum = lin_compute_checksum((uint8_t)id, payload, dlc, (dev->flags & ANNEX_LIN_ENHANCED_CHECKSUM) != 0);
        if (annex_hal_uart_write_fifo_u8(dev->uart_handle, &checksum, 1) != 1)
            return LIN_ERR;

        dev->last_request_id = id;
        return LIN_OK;
    }

    if (dev->flags & ANNEX_LIN_SLAVE) {
        if (!msg)
            return LIN_ERR;

        if (dlc == 0)
            return LIN_OK;

        uint8_t *payload = (uint8_t *)msg;
        if (annex_hal_uart_write_fifo_u8(dev->uart_handle, payload, dlc) != (int)dlc)
            return LIN_ERR;

        uint8_t checksum = lin_compute_checksum((uint8_t)dev->last_request_id, payload, dlc, (dev->flags & ANNEX_LIN_ENHANCED_CHECKSUM) != 0);
        if (annex_hal_uart_write_fifo_u8(dev->uart_handle, &checksum, 1) != 1)
            return LIN_ERR;

        return LIN_OK;
    }

    return LIN_ERR;
}

annex_lin_status_t annex_lin_request_message(annex_lin_dev_t *dev, uint32_t id)
{
    if (!dev)
        return LIN_NOT_INITIALIZED;

    if (!(dev->flags & ANNEX_LIN_MASTER))
        return LIN_ERR;

    if (id > 0x3FU)
        return LIN_ERR;

    if (lin_send_break(dev->uart_handle, dev->gpio_handle, dev->gpio_tx_pin, dev->break_duration, dev->bit_duration) != LIN_OK)
        return LIN_ERR;

    if (lin_send_header(dev, id) != LIN_OK)
        return LIN_ERR;

    dev->last_request_id = id;
    return LIN_OK;
}

annex_lin_status_t annex_lin_read_message(annex_lin_dev_t *dev, uint32_t *id, void *msg, uint32_t *dlc, uint32_t *flags)
{
    return annex_lin_read_message_wait(dev, id, msg, dlc, flags, 0);
}

annex_lin_status_t annex_lin_read_message_wait(annex_lin_dev_t *dev, uint32_t *id, void *msg, uint32_t *dlc, uint32_t *flags, uint32_t timeout)
{
    if (!dev)
        return LIN_NOT_INITIALIZED;

    if (!id || !dlc || !flags)
        return LIN_ERR;

    if (dev->flags & ANNEX_LIN_MASTER)
        return lin_read_response(dev, id, msg, dlc, flags, timeout);

    if (dev->flags & ANNEX_LIN_SLAVE)
        return lin_read_header(dev, id, dlc, flags, timeout);

    return LIN_ERR;
}


