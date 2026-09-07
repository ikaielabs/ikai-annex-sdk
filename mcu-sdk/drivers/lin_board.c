#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lin_board.h"
#include "inx_hal_uart.h"

struct inx_lin_board_dev {
    inx_hal_uart_dev_t *uart_handle;
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
 * inx_lin_board_config()/inx_lin_board_start(), via inx_hal_uart_lin_init().
 * Sending a break per frame should not stop/reconfigure/restart the
 * whole UART -- that was needlessly re-entering LIN mode (and racing
 * against in-flight transmissions) on every single frame.
 */
static inx_lin_board_status_t lin_send_break(inx_hal_uart_dev_t *uart,
                                           uint32_t break_duration,
                                           uint32_t bit_duration)
{
    (void)break_duration;
    (void)bit_duration;

    if (!uart)
        return INX_LIN_BOARD_STATUS_ERR;

    if (inx_hal_uart_send_break(uart) != 0)
        return INX_LIN_BOARD_STATUS_ERR;

    return INX_LIN_BOARD_STATUS_OK;
}

static inx_lin_board_status_t lin_detect_break(inx_hal_uart_dev_t *uart, uint32_t timeout)
{
    if (!uart)
        return INX_LIN_BOARD_STATUS_ERR;

    if (inx_hal_uart_wait_for_break(uart, timeout) != 0)
        return INX_LIN_BOARD_STATUS_ERR;

    return INX_LIN_BOARD_STATUS_OK;
}

static inx_lin_board_status_t lin_send_header(inx_lin_board_dev_t *dev, uint32_t id, uint32_t timeout)
{
    if (!dev || id > 0x3FU)
        return INX_LIN_BOARD_STATUS_ERR;

    uint8_t header[2] = {
        0x55,
        lin_compute_protected_id((uint8_t)id)
    };

    if (inx_hal_uart_write_fifo_u8(dev->uart_handle, header, sizeof(header), timeout) != 0U)
        return INX_LIN_BOARD_STATUS_ERR;

    dev->last_request_pid = header[1];
    return INX_LIN_BOARD_STATUS_OK;
}

static int lin_read_bytes_wait(inx_hal_uart_dev_t *uart, uint8_t *buf, uint32_t len, uint32_t timeout)
{
    if(inx_hal_uart_read_fifo_u8(uart, buf, len, timeout) != 0) {
        return -1;
    }

    return 0;
}

static inx_lin_board_status_t lin_read_header(inx_lin_board_dev_t *dev, uint8_t *id, uint32_t *flags, uint32_t timeout)
{
    if (!dev || !id || !flags)
        return INX_LIN_BOARD_STATUS_ERR;

    uint8_t byte = 0;
    uint32_t sync_timeout = 0xFF;
    while(--sync_timeout) {
      if(lin_read_bytes_wait(dev->uart_handle, &byte, 1, timeout) != 0)
    	  return INX_LIN_BOARD_STATUS_ERR;

      if (byte != 0x55 && byte != 0U) {
          (*flags) |= INX_LIN_BOARD_SYNCH_ERROR;
    	  break;
      }
      if(byte == 0x55){
    	/* Sync Received */
    	break;
      }
    }

    uint8_t pid = 0;
    if (lin_read_bytes_wait(dev->uart_handle, &pid, 1, timeout) != 0)
        return INX_LIN_BOARD_STATUS_ERR;

    uint8_t raw_id = 0;
    if (!lin_validate_protected_id(pid, &raw_id)){
    	 *flags |= INX_LIN_BOARD_PARITY_ERROR;
        return INX_LIN_BOARD_STATUS_ERR;
    }

    dev->last_request_id  = raw_id;
    dev->last_request_pid = pid;

    *id    = raw_id;
    *flags |= INX_LIN_BOARD_NODATA;

    return INX_LIN_BOARD_STATUS_OK;
}

static inx_lin_board_status_t lin_read_response(inx_lin_board_dev_t *dev, uint8_t *id, uint8_t *msg, uint8_t dlc, uint32_t *flags, uint32_t timeout)
{
    if (!dev || !id || !msg || !flags)
        return INX_LIN_BOARD_STATUS_ERR;

    uint8_t buffer[9];
    if(inx_hal_uart_read_fifo_u8(dev->uart_handle, buffer, (dlc+1), timeout) != 0) {
        return INX_LIN_BOARD_STATUS_ERR;
    }

    *flags &= ~(INX_LIN_BOARD_NODATA);

    uint8_t checksum = buffer[dlc];

    uint8_t expected = lin_compute_checksum(dev->last_request_pid, buffer, dlc, (dev->flags & INX_LIN_BOARD_ENHANCED_CHECKSUM) != 0);
    if (checksum != expected) {
        *flags |= INX_LIN_BOARD_CSUM_ERROR;
        return INX_LIN_BOARD_STATUS_ERR;
    }

    memcpy(msg, buffer, dlc);
    *id    = dev->last_request_id;
    *flags = 0;

    return INX_LIN_BOARD_STATUS_OK;
}

inx_lin_board_dev_t *inx_lin_board_open (const inx_lin_board_hw_desc_t *hw)
{
    if (!hw)
        return NULL;

    inx_lin_board_dev_t *dev = (inx_lin_board_dev_t *)calloc(1, sizeof(*dev));
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

void inx_lin_board_close(inx_lin_board_dev_t *dev)
{
    if (!dev)
        return;

    inx_hal_uart_close(dev->uart_handle);
    free(dev);
}

inx_lin_board_status_t inx_lin_board_config(inx_lin_board_dev_t *dev, inx_lin_board_config_t *cfg)
{
    if (!dev)
        return INX_LIN_BOARD_STATUS_NOT_INITIALIZED;

    if (!cfg)
        return INX_LIN_BOARD_STATUS_ERR;

    if (inx_hal_uart_stop(dev->uart_handle) != 0)
        return INX_LIN_BOARD_STATUS_ERR;

    inx_hal_uart_config_t uart_cfg = {
        .baud_rate    = cfg->lin_baudrate,
        .data_bits    = 8,
        .stop_bits    = 1,
        .parity       = 0,
        .flow_control = 0
    };

    if (inx_hal_uart_config(dev->uart_handle, &uart_cfg) != 0)
        return INX_LIN_BOARD_STATUS_ERR;

    if (inx_hal_uart_lin_init(dev->uart_handle, cfg->break_width) != 0)
        return INX_LIN_BOARD_STATUS_ERR;

    dev->flags = cfg->flags;
    dev->break_duration = (uint32_t)((float)(cfg->break_width * 1000000U) / (float)cfg->lin_baudrate);
    dev->bit_duration   = (uint32_t)((float)(1U * 1000000U) / (float)cfg->lin_baudrate);
    dev->last_request_id  = 0;
    dev->last_request_pid = 0;

    return INX_LIN_BOARD_STATUS_OK;
}

inx_lin_board_status_t inx_lin_board_start(inx_lin_board_dev_t *dev)
{
    if (!dev)
        return INX_LIN_BOARD_STATUS_NOT_INITIALIZED;

    if (inx_hal_uart_start(dev->uart_handle, true, true) != 0)
        return INX_LIN_BOARD_STATUS_ERR;

    return INX_LIN_BOARD_STATUS_OK;
}

inx_lin_board_status_t inx_lin_board_stop(inx_lin_board_dev_t *dev)
{
    if (!dev)
        return INX_LIN_BOARD_STATUS_NOT_INITIALIZED;

    if (inx_hal_uart_stop(dev->uart_handle) != 0)
        return INX_LIN_BOARD_STATUS_ERR;

    return INX_LIN_BOARD_STATUS_OK;
}

inx_lin_board_status_t inx_lin_board_write_message(inx_lin_board_dev_t *dev, uint8_t id, uint8_t *msg, uint32_t dlc, uint32_t timeout)
{
    if (!dev)
        return INX_LIN_BOARD_STATUS_NOT_INITIALIZED;

    if (dlc > 8)
        return INX_LIN_BOARD_STATUS_ERR;

    if (dev->flags & INX_LIN_BOARD_MASTER) {
        if (!msg || dlc == 0 || id > 0x3FU)
            return INX_LIN_BOARD_STATUS_ERR;

        if (lin_send_break(dev->uart_handle, dev->break_duration, dev->bit_duration) != INX_LIN_BOARD_STATUS_OK)
            return INX_LIN_BOARD_STATUS_ERR;

        if (lin_send_header(dev, id, timeout) != INX_LIN_BOARD_STATUS_OK)
            return INX_LIN_BOARD_STATUS_ERR;

        if (inx_hal_uart_write_fifo_u8(dev->uart_handle, msg, dlc, timeout) != 0)
            return INX_LIN_BOARD_STATUS_ERR;

        uint8_t checksum = lin_compute_checksum(dev->last_request_pid, msg, dlc, (dev->flags & INX_LIN_BOARD_ENHANCED_CHECKSUM) != 0);
        if (inx_hal_uart_write_fifo_u8(dev->uart_handle, &checksum, 1, timeout) != 0)
            return INX_LIN_BOARD_STATUS_ERR;

        dev->last_request_id = id;
        return INX_LIN_BOARD_STATUS_OK;
    }

    else { /* SLAVE */
        if (!msg)
            return INX_LIN_BOARD_STATUS_ERR;

        if (dlc == 0)
            return INX_LIN_BOARD_STATUS_OK;

        if (inx_hal_uart_write_fifo_u8(dev->uart_handle, msg, dlc, timeout) != (int)dlc)
            return INX_LIN_BOARD_STATUS_ERR;

        uint8_t checksum = lin_compute_checksum(dev->last_request_pid, msg, dlc, (dev->flags & INX_LIN_BOARD_ENHANCED_CHECKSUM) != 0);
        if (inx_hal_uart_write_fifo_u8(dev->uart_handle, &checksum, 1, timeout) != 0)
            return INX_LIN_BOARD_STATUS_ERR;

        return INX_LIN_BOARD_STATUS_OK;
    }

    return INX_LIN_BOARD_STATUS_ERR;
}

inx_lin_board_status_t inx_lin_board_request_message(inx_lin_board_dev_t *dev, uint8_t id, uint32_t timeout)
{
    if (!dev)
        return INX_LIN_BOARD_STATUS_NOT_INITIALIZED;

    if (!(dev->flags & INX_LIN_BOARD_MASTER))
        return INX_LIN_BOARD_STATUS_ERR;

    if (id > 0x3FU)
        return INX_LIN_BOARD_STATUS_ERR;

    if (lin_send_break(dev->uart_handle, dev->break_duration, dev->bit_duration) != INX_LIN_BOARD_STATUS_OK)
        return INX_LIN_BOARD_STATUS_ERR;

    if (lin_send_header(dev, id, timeout) != INX_LIN_BOARD_STATUS_OK)
        return INX_LIN_BOARD_STATUS_ERR;

    dev->last_request_id = id;
    return INX_LIN_BOARD_STATUS_OK;
}

inx_lin_board_status_t inx_lin_board_read_message(inx_lin_board_dev_t *dev, uint8_t *id, uint8_t *msg, uint8_t dlc, uint32_t *flags)
{
    return inx_lin_board_read_message_wait(dev, id, msg, dlc, flags, 0);
}

inx_lin_board_status_t inx_lin_board_read_message_wait(inx_lin_board_dev_t *dev, uint8_t *id, uint8_t *msg, uint8_t dlc, uint32_t *flags, uint32_t timeout)
{
    if (!dev)
        return INX_LIN_BOARD_STATUS_NOT_INITIALIZED;

    if (!id || !dlc || !flags)
        return INX_LIN_BOARD_STATUS_ERR;
    
    if (dlc > 8)
        return INX_LIN_BOARD_STATUS_ERR;

    if (dev->flags & INX_LIN_BOARD_MASTER)
        return lin_read_response(dev, id, msg, dlc, flags, timeout);
    else { /* SLAVE */
    	if(lin_detect_break(dev->uart_handle, timeout) != INX_LIN_BOARD_STATUS_OK)
            return INX_LIN_BOARD_STATUS_ERR;

        if(lin_read_header(dev, id, flags, timeout) != INX_LIN_BOARD_STATUS_OK)
            return INX_LIN_BOARD_STATUS_ERR;

        if(lin_read_response(dev, id, msg, dlc, flags, timeout) != INX_LIN_BOARD_STATUS_OK)
            return INX_LIN_BOARD_STATUS_ERR;

        return INX_LIN_BOARD_STATUS_OK;
    }
}


