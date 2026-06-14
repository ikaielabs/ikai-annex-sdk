#include "annex_circular_buffer.h"

void annex_cbuf_init(annex_circular_buf *cbuffer) {
    cbuffer->read_index = 0;
    cbuffer->write_index = 0;
}

void annex_cbuf_put(annex_circular_buf *cbuffer, uint8_t data) {
    cbuffer->buffer[cbuffer->write_index++] = data;
}

uint8_t annex_cbuf_get(annex_circular_buf *cbuffer) {
    return cbuffer->buffer[cbuffer->read_index++];
}

uint8_t annex_cbuf_data_available(annex_circular_buf *cbuffer) {
    return (cbuffer->write_index - cbuffer->read_index);
}
