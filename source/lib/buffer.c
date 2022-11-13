#include "buffer.h"

void mBufferInit(struct mBuffer *buf) {
    buf->front = 0;
    buf->end = 0;
    buf->lock = xSemaphoreCreateBinary();
}

void mBufferPush(struct mBuffer *buf, message_t m) {
    xSemaphoreTakeRecursive(buf->lock, 0);
    // put message at buf->end and increment buf->end
    // no need to roll over at end of buffer, taken care of overflow
    buf->buf[buf->end++] = m;
    if (buf->size == 256) {
        buf->front++;
    } else {
        buf->size++;
    }
    xSemaphoreGiveRecursive(buf->lock);
}

message_t mBufferPop(struct mBuffer *buf) {
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