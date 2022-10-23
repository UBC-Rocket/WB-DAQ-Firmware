#ifndef _MESSAGE_TYPES_H_
#define _MESSAGE_TYPES_H_

// Create Message Type that takes specific values using Enumerate:

typedef enum {
    No_Command,
    PWM_Pause,
    PWM_Resume
} message_t;

#endif /* _MESSAGE_TYPES_H_ */