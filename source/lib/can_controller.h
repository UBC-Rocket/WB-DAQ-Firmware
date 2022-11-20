#ifndef _CAN_CONTROLLER_H_
#define _CAN_CONTROLLER_H_

#include <FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include "buffer.h"
// Initialize Semaphores:
extern SemaphoreHandle_t semaphore_Message;
extern SemaphoreHandle_t semaphore_PWMActive;

extern TaskHandle_t actuator_task;


void mainTask(void *pv);

#endif
