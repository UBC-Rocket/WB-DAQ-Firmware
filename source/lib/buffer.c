#include "buffer.h"


void m_buffer_init(struct m_buffer *buf) {
    buf->front = 0;
    buf->end = 0;
    buf->lock = xSemaphoreCreateBinary();
}

void m_buffer_push(struct m_buffer *buf, message_t m) {
    xSemaphoreTakeRecursive(buf->lock, 0);
    // put message at buf->end and increment buf->end
    buf->buf[buf->end++] = m;
    if (buf->size == 256) {
        buf->front++;
    } else {
        buf->size++;
    }
    xSemaphoreGiveRecursive(buf->lock);
}

message_t m_buffer_pop(struct m_buffer *buf) {
    xSemaphoreTakeRecursive(buf->lock, 0);
    if (!buf->size) {
        // return No_Command if buffer is empty
        return No_Command;
    }
    // get message at buf->front and increment buf->front
    message_t r = buf->buf[buf->front++];
    buf->size--;
    xSemaphoreGiveRecursive(buf->lock);
    return r;
}