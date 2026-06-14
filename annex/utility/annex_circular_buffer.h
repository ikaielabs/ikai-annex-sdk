#ifndef ANNEX_CIRCULAR_BUFFER_H
#define ANNEX_CIRCULAR_BUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t buffer[256];
    volatile uint8_t write_index;
    volatile uint8_t read_index;
} annex_circular_buf;

void annex_cbuf_init(annex_circular_buf *cbuffer);
void annex_cbuf_put(annex_circular_buf *cbuffer, uint8_t data);
uint8_t annex_cbuf_get(annex_circular_buf *cbuffer);
uint8_t annex_cbuf_data_available(annex_circular_buf *cbuffer);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ANNEX_CIRCULAR_BUFFER_H */