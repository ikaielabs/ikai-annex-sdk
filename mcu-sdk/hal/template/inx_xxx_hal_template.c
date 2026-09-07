#include <stdint.h>

/*
 * XXX HAL template for porting Add-on Board SDK to a new platform.
 * Copy this file and replace TODO stubs with MCU-specific logic.
 */

void inx_hal_xxx_init(void)
{
    /* TODO: initialize XXX controller and settings */
}

int inx_hal_xxx_send(const uint8_t *data, uint16_t length)
{
    /* TODO: send 'length' bytes over XXX */
    (void)data;
    (void)length;
    return 0;
}

int inx_hal_xxx_receive(uint8_t *data, uint16_t length)
{
    /* TODO: receive up to 'length' bytes from XXX */
    (void)data;
    (void)length;
    return 0;
}
