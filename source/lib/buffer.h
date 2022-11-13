#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdint.h>
#include <stdbool.h>
#include "message_types.h"
#include "FreeRTOS.h"
#include "semphr.h"

// NOTE: if you make this value larger, you have to update the types for front and back in the c_buffer
// you also have to update how we update the front and end pointers when doing operations
#define MESSAGE_BUFFER_SIZE 256

/**
 * A structure for storing messages. On overflow, will push the front pointer forwards.
 * NOTE: If you want to do multiple operations, such as check the size and then push/pop,
 * it is safe to acquire the lock outside of push/pop and it will not result in deadlock.
*/
struct mBuffer {
    /// @brief The buffer that stores messages
    message_t buf[MESSAGE_BUFFER_SIZE];
    /// @brief The index of the next item to remove
    uint8_t front;
    /// @brief The index where we will insert the next item into the circular buffer
    uint8_t end;
    /// @brief The size of the buffer
    uint16_t size;
    /// @brief Lock for thread safety
    SemaphoreHandle_t lock;
};

/**
 * Initializes an mBuffer
*/
void mBufferInit(struct mBuffer *buf);

/**
 * puts a message into the given buffer.
 * NOTE: If the buffer overflows, overwrites the message at front of buffer;
 * @param[in] buf the buffer to insert into
 * @param[in] m   the message to insert into the buffer
*/
void mBufferPush(struct mBuffer *buf, message_t m);

/**
 * takes a message out of the given buffer.
 * NOTE: If the buffer is empty returns No_Command
 * @param[in] buf the buffer to remove the message from
*/
message_t mBufferPop(struct mBuffer *buf);

#endif /* _BUFFER_H_ */