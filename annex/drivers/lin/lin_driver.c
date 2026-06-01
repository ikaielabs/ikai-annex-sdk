#include <stdint.h>
#include "annex/lin.h"

/*
 * LIN driver prototype implementation.
 * Replace these stubs with actual hardware-specific code.
 */

void lin_driver_init(void)
{
    /* TODO: initialize LIN hardware and protocol state */
}

void lin_driver_send_frame(const uint8_t *data, uint16_t length)
{
    /* TODO: transmit a LIN frame on the configured bus */
    (void)data;
    (void)length;
}

void lin_driver_process(void)
{
    /* TODO: poll LIN hardware and handle incoming frames */
}
