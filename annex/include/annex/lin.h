#ifndef ANNEX_LIN_H
#define ANNEX_LIN_H

#include <stdint.h>

typedef enum {
    ANNEX_LIN_STATUS_OK = 0,
    ANNEX_LIN_STATUS_ERROR,
} annex_lin_status_t;

void annex_lin_init(void);
annex_lin_status_t annex_lin_send(const uint8_t *data, uint16_t length);
void annex_lin_process(void);

#endif /* ANNEX_LIN_H */
