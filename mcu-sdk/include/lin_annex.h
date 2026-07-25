#ifndef ANNEX_LIN_H
#define ANNEX_LIN_H

#include <stdint.h>
#include <stdbool.h>
#include "annex_hal_uart.h"

#ifdef __cplusplus
extern "C" {
#endif


/* LIN Status flags */
#define ANNEX_LIN_BIT_ERROR 128 /* Bit error when transmitting */
#define ANNEX_SYNCH_ERROR 64 /* A synch error */
#define ANNEX_LIN_PARITY_ERROR 32 /* ID parity error */
#define ANNEX_LIN_CSUM_ERROR 16 /* Checksum error */
#define ANNEX_LIN_WAKEUP_FRAME 4 /* A wake up frame was received. */
#define ANNEX_LIN_NODATA 8 /* No data, only a header */

/* LIN Config flags */
#define ANNEX_LIN_MASTER 1 /* The LIN interface will be a LIN MASTER, (Default will be SLAVE)*/
#define ANNEX_LIN_VARIABLE_DLC 2 /* When specified, turns variable message length ON so the \
                                    message length will depend on the message ID */
#define ANNEX_LIN_ENHANCED_CHECKSUM 4 /* LIN interface will use the "enhanced checksum" according to LIN 2.0 */

typedef enum {
    LIN_OK = 0,
    LIN_ERR = -1,
    LIN_NOT_INITIALIZED = -2
} annex_lin_status_t;

typedef struct {
    uint32_t lin_baudrate;
    uint8_t break_width;
    uint8_t flags;
} annex_lin_config_t;

typedef struct {
    annex_hal_uart_dev_t *uart_handle;     /* UART handle                     */
    void *platform_data;                  /* Anything else (optional)        */
} annex_lin_hw_desc_t;

typedef struct annex_lin_dev annex_lin_dev_t;

annex_lin_dev_t *annex_lin_open (const annex_lin_hw_desc_t *hw);
void annex_lin_close(annex_lin_dev_t *dev);
annex_lin_status_t annex_lin_start(annex_lin_dev_t *dev);
annex_lin_status_t annex_lin_config(annex_lin_dev_t *dev, annex_lin_config_t *cfg);
annex_lin_status_t annex_lin_write_message(annex_lin_dev_t *dev, uint32_t id, void *msg, uint32_t dlc, uint32_t timeout);
annex_lin_status_t annex_lin_request_message(annex_lin_dev_t *dev, uint32_t id, uint32_t timeout);
annex_lin_status_t annex_lin_read_message(annex_lin_dev_t *dev, uint32_t *id, void *msg, uint32_t *dlc, uint32_t *flags);
annex_lin_status_t annex_lin_read_message_wait(annex_lin_dev_t *dev, uint32_t *id, void *msg, uint32_t *dlc, uint32_t *flags, uint32_t timeout);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ANNEX_LIN_H */
