#ifndef _CAN_CONTROLLER_H_
#define _CAN_CONTROLLER_H_

#include <FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
// Initialize Semaphores:
extern SemaphoreHandle_t semaphore_Message;
extern SemaphoreHandle_t semaphore_PWMActive;

// Create Message Type that takes specific values using Enumerate:

typedef enum {
        No_Command,
        PWM_Pause,
        PWM_Resume
    } message_t;
extern message_t message;

void mainTask(void *pv);

#endif