#ifndef _PWM_H_
#define _PWM_H_


#include <stdint.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "can_controller.h"
#include <stdio.h>
#include "fsl_gpio.h"
#include "pin_mux.h"


#define VALVE_PIN BOARD_INITPINS_HS_SWITCH_B_IN0_GPIO
#define VALVE_PIN_MASK BOARD_INITPINS_HS_SWITCH_B_IN0_GPIO_PIN_MASK

extern uint32_t duty_cycle;
extern uint32_t period;

void actuatorTask(void *pv);

#endif