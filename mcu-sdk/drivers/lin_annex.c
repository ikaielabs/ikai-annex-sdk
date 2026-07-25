#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lin_annex.h"
#include "annex_hal_uart.h"

struct annex_lin_dev {
    annex_hal_uart_dev_t *uart_handle;
    uint32_t flags;
    uint32_t break_duration;
    uint32_t bit_duration;
    uint32_t last_request_id;
    uint8_t last_request_pid;   /* protected ID of the last header sent,
                                   needed for the enhanced checksum */
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

/*
 * LIN mode and break-detection length are configured once, in
 * annex_lin_config()/annex_lin_start(), via annex_hal_uart_lin_init().
 * Sending a break per frame should not stop/reconfigure/restart the
 * whole UART -- that was needlessly re-entering LIN mode (and racing
 * against in-flight transmissions) on every single frame.
 */
static annex_lin_status_t lin_send_break(annex_hal_uart_dev_t *uart,
                                           uint32_t break_duration,
                                           uint32_t bit_duration)
{
    (void)break_duration;
    (void)bit_duration;

    if (!uart)
        return LIN_ERR;

    if (annex_hal_uart_send_break(uart) != 0)
        return LIN_ERR;

    return LIN_OK;
}

static annex_lin_status_t lin_send_header(annex_lin_dev_t *dev, uint32_t id, uint32_t timeout)
{
    if (!dev || id > 0x3FU)
        return LIN_ERR;

    uint8_t header[2] = {
        0x55,
        lin_compute_protected_id((uint8_t)id)
    };

    if (annex_hal_uart_write_fifo_u8(dev->uart_handle, header, sizeof(header), timeout) != 0U)
        return LIN_ERR;

    dev->last_request_pid = header[1];
    return LIN_OK;
}

static int lin_read_bytes_wait(annex_hal_uart_dev_t *uart, uint8_t *buf, uint32_t len, uint32_t timeout)
{
    if (!uart || !buf || len == 0)
        return -1;
    
    if(annex_hal_uart_read_fifo_u8(uart, buf, len, timeout) == 0) {
        return -1;
    }

    return 0;
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

    dev->last_request_id  = raw_id;
    dev->last_request_pid = pid;

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
    if(annex_hal_uart_read_fifo_u8(dev->uart_handle, buffer, *dlc, timeout) != 0) {
        return LIN_ERR;
    }

    uint32_t data_len = *dlc;
    if (data_len > 8)
        data_len = 8;

    uint8_t checksum = buffer[*dlc];
    uint8_t expected = lin_compute_checksum(dev->last_request_pid, buffer, data_len, (dev->flags & ANNEX_LIN_ENHANCED_CHECKSUM) != 0);
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

    /* Get the UART handle */
    dev->uart_handle = hw->uart_handle;

    if (!dev->uart_handle) {
        free(dev);
        return NULL;
    }

    return dev;
}

void annex_lin_close(annex_lin_dev_t *dev)
{
    if (!dev)
        return;

    annex_hal_uart_close(dev->uart_handle);
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

    if (annex_hal_uart_lin_init(dev->uart_handle, cfg->break_width) != 0)
        return LIN_ERR;

    dev->flags = cfg->flags;
    dev->break_duration = (uint32_t)((float)(cfg->break_width * 1000000U) / (float)cfg->lin_baudrate);
    dev->bit_duration   = (uint32_t)((float)(1U * 1000000U) / (float)cfg->lin_baudrate);
    dev->last_request_id  = 0;
    dev->last_request_pid = 0;

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

annex_lin_status_t annex_lin_write_message(annex_lin_dev_t *dev, uint32_t id, void *msg, uint32_t dlc, uint32_t timeout)
{
    if (!dev)
        return LIN_NOT_INITIALIZED;

    if (dlc > 8)
        return LIN_ERR;

    if (dev->flags & ANNEX_LIN_MASTER) {
        if (!msg || dlc == 0 || id > 0x3FU)
            return LIN_ERR;

        if (lin_send_break(dev->uart_handle, dev->break_duration, dev->bit_duration) != LIN_OK)
            return LIN_ERR;

        if (lin_send_header(dev, id, timeout) != LIN_OK)
            return LIN_ERR;

        uint8_t *payload = (uint8_t *)msg;
        if (annex_hal_uart_write_fifo_u8(dev->uart_handle, payload, dlc, timeout) != 0)
            return LIN_ERR;

        uint8_t checksum = lin_compute_checksum(dev->last_request_pid, payload, dlc, (dev->flags & ANNEX_LIN_ENHANCED_CHECKSUM) != 0);
        if (annex_hal_uart_write_fifo_u8(dev->uart_handle, &checksum, 1, timeout) != 0)
            return LIN_ERR;

        dev->last_request_id = id;
        return LIN_OK;
    }

    else { /* SLAVE */
        if (!msg)
            return LIN_ERR;

        if (dlc == 0)
            return LIN_OK;

        uint8_t *payload = (uint8_t *)msg;
        if (annex_hal_uart_write_fifo_u8(dev->uart_handle, payload, dlc, timeout) != (int)dlc)
            return LIN_ERR;

        uint8_t checksum = lin_compute_checksum(dev->last_request_pid, payload, dlc, (dev->flags & ANNEX_LIN_ENHANCED_CHECKSUM) != 0);
        if (annex_hal_uart_write_fifo_u8(dev->uart_handle, &checksum, 1, timeout) != 0)
            return LIN_ERR;

        return LIN_OK;
    }

    return LIN_ERR;
}

annex_lin_status_t annex_lin_request_message(annex_lin_dev_t *dev, uint32_t id, uint32_t timeout)
{
    if (!dev)
        return LIN_NOT_INITIALIZED;

    if (!(dev->flags & ANNEX_LIN_MASTER))
        return LIN_ERR;

    if (id > 0x3FU)
        return LIN_ERR;

    if (lin_send_break(dev->uart_handle, dev->break_duration, dev->bit_duration) != LIN_OK)
        return LIN_ERR;

    if (lin_send_header(dev, id, timeout) != LIN_OK)
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
    else /* SLAVE */
        return lin_read_header(dev, id, dlc, flags, timeout);

    return LIN_ERR;
}


