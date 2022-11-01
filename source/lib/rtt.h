#ifndef _RTT_H_
#define _RTT_H_
#include <FreeRTOS.h>
#include "SEGGER_RTT.h"
#include <task.h>
#include <stdio.h>
#include <string.h>
#include <semphr.h>
#include "pwm.h"
#include <can_controller.h>
#include "pid.h"


void rttReceive(void *pv);

#endif